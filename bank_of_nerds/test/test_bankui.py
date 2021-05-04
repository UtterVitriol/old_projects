import unittest

from modules.bankui import BankUI


class TestMenu(unittest.TestCase):
    """How many BankUI can BankUI BankUI if a BankUI can BankUI"""

    def setUp(self):
        self.menu = BankUI()

    def test_bankui_init(self):
        self.assertTrue(self.menu)

    def test_bankui_check_quit(self):
        self.assertTrue(self.menu._check_quit("q"))
        self.assertTrue(self.menu._check_quit("Q"))
        self.assertFalse(self.menu._check_quit("1"))
        self.assertFalse(self.menu._check_quit(1))
