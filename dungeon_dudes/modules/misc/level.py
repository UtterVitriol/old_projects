"""level generator for dungeon dudes"""


from modules.entities.monster import Monster
import random


class LevelGenerator:
    """handles generating levels with monsters"""

    # list of levels and their descriptions
    _levels = [["Water Room", "A room full of puddles."],
               ["Moss Room", "A room full of moss."],
               ["Dungeon", "A room full of cages."],
               ["Family Room", "A room with a TV and a couch"],
               ["Torture Room", "Wooden Horses everywhere."]]

    def __init__(self, monster_count, show_rolls, first_load=True):
        """initializes variables for class"""

        if first_load:  # check for loading .dd_monsters file
            self._load_monsters()

        self._name, self._description = random.choice(self._levels)

        # remove level from list so it's not chosen again
        self._levels.pop(self._levels.index([self._name, self._description]))
        self._monster_count = monster_count
        self._monsters = []
        self._polulate_monsters(show_rolls)
        self._current_monster = self._monsters[0]
        self._has_monster = True
        self._loot = []
        self._loot_dropped = False
        self._last_loot = ""

    def _polulate_monsters(self, show_rolls):
        """populates level with random monsters"""

        for num in range(0, self._monster_count):
            monster = Monster(show_rolls)
            self._monsters.append(monster)

    def next_level(self, monster_count, show_rolls):
        """creates new level"""

        if len(self._levels) == 0:
            return False

        self.__init__(monster_count, show_rolls, False)
        return True

    def check_monster(self):
        """checks if current monster is dead"""

        if self._current_monster.health == 0:
            self._kill_monster()
            return True

        return False

    def _kill_monster(self):
        """gets loot from current monster
           sets current monster to next monster
        """

        self._monster_count -= 1

        loot = self._current_monster.drop_loot()

        if loot is not None:
            self._loot_dropped = True
            self._last_loot = loot
            self._loot.append(loot)
        else:
            self._loot_dropped = False

        if self._monster_count <= 0:
            self._has_monster = False
            return
        else:
            self._next_moster()

    def _next_moster(self):
        """gets next monster from list"""

        self._monsters.pop(0)
        self._current_monster = self._monsters[0]

    def print_dialogue(self):
        """prints description of level"""

        dialogue = f"You enter a {self._name}, {self._description}"
        print(dialogue)

    def add_loot(self, loot):
        """adds loot to level from monster"""

        self._loot.append(loot)

    def remove_loot(self, loot):
        """removes loot from level"""

        self._loot.pop(self._loot.index(loot))

    def _load_monsters(self):
        """loads monster from .dd_monsters file if it exists"""

        try:
            with open("data/.dd_monsters", "r") as file:
                new_monsters = file.readlines()

            for index, _ in enumerate(new_monsters):
                new_monsters[index] = new_monsters[index].strip()
                new_monsters[index] = new_monsters[index].split()
                new_monsters[index][1] = int(new_monsters[index][1])
                new_monsters[index][2] = int(new_monsters[index][2])

            Monster._monster_list = new_monsters
        except FileNotFoundError:
            pass

    @ property
    def name(self): return self._name

    @ property
    def description(self): return self._description

    @ property
    def has_monster(self): return self._has_monster

    @property
    def current_monster(self): return self._current_monster

    @property
    def loot(self): return self._loot

    @property
    def loot_dropped(self): return self._loot_dropped

    @property
    def last_loot(self): return self._last_loot

    @property
    def has_loot(self):
        if len(self.loot) > 0:
            return True

        return False
