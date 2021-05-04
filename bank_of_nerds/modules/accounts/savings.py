"""Savings account class.

   Inherits from Account class.
"""

from modules.accounts.account import Account


class Savings(Account):
    """Savings account class."""

    def __init__(self, amount, owner):
        """Initialize variables for class."""

        super().__init__(amount, owner)

        self._interest_rate = .05
        self._details["Interest_Rate"] = self._interest_rate

    @property
    def interest_rate(self): return self._interest_rate
