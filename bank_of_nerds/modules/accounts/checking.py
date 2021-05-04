"""Checking account class.

   Inherits from Account class.
"""

from modules.accounts.account import Account


class MaxOverDraftError(Exception):
    """Over draft exceeds negative 1000."""
    pass


class Checking(Account):
    """Checking account class."""

    def __init__(self, amount, owner):
        """Initialize variables for class."""

        self._overdraft_fee = 35
        super().__init__(amount, owner)

    def withdrawal(self, amount):
        """Withdrawl amount, raise error on failure, return True on success."""

        if self._balance - amount < 0:

            # check if overdraft will put account below negative 1000
            if self._balance - (self._overdraft_fee + amount) < -1000:
                raise MaxOverDraftError("Max Overdraft")
            else:
                amount += self._overdraft_fee

        self._balance = self._balance - amount

        return True
