from modules.accounts.account import Account
import unittest


class TestAccount(unittest.TestCase):
    """How many account can account account if a account can account"""

    def instantiate(self):
        test = Account(25)

    def test_account_cannot_instantiate(self):
        self.assertRaises(TypeError, self.instantiate)
