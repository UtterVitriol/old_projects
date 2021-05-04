"""This is getting prettty dicey"""

import random


class Dice:
    """dice class for rolling dice with a certain number of sides"""

    def __init__(self, sides):
        """initializes sides for dice"""

        self._sides = sides

    def roll(self):
        """handles rolling dice"""

        return random.randint(1, self._sides)
