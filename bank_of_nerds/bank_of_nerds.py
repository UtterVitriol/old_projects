#!/usr/bin/env python3

"""Bank of Nerd!

   Run this file to use the bank!
   Creates two default customers, "Diet Dr. Pepper" and "Is Better"
   Run with --secret backdoor to print the default
       -customers details in the start menu
"""

import argparse

from modules.bank import Bank


class ArgParseError(Exception):
    """For bad command line arguments"""
    pass


def main():
    """Parse command line arguments, run bank."""

    try:
        parser = argparse.ArgumentParser()
        parser.add_argument("--secret", help="Secret")
        args = parser.parse_args()
    except SystemExit:
        raise ArgParseError("Bad Command Line Argument.")

    show_defaults = False

    if args.secret == "backdoor":
        show_defaults = True

    bank = Bank(show_defaults)
    bank.start()


if __name__ == "__main__":
    try:
        main()
    except ArgParseError:
        pass  # bad command line argument
    except (Exception, KeyboardInterrupt, SystemExit, GeneratorExit) as e:
        print(e)
