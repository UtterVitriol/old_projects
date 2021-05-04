import unittest
import re

from modules.accounts.account import OverDraftError
from modules.accounts.retirement401k import Retirement401k
from modules.accounts.retirement401k import CustomerAgeError
from modules.customer import Customer


class TestRetirement(unittest.TestCase):
    """How many Retirement can Retirement Retirement
       if a Retirement can Retirement
    """

    def setUp(self):
        self.test = Retirement401k(25.50, Customer("Bob", 67))

    def test_retirement_init(self):
        self.assertTrue(self.test)

    def test_retirement_get_property(self):
        date_reg = r'^\d{4}-\d{2}-\d{2}$'
        self.assertEqual(self.test.balance, "25.50")
        self.assertTrue(re.match(date_reg, str(self.test.date_created)))
        self.assertEqual(self.test.account_type, "Retirement401k")

    def test_retirement_withdrawal(self):
        self.assertEqual(self.test.withdrawal(25), True)
        self.assertRaises(OverDraftError, self.test.withdrawal, 100)

    def test_retirement_age_limit(self):
        self.test = Retirement401k(25.50, Customer("Bob", 66))
        self.assertRaises(CustomerAgeError, self.test.withdrawal, 100)
