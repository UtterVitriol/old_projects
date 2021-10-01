#!/usr/bin/env python3

"""The guess game. Requires player.py(included in git)

   Run this file to play! Run with -u and the computer might lie once per game.
"""

import random
import sys
import os
try:
    from player import Player
except ModuleNotFoundError as msg:
    raise SystemExit("Application seems to be corrupted.\n"
                     "Unable to continue...")


class Game:

    """(Guess a number between 1 and 100)
       Handles running the game and saving game stats

       Init game with no arguments, then call the start() method-
       -to run the game.
       Creates game_data.txt to store game stats.
    """

    def __init__(self):
        self._number = random.randint(1, 100)  # Number to be guessed

        if len(sys.argv) == 2 and sys.argv[1] == "-u":  # Checks for -u
            self._lying = True
            self._lie_round = random.randint(1, 10)  # Sets round to lie on
        else:
            self._lie_round = None
            self._lying = False

        self._lie_stuff = (0, '', False)  # Used in the lie print statement
        self._lie = " I lied about {} being too {}."
        self._not_lie = " I did not lie this game."
        self._save_file = "game_data.txt"

    def _exit(self, guess):  # Exit menu
        response = ''
        options = ["Y", "N"]
        while response != options[0] or response != options[1]:
            try:
                response = input("\nAre you sure you want to exit? Y or N: ")
            except (KeyboardInterrupt, EOFError):
                guess = (guess, None)
                print()
                return guess
            if response == options[0]:
                guess = (guess, None)
                return guess
            elif response == options[1]:
                if guess == "":  # "" is used to see if it's the first turn
                    return guess
                elif guess is False:  # Used for player name
                    return ""
                else:  # During main loop
                    guess = (guess, "1")
                    return guess

    def _input(self, guess, string=""):  # Input validation
        try:
            if guess is False:
                user_name = input(string)
                return user_name
            else:
                guess = input()
                return guess
        except (KeyboardInterrupt, EOFError):
            guess = self._exit(guess)
            return guess

    def _read_save(self):  # Reads saved game stats
        with open(self._save_file) as data:
            data = data.readlines()
        game_stats = [int(len(data)),
                      sum([int(x.split()[0]) for x in data]),
                      sum([int(x.split()[1]) for x in data])]

        return game_stats

    def _save_data(self, good_guesses, bad_guesses):  # Saves game stats
        if os.path.exists(self._save_file):
            game_stats = self._read_save()  # Gets game stats
            game_stats[0] += 1
            game_stats[1] += good_guesses
            game_stats[2] += bad_guesses

            game_stats.append(game_stats[1] / game_stats[0])

        else:  # Sets base game stats
            game_stats = [1, good_guesses, bad_guesses, good_guesses]

        player = self._player._get_new(good_guesses, bad_guesses)

        fmt = "".join(["{}Games Played: {}, Good Guesses: {}",
                       ", Invalid Guesses: {}, Average Guesses to Win:",
                       " {:0.2f}"])

        player_phrase = " ".join([self._player._name, "- "])

        print(fmt.format("", game_stats[0], game_stats[1], game_stats[2],
                         round(game_stats[3], 2)))

        print(fmt.format(player_phrase, player[0], player[1], player[2],
                         round(player[3], 2)))

        line = f"{good_guesses} {bad_guesses}\n"  # Line to write to file

        with open(self._save_file, 'a') as data:
            data.writelines(line)  # Save game stats

        self._player._save_data(good_guesses, bad_guesses)  # Player save

    def _main_loop(self):  # Main game loop....or loops
        lines = ["I'm thinking of a number from 1 to 100\n",
                 "Try to guess my number: ",
                 "{} is too low - please guess again: ",
                 "{} is too high - please guess again: ",
                 "{} is not a valid guess - please guess again: "]

        number = self._number
        guess = ""
        good_guesses = 0
        bad_guesses = 0
        guess_round = 1  # Sometimes 1 is better than 0

        def _bad_value(guess, bad_guesses):  # Player entered bad value
            print(lines[4].format(guess), end="")
            guess = self._input(guess)
            bad_guesses += 1
            return guess, bad_guesses

        print(lines[0], end="")  # Can you guess the number????????????????????

        while guess == "":  # First guess
            print(lines[1], end="")
            guess = self._input(guess)
            if isinstance(guess, tuple):  # Checks to exit
                if guess[1] is None:
                    return
        while True:  # game loop
            if isinstance(guess, tuple):  # Checks to exit
                if guess[1] is None:
                    return
            try:
                if isinstance(guess, tuple):  # If interrupted
                    guess = guess[0]
                    if str(guess).isnumeric():  # "Resets" guesses
                        good_guesses -= 1
                    else:
                        bad_guesses -= 1
                    continue
                else:
                    guess = int(guess)  # probably not necessary. just in case

                if guess not in range(1, 101):  # cat is cat
                    guess, bad_guesses = _bad_value(guess, bad_guesses)
                    continue
                else:
                    good_guesses += 1

            except ValueError:
                guess, bad_guesses = _bad_value(guess, bad_guesses)
                continue

            if guess > number:
                if guess_round == self._lie_round:  # YOU LIAR
                    if self._lying:
                        print(lines[2].format(guess), end="")
                        self._lie_stuff = (guess, "low", True)
                        self._lying = False
                        guess = self._input(guess)
                else:
                    print(lines[3].format(guess), end="")
                    guess = self._input(guess)

            elif guess < number:
                if guess_round == self._lie_round:  # YOU LIAR
                    if self._lying:
                        print(lines[3].format(guess), end="")
                        self._lie_stuff = (guess, "high", True)
                        self._lying = False
                        guess = self._input(guess)
                else:
                    print(lines[2].format(guess), end="")
                    guess = self._input(guess)

            else:  # End of game, print the stats for this game
                lie = ""

                if self._lie_stuff[2]:
                    lie = self._lie.format(self._lie_stuff[0],
                                           self._lie_stuff[1])
                else:
                    if self._lie_round is not None:
                        lie = self._not_lie

                verbage_guess = ["guess", "guesses"]

                if good_guesses == 1:
                    good_verb = verbage_guess[0]
                else:
                    good_verb = verbage_guess[1]

                if bad_guesses == 1:
                    bad_verb = verbage_guess[0]
                else:
                    bad_verb = verbage_guess[1]

                bad = "."

                if bad_guesses > 0:
                    bad = f" and made {bad_guesses} invalid {bad_verb}."

                print(f"{guess} is correct! You guessed my number in "
                      f"{good_guesses} {good_verb}{bad}"
                      f"{lie}")

                break

            guess_round += 1

        self._save_data(good_guesses, bad_guesses)
        return True

    def start(self):  # Create player, start main_loop
        print("Welcome to Guess-the-Number!\n"
              "Play anonymously? - A\n"
              "New or returning player? - P")

        choice = ""
        anon = "Anonymous"  # DRY DRY DRY DRY DRY DRY DRY DRY XD
        options = ["A", "P"]

        while choice != options[0] and choice != options[1]:
            choice = self._input(False, "A or P: ")
            if isinstance(choice, tuple):  # Checks to exit
                if choice[1] is None:
                    return

        if choice == options[0]:
            self._player = Player(anon)
        elif choice == options[1]:
            player_name = ""
            while player_name == "":
                player_name = self._input(False, "Enter your name: ")
                if isinstance(player_name, tuple):  # Checks to exit
                    if player_name[1] is None:
                        return
            self._player = Player(player_name)
        else:
            self._player = Player(anon)

        print("Welcome, ", self._player._name, "!", sep="")
        self._main_loop()


def main():

    """Instantiates the game and starts it."""

    game = Game()
    game.start()


if __name__ == "__main__":
    try:
        main()
    except (SystemExit, KeyboardInterrupt, GeneratorExit, Exception) as msg:
        print("\n\n", msg, "\nGoodbye", sep="")
