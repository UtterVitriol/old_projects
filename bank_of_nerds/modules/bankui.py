"""User interface for Bank Class."""

import os

from modules.accounts.account import OverDraftError
from modules.accounts.checking import Checking, MaxOverDraftError
from modules.accounts.savings import Savings
from modules.accounts.moneymarketfund import MoneyMarketFund
from modules.accounts.moneymarketfund import TransactionLimitError
from modules.accounts.retirement401k import Retirement401k, CustomerAgeError
from modules.customer import Customer


class BankUI:
    """User interface for Bank Class.

       Handles logic for Bank Class.
    """

    def __init__(self):
        """Initialize variables for class."""

        # string variables to stay DRY
        self._invalid = "Invalid choice."
        self._quit = "Q | Exit"
        self._cursor = "> "

    def _clear_screen(self):
        """Clear screen."""

        if os.name == "posix":
            os.system("clear")
        else:
            print("*" * 50)

    def _pause_to_continue(self):
        """pause to allow user to read information."""

        input("Press enter to continue")

    def _check_quit(self, choice):
        """Check choice equal to "q" or "Q"."""

        if isinstance(choice, str):
            if choice.lower() == "q":
                return True

        return False

    def start(self, bank):
        """Main menu for Bank"""

        self._bank = bank
        options = [1, 2]

        while True:
            self._clear_screen()

            # if --secret backdoor flag was set
            if bank._show_defualts:
                bank.print_defaults()

            print("Welcome to Bank.")
            print("1 | New Customer")
            print("2 | Existing Customers")
            print(self._quit)

            choice = self._get_choice(options)

            if self._check_quit(choice):
                break

            if choice:
                choice = choice - 1
                if choice == 0:
                    self._add_customer()
                elif choice == 1:
                    self._existing_customers()

    def _add_customer(self):
        """Add customer to Bank"""

        self._clear_screen()
        print("Enter Customer Name")
        print(self._quit)
        name = input(self._cursor)

        if self._check_quit(name):
            self._clear_screen()
            return
        elif name == "":
            print("Invalid Name")
            self._pause_to_continue()
            return

        self._clear_screen()

        while True:
            print("Input Customer Age")
            print(self._quit)
            age = self._get_choice(range(0, 123))

            if self._check_quit(age):
                self._clear_screen()
                return

            if age:
                if self._bank.check_customers(name):
                    new_customer = Customer(name, age)
                    self._bank.add_customer(new_customer)
                else:
                    print("Customer already exists")
                    self._pause_to_continue()
                self._clear_screen()
                return

    def _existing_customers(self):
        """Print Bank's current customers."""

        while True:
            self._clear_screen()
            print("Customers:")
            options = self._bank.print_customers()

            if options is None:
                self._pause_to_continue()
                self._clear_screen()
                return

            options = range(0, options)

            print(self._quit)

            choice = self._get_choice(options)

            if self._check_quit(choice):
                return

            if choice:
                choice = choice - 1
                if choice in options:
                    self._manage_customer(choice)
                    continue

    def _manage_customer(self, index):
        """Manage Bank's customer at index"""

        while True:
            self._clear_screen()

            customer = self._bank.select_customer(index)

            print(customer.name, "'s accounts:", sep="")

            options = customer.print_accounts()

            if options is None:
                print("None")
            else:
                options = range(1, options)

            print("\nN | New Account")
            print(self._quit)

            choice = self._get_choice(options, "n")

            if self._check_quit(choice):
                self._clear_screen()
                return
            elif choice == "n":
                self._add_account(customer)
                self._clear_screen()
                continue

            if choice:
                choice = choice - 1
                account = customer.select_account(choice)
                self._manage_account(account)

    def _add_account(self, customer):
        """Add account to Bank"""

        self._clear_screen()
        options = [1, 2, 3, 4]
        print("1 | Checking")
        print("2 | Savings")
        print("3 | Retirement401k")
        print("4 | Money Market Fund")
        print(self._quit)
        choice = self._get_choice(options)

        if self._check_quit(choice):
            return

        if choice:
            if choice == 1:
                self._new_account(customer, Checking)
            elif choice == 2:
                self._new_account(customer, Savings)
            elif choice == 3:
                self._new_account(customer, Retirement401k)
            elif choice == 4:
                self._new_account(customer, MoneyMarketFund)
            else:
                print(choice)
                self._pause_to_continue()

    def _manage_account(self, account):
        """Manage customer account."""

        while True:
            self._clear_screen()
            options = [1, 2, 3, 4]
            print(account.account_type, account.account_number)
            print("1 | Balance")
            print("2 | Withdrawal")
            print("3 | Deposit")
            print("4 | Show Details")
            print(self._quit)

            choice = self._get_choice(options)

            if self._check_quit(choice):
                self._clear_screen()
                return

            if choice:
                if choice == 1:
                    print(account.balance)
                    self._pause_to_continue()
                elif choice == 2:
                    self._withdrawal(account)
                elif choice == 3:
                    self._deposit(account)
                elif choice == 4:
                    self._show_details(account)

    def _show_details(self, account):
        """Print account details"""

        self._clear_screen()
        account.get_details()
        self._pause_to_continue()

    def _deposit(self, account):
        """Deposit to account."""

        amount = self._get_amount()

        if self._check_quit(amount):
            self._clear_screen()
            return

        try:
            account.deposit(amount)
        except TransactionLimitError as tle:
            print(tle)
            self._pause_to_continue()

    def _withdrawal(self, account):
        """Withdrawal from account."""

        amount = self._get_amount()

        if self._check_quit(amount):
            self._clear_screen()
            return

        try:
            account.withdrawal(amount)

            # you shouldn't have showed me how to do this
        except (OverDraftError,
                MaxOverDraftError,
                CustomerAgeError,
                TransactionLimitError) as e:

            print(e)
            self._pause_to_continue()

    def _new_account(self, customer, account):
        """Create new account for customer."""

        amount = self._get_amount()

        if self._check_quit(amount):
            return

        if amount:
            customer.add_account(account(amount, customer))

    def _get_amount(self, can_convert=True):
        """Get amount from user, return amount."""

        while True:
            self._clear_screen()
            minimum = "Minimum = 0.01"

            convert = "\nC | Currency Conversion"

            if not can_convert:
                convert = ""

            print("Enter Amount")
            print(minimum)
            prompt = "\n".join((convert, self._quit, self._cursor))

            amount = input(prompt)

            if can_convert:
                if amount.lower() == "c":
                    amount = self._convert_currency()
                    return amount

            if self._check_quit(amount):
                return amount

            try:
                # check for 0.00 format of amount
                if ("." not in amount or len(amount.split(".")[1]) != 2
                        or len(amount.split(".")) > 2):

                    print(self._invalid)
                    print("Please enter positive values ", end="")
                    print("to two decimal places.")
                    print("Example: 19.00")
                    raise ValueError

                amount = float(amount)

                if amount < 0.01:
                    print(minimum)
                    raise ValueError
            except ValueError:
                self._pause_to_continue()
                continue
            return amount

    def _convert_currency(self):
        """Convert currency."""

        options = [1, 2]
        while True:
            self._clear_screen()
            print("Supported Currencies:")

            pepper = 2
            cookie_crisp = .5

            rate = "Rate:"
            print("1 | Doctors Peppers -", rate, f"{pepper:.2f}")
            print("2 | Cookie Crisps -", rate, f"{cookie_crisp:.2f}")
            print(self._quit)

            choice = self._get_choice(options)

            if self._check_quit(choice):
                return choice

            if choice not in options:
                continue

            amount = self._get_amount(can_convert=False)

            if self._check_quit(amount):
                return amount

            if choice == 1:
                return amount * pepper
            elif choice == 2:
                return amount * cookie_crisp

    def _get_choice(self, options, optional=False):
        """Validate user choice."""

        try:
            choice = input(self._cursor)

            if self._check_quit(choice):
                return choice

            if optional:
                if choice.lower() == optional:
                    return choice.lower()

            if not options:
                options = []

            choice = int(choice)

            if choice in options:
                return choice
            else:
                raise ValueError
        except ValueError:
            print(self._invalid)
            self._pause_to_continue()
            self._clear_screen()
            return False
