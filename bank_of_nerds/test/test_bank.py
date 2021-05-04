import unittest

from modules.bank import Bank
from modules.customer import Customer
from modules.bankui import BankUI


class TestBank(unittest.TestCase):
    """How many Bank can Bank Bank if a Bank can Bank"""

    def setUp(self):
        self.test = Bank(False)

    def test_bank_init(self):
        self.assertTrue(self.test)

    def test_bank_menu(self):
        self.assertTrue(self.test._ui)

    def test_bank_add_customer(self):
        customer = Customer("Bob", 56)
        self.test.add_customer(customer)
        self.assertEqual(self.test._customers[0], customer)

    def test_bank_select_customer(self):
        customer = Customer("Bob", 56)
        self.test.add_customer(customer)
        self.assertEqual(self.test.select_customer(0), customer)

    def test_bank_create_defaults(self):
        self.test.create_defaults()
        self.assertEqual(len(self.test._customers), 2)
        self.assertEqual(self.test._customers[0].name, "Diet Dr. Pepper")
        self.assertEqual(self.test._customers[1].name, "Is Better")

    def test_bank_check_customer(self):
        customer = Customer("Bob", 56)
        self.test.add_customer(customer)
        self.assertFalse(self.test.check_customers("Bob"))
        self.assertTrue(self.test.check_customers("Beans"))
