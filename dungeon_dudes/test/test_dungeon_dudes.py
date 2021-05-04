from dungeon_dudes import GameEngine
from modules.entities.hero import Hero
from modules.misc.level import LevelGenerator
from modules.misc.dice import Dice

import unittest


class TestGameEngine(unittest.TestCase):
    """Is a GameEngine really a GameEngine if it can't even GameEngine?"""

    def setUp(self):
        self.game = GameEngine()
        show_rolls = False
        self.game._show_rolls = show_rolls
        self.game._player = Hero("Hero", show_rolls)
        self.game._player_ran_away = False
        self.game._level = LevelGenerator(1, show_rolls)

    def test_get_GameEngine_proptery(self):
        self.assertEqual(self.game._monsters, 5)
        self.assertEqual(self.game._levels, 5)

    def test_GameEngine_initiative(self):
        self.game._roll_initiative()
        self.assertIn(self.game._monster_initiative, [True, False])

    def test_GameEngine_check_rolls(self):
        player_roll = [6, 4, 1]
        monster_Roll = [6, 4]

        result = self.game._check_rolls(player_roll, monster_Roll)
        self.assertIs(result, True)


if __name__ == "__main__":
    unittest.main()
