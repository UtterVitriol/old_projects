"""Customer class."""

from modules.accounts.checking import Checking
from modules.accounts.savings import Savings
from modules.accounts.retirement401k import Retirement401k
from modules.accounts.moneymarketfund import MoneyMarketFund


class Customer:
    """Customer class."""

    def __init__(self, name, age):
        """Initialize variables for class."""

        self._checking = Checking
        self._savings = Savings
        self._retirement401k = Retirement401k
        self._moneymarketfund = MoneyMarketFund
        self._age = age
        self._name = name
        self._my_checking = []
        self._my_savings = []
        self._my_retirement = []
        self._my_moneymarket = []

    def add_account(self, account):
        """Add account to accounts type list."""

        if isinstance(account, self._checking):
            self._my_checking.append(account)
        elif isinstance(account, self._savings):
            self._my_savings.append(account)
        elif isinstance(account, self._retirement401k):
            self._my_retirement.append(account)
        elif isinstance(account, self._moneymarketfund):
            self._my_moneymarket.append(account)

    def print_details(self):
        """Print customer details."""

        print("Name:", self._name, " | Age:", self._age)
        print("\nDetails:\n")

        # create list of all accounts
        accounts = (self._my_checking + self._my_savings +
                    self._my_retirement + self._my_moneymarket)

        for account in accounts:
            account.get_details()
            print("")

        print("*" * 50)

    def print_accounts(self):
        """Print customer account details."""

        # get account class name as string
        accounts = (self._my_checking + self._my_savings +
                    self._my_retirement + self._my_moneymarket)

        if len(accounts) == 0:
            return None

        for index, account in enumerate(accounts):
            print(index + 1, "|", account.account_type,
                  account.account_number)

        return index + 2

    def select_account(self, index):
        """Return customer account at index."""

        accounts = (self._my_checking + self._my_savings +
                    self._my_retirement + self._my_moneymarket)

        return accounts[index]

    @ property
    def name(self): return self._name

    @ property
    def age(self): return self._age

    @ property
    def checking(self): return self._my_checking

    @ property
    def savings(self): return self._my_savings

    @ property
    def retirement(self): return self._my_retirement

    @ property
    def mmf(self): return self._my_moneymarket
