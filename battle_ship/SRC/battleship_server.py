# /user/bin/env python3
# https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
# https://realpython.com/python-sockets/
'''Battleship online server

   Will start save game if valid
   Will start new game if no valid save game

   Does not currently have checking for previous player 1 & 2
   Will assign player one to client that connects first
'''

import socket
import os
import random
from time import sleep


class Player:

    '''Stores player information for game
    '''

    def __init__(self, health, x_axis, y_axis):
        self.health = health
        self.h_val = int(health)
        self.x = x_axis
        self.y = y_axis

    def _set_grids(self, grids):
        self.grids = grids

    def _set_turn(self, turn):
        self.turn = turn


class Serv:

    '''Battleship online server
       Handles receiving and sending information from client to client

       Init with no args and start with start method, also with no args

       creates ~/.save file to store save games
    '''

    def __init__(self):
        print("Starting server...")

        # try to get host address
        self.host = socket.gethostbyname(socket.gethostname())
        if self.host == '127.0.0.1':
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 1))
            self.host = s.getsockname()[0]
            s.close()

        print("Sever ip:", self.host)

        self.has_save = self._check_save()
        self.port = 6666

    def _check_save(self):

        '''Checks for valid save game

           Reads contents of save game

           returns 1 if valid save game
           returns 0 if invalid save game
        '''

        try:  # read save game
            with open(os.getenv("HOME") + "/.save", "r") as f:
                text = f.read()

            print("Checking save game...")

            text = text.split("\n")
            stats = text[0].split(" ")
            text = text[1:]
            x_axis = stats[0]
            y_axis = stats[1]
            y = int(y_axis)
            one_hp = stats[2]
            two_hp = stats[3]
            turn = stats[4]

            # current save game has ended
            if int(one_hp) < 1 or int(two_hp) < 1:
                print("Save game over...")
                print("Starting new game...")
                return 0

            # populate players with stats and maps
            print("Loading save game...")
            self.p_one = Player(stats[2], stats[0], stats[1])
            self.p_two = Player(stats[3], stats[0], stats[1])
            self.p_one._set_grids("\n".join(text[0:y*2]))
            self.p_two._set_grids("\n".join(text[y*2:y*4]))

            # set players turn
            if stats[4] == "0":
                self.p_one._set_turn("0")
                self.p_two._set_turn("1")
            elif stats[4] == "1":
                self.p_one._set_turn("1")
                self.p_two._set_turn("0")
            else:
                print("Error with save file...")
                print("Starting new game...")
            return 1

        except Exception as e:  # Save file does not exist or permissions error
            print("Error reading save file....")
            print("Starting new game...")
            return 0

    def serv(self):

        '''Connects clients

           Sends player number and if new game or save game
        '''

        print("Starting listener...\n")
        dot = "."
        count = 0

        # create socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        # wait for socket in case timewait
        while True:
            try:
                # bind socket to port
                s.bind((self.host, self.port))
                break
            except Exception:
                print("Waiting on socket" + dot * count)
                count += 1
                sleep(2)

        # listen
        s.listen(5)

        print("Wating for players to connect....\n")

        try:
            # player one connection
            conn1, addr1 = s.accept()
            self.conn1 = conn1
            print("Player_1 connected from: ", addr1, "\n")

            # player two connection
            conn2, addr2 = s.accept()
            self.conn2 = conn2
            print("Player_2 connected from: ", addr2, "\n")

            # close socket after two players connect

            s.close()

            # give time for clients to load
            sleep(.5)

            # send player number to clients
            conn1.sendall(b"1")
            conn2.sendall(b"2")

            print("Waiting for players to start...")

            # send 2 if valid save, 1 if not
            if self.has_save:
                conn1.sendall(b"2")
                conn2.sendall(b"2")
                sleep(.5)
                self._load_game(conn1, conn2)
            else:
                conn1.sendall(b"1")
                conn2.sendall(b"1")
                self._new_game(conn1, conn2)

        except Exception as e:  # connection error
            print("Player disconnected unexpectedly...")
            conn1.close()
            conn2.close()
            exit(1)

    def _disconnected(self, conn1, conn2, data1, data2):
        '''Checks for player disconnect
        '''

        if(data1.decode() == "1"):
            # send player 2, player 1 disconnected
            print("Player_1 disconnected...")
            conn2.sendall(b"1")
            return 1
        else:
            conn2.sendall(b"0")

        if(data2.decode() == "1"):
            # send player 1, player 2 disconnected
            print("Player_2 disconnected...")
            conn1.sendall(b"1")
            return 1
        else:
            conn1.sendall(b"0")

        return 0

    def _game_loop(self, conn1, conn2, p1, p2):
        '''Game loop

           Sends coords and hit or misses back and forth
           Checks for disconnect and calls save_game
        '''

        players = [self.p_one, self.p_two]

        c_player1 = int(p1) - 1
        c_player2 = int(p2) - 1

        # turn one recieve coords from player
        data = conn1.recv(1024)

        # check for disconnect
        if(data.decode() == "3"):
            print("Player_" + p1, "disconnected...")
            conn2.sendall(b"100")
            return
        elif(data.decode() == ""):
            print("Player_" + p1, "disconnected...\n")
            conn2.sendall(b"100")
            return

        print("Player_" + p1, "sent coords: " + data.decode())

        # send coords to player
        conn2.sendall(data)

        # loop rest of game
        while True:

            # recieve hit, miss or win from player
            data = conn2.recv(1024)

            # check hit, miss, win or disconnect
            if data.decode() == "1":
                players[c_player2].h_val -= 1
                print("Player_" + p1, "hit!")

            elif data.decode() == "2":
                print("Player_" + p1, "missed!")

            elif data.decode() == "3":
                conn1.sendall(data)
                print("Player_" + p1, "WINS!")
                self._end_game()
                break

            else:
                print("Player_" + p2, "Disconnected...\n")
                conn1.sendall(b"100")
                break

            # send hit, miss or win to player 1
            conn1.sendall(data)

            # recieve coords from player 2
            data = conn2.recv(1024)

            # check for disconnect
            if(data.decode() == "3"):
                print("Player_" + p2,  "Disconnected...")
                conn1.sendall(b"100")
                self._save_game(conn1, conn2, int(p2) - 1)
                return

            elif(data.decode() == ""):
                print("Player_" + p2,  "Disconnected...\n")
                conn1.sendall(b"100")
                return

            print("Player_" + p2, "sent coords: " + data.decode())

            # send coords to player 1
            conn1.sendall(data)

            # recieve hit, miss or win from player 1
            data = conn1.recv(1024)

            # check hit, miss, win or disconnect
            if data.decode() == "1":
                players[c_player1].h_val -= 1
                print("Player_" + p2,  "hit!")

            elif data.decode() == "2":
                print("Player_" + p2, "missed!")

            elif data.decode() == "3":
                conn2.sendall(data)
                print("Player_" + p2, "WINS!")
                self._end_game()
                break

            else:
                print("Player_" + p1, "Disconnected...\n")
                conn2.sendall(b"100")
                break

            # send hit, miss or win to player 2
            conn2.sendall(data)

            # recieve coords from player 1
            data = conn1.recv(1024)

            # check for disconnect
            if(data.decode() == "3"):
                print("Player_" + p1, "disconnected...")
                conn2.sendall(b"100")
                self._save_game(conn1, conn2, int(p1) - 1)
                return

            elif(data.decode() == ""):
                print("Player_" + p1, "disconnected...\n")
                conn2.sendall(b"100")
                return

            print("Player_" + p1, "sent coords: " + data.decode())

            # send coords to player 2
            conn2.sendall(data)

    def _new_game(self, conn1, conn2):
        '''On invalid save file

           Starts new game
        '''

        # generate random map size
        random.seed()
        x = str(random.randint(10, 25))
        y = str(random.randint(10, 25))
        coords = f"{x} {y}"

        self.p_one = Player("19", x, y)
        self.p_two = Player("19", x, y)
        self.p_one._set_turn("0")
        self.p_two._set_turn("1")

        print("Sending generated x/y coords: " + coords + "\n")

        # give time for clients to load
        sleep(.5)

        # send map size to clients
        conn1.sendall(coords.encode())
        conn2.sendall(coords.encode())

        print("Coords sent...\nWaiting on players...")

        # wait for players to place boats
        data1 = conn1.recv(1024)
        data2 = conn2.recv(1024)

        # check for disconnect
        if self._disconnected(conn1, conn2, data1, data2):
            conn1.close()
            conn2.close()
            return

        # give time for clients to load
        sleep(.5)

        print("Players are ready\n")

        # send clients start signal
        conn1.sendall(b"1")
        conn2.sendall(b"2")
        self._game_loop(conn1, conn2, "1", "2")

    def _load_game(self, conn1, conn2):
        '''On valid load

           Load save game
        '''

        coords = f"{self.p_one.x} {self.p_one.y}"

        # send size of map to clients
        print("Sending players map size...")
        conn1.sendall(coords.encode())
        conn2.sendall(coords.encode())
        sleep(.2)

        # send health
        print("Sending players health...")
        conn1.sendall((self.p_one.health).encode())
        conn2.sendall((self.p_two.health).encode())
        sleep(.2)

        # send turn
        print("Sending players turn...")
        conn1.sendall((self.p_one.turn).encode())
        conn2.sendall((self.p_two.turn).encode())
        sleep(.2)

        # send grids
        print("Sending players maps...")
        conn1.sendall((self.p_one.grids).encode())
        conn2.sendall((self.p_two.grids).encode())

        # start game
        print("Starting game...")
        if(self.p_one.turn == "0"):
            self._game_loop(conn1, conn2, "1", "2")
        else:
            self._game_loop(conn2, conn1, "2", "1")

    def _save_game(self, conn1, conn2, turn):
        '''On disconnect or gameover

           Save game
        '''

        print("Saving game...")

        # get maps from clients
        print("Receiving maps from clients")
        data1 = self.conn1.recv(2601)
        data2 = self.conn2.recv(2601)
        data1 = data1.decode()
        data2 = data2.decode()

        x = self.p_one.x
        y = self.p_one.y
        p1 = self.p_one.h_val
        p2 = self.p_two.h_val
        stats = f"{x} {y} {p1} {p2} {turn}\n"

        # write save file
        print("Writing save....")
        with open(os.getenv("HOME") + "/.save", "w") as f:
            f.write(stats)
            f.write(data1)
            f.write(data2)

        print("Saved game...\n")
        return

    def _end_game(self):
        ''' Changes save game stats to 0's
        '''

        print("Ending game....")
        with open(os.getenv("HOME") + "/.save", "w") as f:
            f.write("0 0 0 0 0")


def main():
    # It's main...

    while True:
        server = Serv()
        server.serv()


if __name__ == "__main__":
    main()
