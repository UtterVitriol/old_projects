from modules.misc.dice import Dice
import unittest


class TestDice(unittest.TestCase):
    """Is a Dice really a Dice if it can't even Dice?"""

    def setUp(self):
        self.d6 = Dice(6)
        self.d20 = Dice(20)

    def test_get_dice_proptery(self):
        self.assertEqual(self.d6._sides, 6)
        self.assertEqual(self.d20._sides, 20)

    def test_dice_roll(self):
        self.assertLessEqual(self.d6.roll(), 6)
        self.assertLessEqual(self.d20.roll(), 20)


if __name__ == "__main__":
    unittest.main()
