from modules.entities.hero import Hero
import unittest


class TestHero(unittest.TestCase):
    """Is a hero really a hero if it can't even hero?"""

    def setUp(self):
        self.hero = Hero("Generic Name", False)

    def test_get_Hero_proptery(self):
        self.assertEqual(self.hero.name, "Generic Name")
        self.assertEqual(self.hero.health, 10)
        self.assertEqual(self.hero._num_dice, 3)
        self.assertEqual(self.hero.inventory, [])

    def test_Hero_fight(self):
        self.assertLessEqual(sum(self.hero.attack()), 18)
        self.assertLessEqual(sum(self.hero.defend()), 18)
        self.hero._health = 2
        self.assertEqual(self.hero.take_damage(), True)
        self.assertEqual(self.hero.take_damage(), False)

    def test_Hero_get_loot(self):
        self.hero.get_loot("Beans")
        self.assertEqual(self.hero.num_items, 1)
        self.assertEqual(self.hero.inventory[0], "Beans")
        self.hero.get_loot("Bones")
        self.assertEqual(self.hero.num_items, 2)
        self.assertEqual(self.hero.inventory[1], "Bones")


if __name__ == "__main__":
    unittest.main()
