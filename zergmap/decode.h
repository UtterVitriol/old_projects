#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Zerg_h Zerg_h;
typedef struct Zergs Zergs;

bool read_pcap(char *file, Zergs *zergs);

void get_status(FILE *fp, Zerg_h z_head, Zergs *zergs);

void get_message(FILE *fp, Zerg_h z_head);

void get_command(FILE *fp, Zerg_h z_head);

bool get_gps(FILE *fp, Zerg_h z_head, Zergs *zergs);

int check_24bit_sign(int);