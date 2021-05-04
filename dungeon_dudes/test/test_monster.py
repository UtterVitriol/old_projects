from modules.entities.monster import Monster
import unittest


class TestMonster(unittest.TestCase):
    """Is a monster really a monster if it can't even monster?"""

    def setUp(self):
        self.monster = Monster(False)

    def test_Monster_fight(self):
        self.assertLessEqual(sum(self.monster.attack()), 18)
        self.assertLessEqual(sum(self.monster.defend()), 18)
        self.monster._health = 2
        self.assertEqual(self.monster.take_damage(), True)
        self.assertEqual(self.monster.take_damage(), False)

    def test_Monster_drop_loot(self):
        temp_list = Monster._monster_loot_list
        temp_list.append(None)
        self.assertIn(self.monster.drop_loot(), temp_list)


if __name__ == "__main__":
    unittest.main()
