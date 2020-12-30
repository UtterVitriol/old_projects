#!/usr/bin/env python3

"""Sings 99 Bottles of Beer"""

import sys
import time
import validate
import random


def int_to_string(num):

    """Turns integers into their words

        Takes one argument of type int
        Returns the word for that int as str
    """
    ones = {0: "", 1: "One", 2: "Two", 3: "Three", 4: "Four", 5: "Five",
            6: "Six", 7: "Seven", 8: "Eight", 9: "Nine", 10: "Ten",
            11: "Eleven", 12: "Twelve", 13: "Thirteen", 14: "Fourteen",
            15: "Fifteen", 16: "Sixteen", 17: "Seventeen",
            18: "Eighteen", 19: "Nineteen"}

    tens = {2: "Twenty", 3: "Thirty", 4: "Forty", 5: "Fifty", 6: "Sixty",
            7: "Seventy", 8: "Eighty", 9: "Ninety"}

    if num > 19:
        num_str = str(num)
        ones_place = int(num_str[0])
        tens_place = int(num_str[1])
        if tens_place == 0:
            num_word = f"{tens[ones_place]}"
        else:
            num_word = f"{tens[ones_place]}-{ones[tens_place].lower()}"

    elif num < 20:
        num_word = ones[num]

    return num_word


def sing(start=99, beverage="beer"):

    """Prints song 99 Bottles of Beer

       Optional arguments:
       start -- number to start from in the song (default 99)
       beverage -- beverage used in the song (default "beer")
    """
    bottles = ["bottles", "bottle"]

    lines = ["{} {} of {} on the wall!\n",
             "{} {} of {}!\n",
             "Take one down\nAnd pass it around\n",
             f"No more bottles of {beverage} on the wall"]

    for num in list(range(1, start+1))[::-1]:
        num_word = int_to_string(num)  # Integer as word
        next_num_word = int_to_string(num-1)  # Next integer as word

        if num > 2:
            print(lines[0].format(num_word, bottles[0], beverage),
                  lines[1].format(num_word, bottles[0], beverage),
                  lines[2],
                  lines[0].format(next_num_word, bottles[0], beverage), sep="")

        elif num == 2:
            print(lines[0].format(num_word, bottles[0], beverage),
                  lines[1].format(num_word, bottles[0], beverage),
                  lines[2],
                  lines[0].format(next_num_word, bottles[1], beverage), sep="")

        else:
            print(lines[0].format(num_word, bottles[1], beverage),
                  lines[1].format(num_word, bottles[1], beverage),
                  lines[2],
                  lines[3], sep="")


def main(arg):

    """Takes list arg, Returns boolean on success of execution"""

    options = validate.input_validate(arg)  # Input validation

    if options is None:  # No command line arguments
        sing()

    elif options is False:  # Bad command line arguments
        return False

    elif options == "random":
        sing(random.randint(1, 99))

    elif options == "pep":
        sing(beverage="authentic tasting Dr. Pepper w/ cane sugar")

    else:
        sing(*options)

    return True


if __name__ == "__main__":
    start = time.perf_counter()  # Time at start of program
    success = main(sys.argv)
    end = time.perf_counter()  # Time at end of program
    if success:  # Prints time taken to run if sing executed successfully
        print(f"\nThis program ran in {end - start:0.4f} seconds.")
