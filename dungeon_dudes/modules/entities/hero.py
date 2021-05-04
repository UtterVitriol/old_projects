"""the player class
   inherits from the LivingThing class
"""

from modules.entities.living_thing import LivingThing


class Hero(LivingThing):
    """class used for player"""

    # string for DRYness
    _attack_potion = "Attack Potion"

    def __init__(self, name, show_rolls):
        """initializes variables for class"""

        self._inventory = []
        self._has_loot = False
        self._has_potion = False
        super().__init__(name, 10, 3, show_rolls)

    def get_loot(self, loot):
        """handles receiving loot"""

        self._has_loot = True
        self._inventory.append(loot)
        if loot == Hero._attack_potion:
            self._has_potion = True

    def drink_potion(self):
        """handles drinking attack potion"""

        if Hero._attack_potion in self._inventory:
            self._buff = True
            self._inventory.pop(self._inventory.index(Hero._attack_potion))
            if Hero._attack_potion not in self._inventory:
                self._has_potion = False

        if len(self._inventory) == 0:
            self._has_loot = False

    @property
    def inventory(self): return self._inventory

    @property
    def num_items(self): return len(self._inventory)

    @property
    def has_loot(self): return self._has_loot

    @property
    def has_potion(self): return self._has_potion

    @property
    def has_buff(self): return self._buff
