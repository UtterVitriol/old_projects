#!/usr/bin/env python3
"""See input_validate"""


def input_validate(arg):

    """Handles input validation

       Takes list arg
       Returns:
       -- False if bad input
       -- None if no input
       -- "random" if -R input
       -- tuple of inputs if valid
    """
    size_of_arg = len(arg)
    error = ("Usage: ./bottles.py number(1-99) word(a beverage)\n"
             "   Or: ./bottles.py -R\n"
             "   Or: ./bottles.py -d\n\n"
             "Examples:\n"
             "./bottles.py 23 DrPepper\n"
             "./bottles.py -R\n"
             "./bottles.py -d")

    if size_of_arg > 3:
        print(error)
        return False

    elif size_of_arg == 3 and arg[1].isdigit() and arg[2].isalpha():
        num = int(arg[1])
        if num not in range(1, 100):
            print(error)
            return False
        else:
            return (int(arg[1]), arg[2])

    elif size_of_arg == 2:
        if arg[1] == "-R":
            return "random"

        elif arg[1] == "-d":
            return "pep"

        else:
            print(error)
            return False

    elif size_of_arg == 1:
        return None

    else:
        print(error)
        return False
