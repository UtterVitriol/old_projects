from modules.entities.living_thing import LivingThing
import unittest


class TestLivingThing(unittest.TestCase):
    """Is a LivingThing really a LivingThing if it can't even LivingThing?"""

    def setUp(self):
        self.thing = LivingThing("Generic Name", 1, 1, False)

    def test_get_LivingThing_proptery(self):
        self.assertEqual(self.thing.name, "Generic Name")
        self.assertEqual(self.thing.health, 1)
        self.assertEqual(self.thing._num_dice, 1)

    def test_LivingThing_fight(self):
        self.assertLessEqual(sum(self.thing.attack()), 18)
        self.assertLessEqual(sum(self.thing.defend()), 18)
        self.thing._health = 2
        self.assertEqual(self.thing.take_damage(), True)
        self.assertEqual(self.thing.take_damage(), False)


if __name__ == "__main__":
    unittest.main()
