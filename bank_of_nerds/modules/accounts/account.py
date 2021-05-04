"""Base class for all account types in bank of nerds."""

from datetime import date
from abc import ABC, abstractmethod


class OverDraftError(Exception):
    """Error for withdrawling too much."""
    pass


class Account(ABC):
    """Abstract account for bank of nerds."""

    _account_number = 6666

    @abstractmethod
    def __init__(self, init_deposit, owner):
        """Initialize variables for class."""

        self._owner = owner
        self._date_created = str(date.today())
        self._balance = init_deposit
        self._account_number = Account._account_number

        # store account class name as string
        self._account_type = str(type(self)).split(".")[3].split("'")[0]

        Account._account_number += 1

        self._details = {"Account_Type": self._account_type,
                         "Account_Number": self._account_number,
                         "Created": self._date_created,
                         "Balance": self._balance}

    def withdrawal(self, amount):
        """Withdrawl amount, raise error on failure, return True on success."""

        if self._balance - amount < 0:
            raise OverDraftError("Cannot withdrawal below 0.00.")

        self._balance = self._balance - amount
        self._details["Balance"] = self._balance

        return True

    def deposit(self, amount):
        """Deposit amount."""

        self._balance += amount
        self._details["Balance"] = self._balance

    def get_details(self):
        """Print account details."""

        for key, value in self._details.items():
            if isinstance(value, float):
                value = f"{value:.2f}"

            print(key, "->", value)

    @property
    def owner(self): return self._owner

    @property
    def account_type(self): return self._account_type

    @ property
    def account_number(self): return self._account_number

    @ property
    def balance(self): return f"{self._balance:.2f}"

    @ property
    def date_created(self): return self._date_created
