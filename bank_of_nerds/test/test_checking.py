import unittest
import re

from modules.accounts.checking import Checking, MaxOverDraftError
from modules.customer import Customer


class TestChecking(unittest.TestCase):
    """How many Checking can Checking Checking if a Checking can Checking"""

    def setUp(self):
        self.test = Checking(25.50, Customer("Bob", 25))

    def test_checking_init(self):
        self.assertTrue(self.test)

    def test_checking_get_property(self):
        date_reg = r'^\d{4}-\d{2}-\d{2}$'
        self.assertEqual(self.test.balance, "25.50")
        self.assertTrue(re.match(date_reg, str(self.test.date_created)))
        self.assertEqual(self.test.account_type, "Checking")

    def test_checking_withdrawal(self):
        self.test.withdrawal(25)
        self.assertEqual(self.test.balance, "0.50")

        self.test.withdrawal(1.50)
        self.assertEqual(self.test.balance, "-36.00")

    def test_checking_max_overdraft_error(self):
        def maxoverdraft():
            self.test.withdrawal(1000)

        self.assertRaises(MaxOverDraftError, maxoverdraft)
