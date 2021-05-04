import unittest
import re

from modules.accounts.moneymarketfund import MoneyMarketFund
from modules.accounts.moneymarketfund import TransactionLimitError
from modules.customer import Customer
from modules.accounts.account import OverDraftError


class TestMoneyMarketFund(unittest.TestCase):
    """How many Retirement can Retirement Retirement
       if a Retirement can Retirement
    """

    def setUp(self):
        self.test = MoneyMarketFund(25.50, Customer("Bob", 67))

    def test_retirement_init(self):
        self.assertTrue(self.test)

    def test_retirement_get_property(self):
        date_reg = r'^\d{4}-\d{2}-\d{2}$'
        self.assertEqual(self.test.balance, "25.50")
        self.assertTrue(re.match(date_reg, str(self.test.date_created)))
        self.assertEqual(self.test.account_type, "MoneyMarketFund")

    def test_retirement_withdrawal(self):
        self.assertEqual(self.test.withdrawal(25), True)
        self.assertRaises(OverDraftError, self.test.withdrawal, 100)

    def test_retirement_transaction_limit(self):
        self.test.deposit(10.50)
        self.test.withdrawal(10.50)
        self.assertRaises(TransactionLimitError, self.test.withdrawal, 10.50)
        self.assertRaises(TransactionLimitError, self.test.deposit, 10.50)
