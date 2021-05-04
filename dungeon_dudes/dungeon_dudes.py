#!/usr/bin/env python3

"""Dungeon Dudes, the game!

   Run this file to play the game!
   Run with --dice and the combat rolls will be displayed
"""

from modules.entities.hero import Hero
from modules.misc.level import LevelGenerator
from modules.misc.dice import Dice
import os
import sys
import random


class PlayerQuit(Exception):
    """custom exception to handle player quitting game"""
    pass


class GameEngine:
    """handles top level game logic

       init, then call start_game() method-
       -to run the game.
    """

    def __init__(self):
        self._monsters = 5  # number of monsters to be faced off against
        self._levels = 5  # number of levels to clear to win

    def _clear_screen(self):
        """checks os type and clears screen"""

        if os.name == "nt":
            os.system("cls")
        else:
            os.system("clear")

    def start_game(self, show_rolls):
        """handles starting the game

           gets player name and initializes player/level
           starts game loop
        """
        self._show_rolls = show_rolls
        self._clear_screen()
        print("Welcome, Adventurer!")
        name = input("Please enter your name: ")
        if name == "":  # player didn't enter anything
            name = "Hero"
        self._clear_screen()
        self._player = Hero(name, show_rolls)
        self._player_ran_away = False
        self._level = LevelGenerator(1, show_rolls)
        self._game_loop()

    def _roll_initiative(self):
        """determines if the monsters or the player attacks first"""

        monster_roll = 0
        player_roll = 0

        dialogue = "{} got the jump on {}!"
        monster = "The monsters in this room"
        player = "You"

        while monster_roll == player_roll:  # rolls until there's not a tie
            monster_roll = self._level.current_monster.d20.roll()
            player_roll = self._player.d20.roll()

        if player_roll > monster_roll:  # player has initiative
            print(dialogue.format(player, monster))
            self._monster_initiative = False
            return

        print(dialogue.format(monster, player))
        self._monster_initiative = True  # monsters have initiative

    def _game_loop(self):
        """main game loop for game"""

        while self._levels > 0:  # loop while there are still levels
            self._level.print_dialogue()
            self._roll_initiative()
            self._pause_to_continue()
            self._clear_screen()

            # loop while there are still monsters
            while self._level.has_monster:
                self._approaches()

                player_died = self._battle()

                if (player_died):
                    return

                if self._player_ran_away:
                    self._player_ran_away = False
                    self._clear_screen()
                    break

            print("You beat this room!")

            while self._print_menu(False) is not True:
                self._clear_screen()
                pass

            self._clear_screen()
            self._level.next_level(1, self._show_rolls)
            self._levels -= 1

        self._clear_screen()
        print("You win!")

    def _print_menu(self, in_battle):
        """all-in-one menu for game"""

        ask = f"What would you like to do, {self._player.name}?"
        invalid = "Invalid selection"

        monster_stats = (self._level.current_monster.name,
                         "health: ", self._level.current_monster.health)

        player_stats = ("Your health: ", self._player.health)

        while True:  # loop until players choice progresses game
            options = []
            print(*player_stats)

            if in_battle:
                print(*monster_stats)

            if self._level.has_loot and not in_battle:
                print("Current loot in room")
                for loot in self._level.loot:
                    print("-", loot)

            print(ask)  # ask the player what they want to do

            if self._player.has_loot:
                print(" 1 | Check inventory")
                options.append(1)
            if in_battle:
                print(" 2 | Attack")
                options.append(2)

                print(" 6 | Run Away")
                options.append(6)
            else:
                print(" 3 | Next level")
                options.append(3)

            if self._level.has_loot:
                print(" 4 | Pickup Loot")
                options.append(4)

            if (self._player.has_potion and in_battle
                    and not self._player.has_buff):
                print(" 5 | Drink Potion")
                options.append(5)

            print(" 0 | Quit")
            options.append(0)

            try:  # validate player choice
                choice = int(input("> "))
                if choice in options:
                    return self._menu_logic(choice)
            except ValueError:
                pass

            self._clear_screen()
            print(invalid)

    def _menu_logic(self, choice):
        """handles player menu choice"""

        if choice == 1:  # show inventory
            self._show_inventory()
        elif choice == 2:  # attack
            self._clear_screen()
            self._player_turn(self._player, self._level.current_monster)
            return True
        elif choice == 3:  # next level
            return True
        elif choice == 4:  # pickup loot
            try:
                self._clear_screen()
                print("Current loot in room")
                for index, loot in enumerate(self._level.loot, 1):
                    print(index, loot, sep=" | ")
                item = int(input("Which item?\n> "))
                item -= 1
                if item in range(0, len(self._level.loot)):
                    self._player.get_loot(self._level.loot[item])
                    self._level.remove_loot(self._level.loot[item])
            except ValueError:
                pass

            self._clear_screen()
        elif choice == 5:  # drink potion
            self._player.drink_potion()
        elif choice == 6:  # run away
            if self._run_away():
                return True
        elif choice == 0:  # quit
            raise PlayerQuit("Player Chose to quit")

        return False

    def _battle(self):
        """handles battle between player and monster"""

        # monster initiative
        if (self._monster_initiative):
            self._monster_turn(self._level.current_monster, self._player)
            self._clear_screen()

        while True:

            # player trurn
            while self._print_menu(True) is not True:
                self._clear_screen()
                pass

            if self._player_ran_away:
                return False

            # player died
            if self._player.health == 0:
                return True

            self._clear_screen()

            # check if monster died
            if self._level.check_monster():
                self._victory()
                self._clear_screen()
                return False

            # monster turn
            if self._monster_turn(self._level.current_monster, self._player):
                return True

            self._clear_screen()

    def _monster_turn(self, monster, player):
        """handles monster attack phase"""

        monster_name = self._level.current_monster.name
        print(f"{monster_name}'s turn.")

        monster_roll = monster.attack()
        player_roll = player.defend()

        if not self._check_rolls(monster_roll, player_roll):
            print(
                f"You successfully evade The {monster_name}'s attack.")
            self._pause_to_continue()
            return False

        print(f"The {monster_name} slaps you hard!")
        self._pause_to_continue()

        # check if player died
        if not player.take_damage():
            self._game_over()
            return True

    def _player_turn(self, player, monster):
        """handles player attack phase"""

        print("Your turn.")

        player_roll = player.attack()
        monster_roll = monster.defend()
        monster_name = self._level.current_monster.name

        if not self._check_rolls(player_roll, monster_roll):
            print(f"The {monster_name} successfully evades your attack.")
            self._pause_to_continue()
            return

        print(f"You slap the {monster_name} hard!")
        self._pause_to_continue()
        monster.take_damage()

    def _show_inventory(self):
        """prints player inventory"""

        self._clear_screen()
        print(f"{self._player.name}, these are the items in your inventory:")

        for item in self._player.inventory:
            print("-", item)

        self._pause_to_continue()

    def _victory(self):
        """handles player beating monster"""

        print("You successfully killed the monster!")

        dropped = "The monster dropped"
        if (self._level.loot_dropped):  # dropped loot
            while True:
                print(dropped, f"a {self._level.last_loot}!")
                print(f"Take {self._level.last_loot}?")
                choice = input("Y/N?\n> ")

                if choice.lower() == "y":
                    self._player.get_loot(self._level.last_loot)
                    self._level.remove_loot(self._level.last_loot)
                    return
                elif choice.lower() == "n":
                    return

                self._clear_screen()
        else:  # dropped nothin
            print(dropped, "Nothing")

        self._pause_to_continue()

    def _run_away(self):
        """handles player attempt to run away from monster"""

        self._clear_screen()
        print("You attempt to run away.")
        chance = (self._player.health * 10) / 100  # chance to escape

        if random.random() <= chance:
            print("You got away safely!")
            self._pause_to_continue()
            self._player_ran_away = True
            return True
        else:
            print(
                f"You ran right into the {self._level.current_monster.name}.")
            return self._counter_attack()

    def _counter_attack(self):
        """handles player failing to run way from monster"""

        monster_name = self._level.current_monster.name

        monster_roll = self._level.current_monster.attack()
        player_roll = self._player.defend(1)

        if not self._check_rolls(monster_roll, player_roll):
            print(
                f"The {monster_name} trips over itself.")
            self._pause_to_continue()
            return False

        print(f"The {monster_name} judo chops you in the throat!")
        self._pause_to_continue()

        # check if player died
        if not self._player.take_damage():
            self._game_over()
            return True

        return False

    def _game_over(self):
        """prints game over and items in players inventory if not empty"""

        self._clear_screen()

        print("Game Over")

        if len(self._player.inventory) > 0:
            print("You died with:")
            for item in self._player.inventory:
                print("-", item)

    def _check_rolls(self, attacker, defender):
        """handles determining winner of an attack phase"""

        attacker = sorted(attacker, reverse=True)
        defender = sorted(defender, reverse=True)

        check = (attacker, defender)

        check = sorted(check, reverse=True)

        if check[0] == check[1]:  # rolls are the same, attacker fails
            return False

        if check[0] == attacker:  # attacker wins
            return True

        return False  # attacker fails

    def _approaches(self):
        """prints monster approach message"""

        print(f"A {self._level.current_monster.name} approches you quickly!")

    def _pause_to_continue(self):
        """pauses the game to allow player to read information"""

        input("Press enter to continue")


def handle_args():
    """handles command line arguments"""

    if len(sys.argv) == 2:
        if sys.argv[1] == "--dice":
            return True
        else:
            raise Exception(
                " ".join((sys.argv[1], "is not a valid argument")))
    elif len(sys.argv) > 2:
        raise Exception("Too many arguments")

    return False


def main():
    """it's main"""

    print_rolls = handle_args()
    dungeon_dudes = GameEngine()
    dungeon_dudes.start_game(print_rolls)


if __name__ == "__main__":
    try:
        main()
    except PlayerQuit as pq:
        pass  # Player chose to quit
    except (Exception, KeyboardInterrupt, SystemExit, GeneratorExit) as e:
        print(e)
