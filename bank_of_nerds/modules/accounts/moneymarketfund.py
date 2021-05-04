"""MoneyMarketFund account class.

   Inherits from Account class.
"""

from modules.accounts.account import Account, OverDraftError


class TransactionLimitError(Exception):
    """Too many transactions in a month."""
    pass


class MoneyMarketFund(Account):
    """MoneyMarketFund account class."""

    def __init__(self, amount, owner):
        """Initialize variables for class."""

        super().__init__(amount, owner)
        self._transactions = 0
        self._error = "Transaction limit for the month"
        self._interest_rate = .45
        self._details["Interest_Rate"] = self._interest_rate

    def withdrawal(self, amount):
        """Withdrawl amount, raise error on failure, return True on success."""

        if self._transactions == 2:
            raise TransactionLimitError(self._error)

        if self._balance - amount < 0:
            raise OverDraftError("Cannot withdrawal below 0.00.")

        self._transactions += 1
        self._balance = self._balance - amount
        self._details["Balance"] = self._balance

        return True

    def deposit(self, amount):
        """Deposit amount, raise error on failure."""

        if self._transactions == 2:
            raise TransactionLimitError(self._error)

        self._transactions += 1
        self._balance += amount
        self._details["Balance"] = self._balance

    @property
    def interest_rate(self): return self._interest_rate
