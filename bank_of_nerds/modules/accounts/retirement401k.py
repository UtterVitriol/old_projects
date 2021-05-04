"""Retirement401k account class.

   Inherits from Account class.
"""

from modules.accounts.account import Account, OverDraftError


class CustomerAgeError(Exception):
    """Customer age is invalid"""
    pass


class Retirement401k(Account):
    """Retirement401k account class."""

    def __init__(self, amount, owner):
        """Initialize variables for class."""

        super().__init__(amount, owner)
        self._interest_rate = .06
        self._details["Interest_Rate"] = self._interest_rate

    def withdrawal(self, amount):
        """Withdrawl amount, raise error on failure, return True on success."""

        if self.owner.age < 67:
            raise CustomerAgeError(
                "Cannot withdrawal from 401k before age 67.")

        if self._balance - amount < 0:
            raise OverDraftError("Cannot withdrawal below 0.00.")

        self._balance = self._balance - amount
        self._details["Balance"] = self._balance

        return True

    @property
    def interest_rate(self): return self._interest_rate
