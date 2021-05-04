import unittest

from modules.customer import Customer
from modules.accounts.checking import Checking
from modules.accounts.savings import Savings
from modules.accounts.retirement401k import Retirement401k
# from modules.accounts.account import Checking


class TestCustomer(unittest.TestCase):
    """How many Customer can Customer Customer if a Customer can Customer"""

    def setUp(self):
        self.test = Customer("Bob", 69)

    def test_customer_init(self):
        self.assertTrue(self.test)

    def test_customer_get_property(self):
        self.assertEqual(self.test.age, 69)
        self.assertEqual(self.test.checking, [])
        self.assertEqual(self.test.savings, [])
        self.assertEqual(self.test.retirement, [])
        self.assertEqual(self.test.mmf, [])

    def test_customer_add_checking(self):
        self.assertEqual(self.test.checking, [])
        account = Checking(23, self.test)
        self.test.add_account(account)
        self.assertEqual(self.test.checking[0], account)

    def test_customer_add_savings(self):
        self.assertEqual(self.test.savings, [])
        account = Savings(23, self.test)
        self.test.add_account(account)
        self.assertEqual(self.test.savings[0], account)

    def test_customer_add_retirement(self):
        self.assertEqual(self.test.retirement, [])
        account = Retirement401k(23, self.test)
        self.test.add_account(account)
        self.assertEqual(self.test.retirement[0], account)

    def test_customer_select_account(self):
        account = Checking(23, self.test)
        self.test.add_account(account)
        check = self.test.select_account(0)
        self.assertEqual(account, check)
