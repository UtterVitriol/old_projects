// function definitions for checksm.c

#pragma once

uint16_t sum_command(struct command_h);

uint16_t sum_message(struct message_h);

uint16_t sum_gps(struct gps_h);

uint16_t sum_status(struct status_h, char *);

uint16_t udp_ipv6_checksum(struct udp_h, struct ip6_h, struct zerg_h, uint16_t);

uint16_t udp_ipv4_checksum(struct udp_h, struct ip4_h, struct zerg_h, uint16_t);

uint16_t ipv4_checksum(struct ip4_h);
