"""base class for all living things in the dungeon dudes"""

from modules.misc.dice import Dice


class LivingThing:
    """class used for living things"""

    def __init__(self, name, health, num_dice, show_rolls):
        """initializes variables for class"""

        self._name = name
        self._health = health
        self._max_health = health
        self._num_dice = num_dice
        self._d6 = Dice(6)
        self._d20 = Dice(20)
        self._show_rolls = show_rolls
        self._buff = False

    def attack(self, is_attack=True, dice=-1):
        """handles rolling dice for attack"""

        if dice == -1:
            dice = self._num_dice

        total = []
        if self._show_rolls:
            print(self._name, "rolls ", end="")

        for die in range(0, dice):
            roll = self._d6.roll()
            if self._show_rolls:
                print(roll, end=" ")
            total.append(roll)

        if self._buff:
            roll = self._d6.roll()
            if self._show_rolls:
                print(roll, end=" ")
            total.append(roll)
            self._buff = False

        if self._show_rolls:
            print("")
        return total

    def defend(self, dice=-1):
        """calls attack to roll for defence"""

        return self.attack(False, dice)

    def take_damage(self):
        """handles taking damage"""

        self._health -= 1

        if self._health > 0:
            return True

        return False

    @property
    def name(self): return self._name

    @property
    def health(self): return self._health

    @property
    def d6(self): return self._d6

    @property
    def d20(self): return self._d20

    @property
    def has_buff(self): return self._buff
