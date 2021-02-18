/* Decodes zerg packets from a .pcap file and displays relevant
 * information in a human readable format.
 *
 * Supports ipv4 and ipv6,
 * little and big endian,
 * vlan taggin,
 * 4in6
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#include "structs.h"
#include "zergmap.h"
#include "decode.h"

bool read_pcap(char *file, Zergs *zergs)
{
	/* reads global header
	 * loops through reading the pcap header and packets
	 * skips irrelevant packets
	 * calls zerg payload functions depending on the zerg header type
	 */

	FILE *fp;

	fp = NULL;
	fp = fopen(file, "rb");

	if (!fp) {
		fprintf(stderr, "File: \"%s\", does not exist...\n", file);
		return false;
	}

	Global_h global;

	global.magic = 0;

	// read global header
	fread(&global, GLOBALH, 1, fp);

	global.magic = ntohl(global.magic);

	int big_endian = 0;

	// check for magic number and endianness
	if (global.magic == 0xa1b2c3d4) {
		big_endian = 1;
	} else if (global.magic == 0xd4c3b2a1) {
		;
	} else {
		fprintf(stderr, "File: \"%s\", is not a pcap...\n", file);
		fclose(fp);
		return false;
	}

	Pcap_h pcap;
	Ether_h ether;
	Ip6_h ip6;
	Ip4_h ip4;
	Udp_h udp;
	Zerg_h z_head;
	unsigned long start, end, total;
	int ip_len = 0;

	/* loops through reading pcap headers and packets
	 * calls functions corresponding to zerg header type field
	 */

	while (fread(&pcap, PCAPH, 1, fp) != 0) {

		// swap pcap.len bytes if pcap is big endian
		if (big_endian) {
			pcap.len = ntohl(pcap.len);
		}

		// grab the starting position of file pointer in the loop
		start = ftell(fp);

		// get the ending byte location expected at end of loop
		total = start + pcap.len;

		// read ethernet frame
		fread(&ether, 1, ETHERH, fp);

		// check for 802.1q tagging
		if (htons(ether.type) == 0x8100) {

			// 802.1 tagging, skip 2 bytes and read ether type
			fseek(fp, 2, SEEK_CUR);
			fread(&ether.type, 1, 2, fp);
		}

		// check for ipv4
		if (htons(ether.type) == 0x800) {

			// read ipv4 header
			fread(&ip4, 1, IP4H, fp);

			// set ip_len based on ipv4 header length
			ip_len = ip4.ihl * 4;

			if (ip_len > 20) {
				fseek(fp, ip_len - 20, SEEK_CUR);
			}

			// check for udp, skip if not udp
			if (ip4.prot != 17) {
				fprintf(stderr, "Skipping Non-UDP packet\n\n");
				fseek(fp, pcap.len - (ETHERH + ip_len),
				      SEEK_CUR);
				continue;
			}

			// check for ipv6
		} else if (htons(ether.type) == 0x86dd) {

			// set ip_len based on ipv6 header length
			ip_len = IP6H;

			// read ipv6 header
			fread(&ip6, 1, IP6H, fp);

			if (ip6.ver == 4) {
				// 4in6

				fseek(fp, -IP6H, SEEK_CUR);
				// read ipv4 header
				fread(&ip4, 1, IP4H, fp);

				// set ip_len based on ipv4 header length
				ip_len = ip4.ihl * 4;

				if (ip_len > 20) {
					fseek(fp, ip_len - 20, SEEK_CUR);
				}

				// check for udp, skip if not udp
				if (ip4.prot != 17) {
					fprintf(stderr,
						"Skipping Non-UDP packet\n\n");
					fseek(fp, pcap.len - (ETHERH + ip_len),
					      SEEK_CUR);
					continue;
				}
			} else {
				// check for udp, skip if not udp
				if (ip6.prot != 17) {
					fprintf(stderr,
						"Skipping Non-UDP packet\n\n");
					fseek(fp, pcap.len - (ETHERH + IP6H),
					      SEEK_CUR);
					continue;
				}
			}
		}

		// read udp header
		fread(&udp, 1, UDPH, fp);

		// check destination port for 3751, skip if not 3751
		if (htons(udp.dport) != 3751) {
			fprintf(stderr, "Skipping wrong Dst Port packet\n\n");
			fseek(fp, pcap.len - (ETHERH + ip_len + UDPH),
			      SEEK_CUR);
			continue;
		}

		// read zerg header
		fread(&z_head, 1, ZERGH, fp);

		// check for version 1, skip if not version 1.
		if (z_head.ver != 1) {
			fseek(fp, pcap.len - (ETHERH + ip_len + UDPH + ZERGH),
			      SEEK_CUR);
			fprintf(stderr, "Skipping Bad Zerg Version packet\n\n");
			continue;
		}

		// flip src for use later
		z_head.src = htons(z_head.src);

		// call zerg payload function based on zerg header type
		// field
		switch (z_head.type) {
		case (0):
			get_message(fp, z_head);
			break;
		case (1):
			get_status(fp, z_head, zergs);
			break;
		case (2):
			get_command(fp, z_head);
			break;
		case (3):
			if (!get_gps(fp, z_head, zergs)) {
				fclose(fp);
				return false;
			}
			break;
		}

		// get position in file at end of loop
		end = ftell(fp);

		// check if read expected number of bytes, fseek to correct
		if (end != (unsigned long)(total)) {

			// didn't read enough
			if (end < (unsigned long)(total)) {
				fseek(fp, (total - end), SEEK_CUR);

				// read too much
			} else if (end > (unsigned long)(total)) {
				fseek(fp, -(end - total), SEEK_CUR);
			}
		}
	}

	fclose(fp);

	return true;
}

void get_status(FILE *fp, Zerg_h z_head, Zergs *zergs)
{
	/*
	 * Read status payload, add zerg
	 */

	Status_h status;

	// read status header
	fread(&status, 1, STATH, fp);

	// get size of name
	int name_size = htonl(z_head.len << 8) - (ZERGH + STATH);

	// check if there is a name, display **NO_NAME** if no name
	if (name_size > 0) {
		fseek(fp, name_size, SEEK_CUR);
	}

	uint16_t source = z_head.src;

	Zerg *zerg = create_zerg(source);

	zerg->health = check_24bit_sign(htonl(status.hp << 8));
	zerg->max_health = htonl(status.mhp << 8);

	add_zerg(zergs, zerg, false);
}

bool get_gps(FILE *fp, Zerg_h z_head, Zergs *zergs)
{
	/*
	 * read gps payload, add zerg if valid data
	 */

	double lon, lat;
	float alt, acc;

	Gps_h gps;

	// read gps header
	fread(&gps, 1, GPSH, fp);

	// convert binary to double
	gps.lon = __builtin_bswap64(gps.lon);
	memcpy(&lon, &gps.lon, 8);

	// convert binary to double
	gps.lat = __builtin_bswap64(gps.lat);
	memcpy(&lat, &gps.lat, 8);

	// convert binary to float
	gps.alt = htonl(gps.alt);
	memcpy(&alt, &gps.alt, 4);

	// convert binary to float
	gps.acc = htonl(gps.acc);
	memcpy(&acc, &gps.acc, 4);

	// check for invalid lat
	if (lat > 90 || lat < -90) {
		fprintf(stderr, "Invalid Latitude...\n");
		return true;
	}

	// check for invalid lon
	if (lon > 180 || lon < -180) {
		fprintf(stderr, "Invalid Longitude...\n");
		return true;
	}

	// convert to m from fathoms
	alt = (alt * 1.8288);

	// check if altitude is outside atmosphere or past the center of the
	// earth
	if (alt > 100000 || alt < -6370000) {
		fprintf(stderr, "Invalid Altitude...\n");
		return true;
	}

	Zerg *zerg = NULL;

	zerg = create_zerg(z_head.src);
	zerg->lat = lat;
	zerg->lon = lon;
	zerg->alt = alt;
	zerg->acc = acc;
	zerg->health = INT_MAX;
	zerg->max_health = INT_MAX;

	// conflicting gps coordinates
	if (!add_zerg(zergs, zerg, true)) {
		return false;
	}

	return true;
}

int check_24bit_sign(int num)
{
	// checks for negative 24 bit ints and returns them as 32 bit ints

	int bit = 0;

	// get sign bit
	bit = (num >> 23) & 1U;

	// check for negative, return negative 32 bit
	if (bit == 1) {
		num += 0xff000000;
		return num;
	}

	// sign bit not set
	return num;
}

void get_message(FILE *fp, Zerg_h zerg)
{
	/*
	 * Skip message payloads, they are irrelevant for this project
	 */

	int size;

	// get message size
	size = htonl(zerg.len << 8) - ZERGH;

	// check if there is a message
	if (size > 0) {
		fseek(fp, size, SEEK_CUR);
	}
}

void get_command(FILE *fp, Zerg_h zerg)
{
	/*
	 * Skip command payloads, they are irrelevant for this project
	 */

	int size;

	// get command header size
	size = htonl(zerg.len << 8) - ZERGH;

	// read command header
	fseek(fp, size, SEEK_CUR);
}
