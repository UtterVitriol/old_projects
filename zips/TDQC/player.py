"""Handles unique players for guess.py"""

import os


class Player:

    """Creates and tracks stats for players for guess.py

       Creates ./player_data directory if it doesn't exists.
       Stores player stats in player files named: "player name".txt -
       -in the ./player_data directory
    """

    def __init__(self, name):  # Creates the dir and makes the player

        """Attribute name is the name of the player"""

        self._dir = "player_data"  # DRY DRY DRY DRY DRY XD
        self._name = name
        self._path = f"{self._dir}/{self._name}"

        if not os.path.exists(self._dir):
            os.mkdir(self._dir)

        if os.path.exists(self._path):
            with open(self._path) as file:
                self._data = file.readlines()

            self._player_stats = [int(len(self._data)),
                                  sum([int(x.split()[0]) for x in self._data]),
                                  sum([int(x.split()[1]) for x in self._data])]
        else:
            with open(self._path, "w") as file:
                file.write("")
            self._player_stats = [0, 0, 0]

    def _get_new(self, good_guesses, bad_guesses):

        """Updates player stats"""

        self._player_stats[0] += 1
        self._player_stats[1] += good_guesses
        self._player_stats[2] += bad_guesses
        self._player_stats.append(self._player_stats[1] /
                                  self._player_stats[0])
        return self._player_stats

    def _save_data(self, good_guesses, bad_guesses):

        """Writes player stats to player file in ./player_data"""

        with open(self._path, "a") as data:
            line = f"{good_guesses} {bad_guesses}\n"
            data.writelines(line)
