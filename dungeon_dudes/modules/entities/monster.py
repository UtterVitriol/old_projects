"""the monster class
   inherits from the LivingThing class
"""

from modules.entities.living_thing import LivingThing
import random


class Monster(LivingThing):
    """class used for monsters"""

    # Name, Health, Dice
    _monster_list = [["Bat", 1, 1],
                     ["Goblin", 1, 2],
                     ["Naga", 2, 2],
                     ["Molebear", 2, 3],
                     ["Dragon", 3, 3]]

    # loot that can be dropped
    _monster_loot_list = ["Gold Coin",
                          "Meat",
                          "Sparkly Gem",
                          "Legendary Yo-Yo Blade",
                          "Rock",
                          "Attack Potion"]

    def __init__(self, show_rolls):
        """initializes variables for class"""

        super().__init__(*self._gen_monster(), show_rolls)

    def _gen_monster(self):
        """creates random monster from _monster_list"""

        monster = random.choice(Monster._monster_list)

        # removes monster from list so it isn't chosen again
        Monster._monster_list.pop(Monster._monster_list.index(monster))
        return monster

    def drop_loot(self):
        """handles monster dropping loot"""

        drop_chance = ((self._num_dice + self._max_health) * 10) / 100

        if random.random() <= drop_chance:
            return random.choice(self._monster_loot_list)

        return None
