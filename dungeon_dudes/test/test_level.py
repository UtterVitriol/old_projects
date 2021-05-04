from modules.misc.level import LevelGenerator
import unittest


class TestLevelGenerator(unittest.TestCase):
    """Is a LevelGenerator really a LevelGenerator if it can't even LevelGenerator?"""

    def setUp(self):
        self.level = LevelGenerator(3, False)

    def test_get_LevelGenerator_proptery(self):
        self.assertNotIn(
            [self.level._name, self.level.description], LevelGenerator._levels)
        self.assertEqual(self.level._monster_count, 3)

    def test_LevelGenerator_kill(self):
        self.assertEqual(self.level.has_monster, True)
        self.level._kill_monster()
        self.assertEqual(self.level._monster_count, 2)
        self.level._kill_monster()
        self.assertEqual(self.level._monster_count, 1)
        self.level._kill_monster()
        self.assertEqual(self.level.has_monster, False)


if __name__ == "__main__":
    unittest.main()
