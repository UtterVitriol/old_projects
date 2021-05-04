"""Bank Class."""

import os

from modules.accounts.checking import Checking
from modules.accounts.savings import Savings
from modules.accounts.retirement401k import Retirement401k
from modules.accounts.moneymarketfund import MoneyMarketFund
from modules.customer import Customer
from modules.bankui import BankUI


class Bank:
    """Bank Class."""

    def __init__(self, show_defaults):
        """Initialize variables for class."""

        self._customers = []
        self._ui = BankUI()
        self._show_defualts = show_defaults

    def start(self):
        """Call create_defaults and ui.start."""

        self.create_defaults()
        self._ui.start(self)

    def create_defaults(self):
        """Create default users."""

        self.default1 = Customer("Diet Dr. Pepper", 23)
        self.default2 = Customer("Is Better", 67)
        self.default1.add_account(Savings(55.25, self.default1))
        self.default1.add_account(MoneyMarketFund(56.25, self.default1))
        self.default2.add_account(Savings(57.25, self.default2))
        self.default2.add_account(Checking(58.25, self.default2))
        self.default2.add_account(Retirement401k(59.25, self.default2))
        self.add_customer(self.default1)
        self.add_customer(self.default2)

    def print_defaults(self):
        """Print default user details."""

        self.default1.print_details()
        self.default2.print_details()

    def add_customer(self, customer):
        """Add customer to customers list."""

        self._customers.append(customer)

    def check_customers(self, name):
        """Return False if customer exists, True otherwise."""

        for customer in self._customers:
            if name.lower() == customer.name.lower():
                return False

        return True

    def print_customers(self):
        """Print customers from customers list."""

        if len(self._customers) == 0:
            print("None")
            return None

        for index, customer in enumerate(self._customers):
            print(index + 1, "|", customer.name)

        return index + 2

    def select_customer(self, index):
        """Return customer from customers list at index"""

        return self._customers[index]

    @ property
    def customers(self): return self._customers
