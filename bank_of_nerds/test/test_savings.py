import unittest
import re

from modules.accounts.account import OverDraftError
from modules.accounts.savings import Savings
from modules.customer import Customer


class TestSavings(unittest.TestCase):
    """How many Savings can Savings Savings if a Savings can Savings"""

    def setUp(self):
        self.test = Savings(25.50, Customer("Bob", 23))

    def test_savings_init(self):
        self.assertTrue(self.test)

    def test_savings_get_property(self):
        date_reg = r'^\d{4}-\d{2}-\d{2}$'
        self.assertEqual(self.test.balance, "25.50")
        self.assertTrue(re.match(date_reg, str(self.test.date_created)))
        self.assertEqual(self.test.account_type, "Savings")

    def test_savings_withdrawal(self):
        self.assertEqual(self.test.withdrawal(25), True)
        self.assertRaises(OverDraftError, self.test.withdrawal, 100)
