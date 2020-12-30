/*  Decodes zerg packets from a .pcap file and displays relevant
    information in a human readable format.
    
    Supports ipv4 and ipv6.
    
    Will display utf-8 characters,
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "structs.h"


void read_pcap(char *file);

void get_status(FILE *, struct zerg_h);

void get_message(FILE *, struct zerg_h);

void get_command(FILE *, struct zerg_h);

void get_gps(FILE *);

int check_24bit_sign(int);


int main(int argc, char **argv)
{	
	// checks for args and calls read_pcap

	if(argc == 1)
	{
		puts("Please provide a pcap file...");
		exit(1);
	}else{
		for(int i = 1; i < argc; i++){
			read_pcap(argv[i]);
		}
	}
}

void read_pcap(char *file)
{
	/* reads global header
	 * loops through reading the pcap header and packets
	 * skips irrelevant packets
	 * calls zerg payload functions depending on the zerg header type
	 */
	 
	FILE *fp;
	
	fp = fopen(file, "rb");
	
	if(fp == NULL){
		printf("File: \"%s\", does not exist...\n", file);
		exit(1);
	}
	
	struct global_h global;
	
	// read global header
	fread(&global, GLOBALH, 1, fp);
	
	global.magic = ntohl(global.magic);
	
	int big_endian = 0;
	
	// check for magic number and endianness
	if(global.magic == 0xa1b2c3d4){
		big_endian = 1;
	}else if(global.magic == 0xd4c3b2a1){
		;
	}else{
		printf("File: \"%s\", is not a pcap...\n", file);
		fclose(fp);
		exit(1);
	}
	
	struct pcap_h pcap;
	struct ether_h ether;
	struct ip6_h ip6;
	struct ip4_h ip4;
	struct udp_h udp;
	struct zerg_h zerg;
	unsigned long start, end, total;
	int ip_len = 0;
	
	/* loops through reading pcap headers and packets
	 * calls functions corresponding to zerg header type field
	 */
	 
	while(fread(&pcap, PCAPH, 1, fp) != 0){
		
		// swap pcap.len bytes if pcap is big endian
		if(big_endian){
			pcap.len = ntohl(pcap.len);
		}
		
		// grab the starting position of file pointer in the loop
		start = ftell(fp);
		
		// get the the ending byte location expected at end of loop
		total = start + pcap.len;
		
		// read ethernet frame
		fread(&ether, 1, ETHERH, fp);
		
		// check for 802.1q tagging
		if(htons(ether.type) == 0x8100){
			
			// 802.1 tagging, skip 2 bytes and read ether type
			fseek(fp, 2, SEEK_CUR);
			fread(&ether.type, 1, 2, fp);
		}
		
		// check for ipv4
		if(htons(ether.type) == 0x800){
			
			//read ipv4 header
			fread(&ip4, 1, IP4H, fp);
			
			// set ip_len based on ipv4 header length
			ip_len = ip4.ihl * 4;
			
			if(ip_len > 20){
				fseek(fp, ip_len - 20, SEEK_CUR);
			}
			
			// check for udp, skip if not udp
			if(ip4.prot != 17){
				fprintf(stderr, "Skipping Non-UDP packet\n\n");
				fseek(fp, pcap.len - (ETHERH + ip_len),\
				      SEEK_CUR);
				continue;
			}
		
		// check for ipv6
		}else if(htons(ether.type) == 0x86dd){
			
			// set ip_len based on ipv6 header length
			ip_len = IP6H;
			
			// read ipv6 header
			fread(&ip6, 1, IP6H, fp);
			
			// check for udp, skip if not udp
			if(ip6.prot != 17){
				fprintf(stderr, "Skipping Non-UDP packet\n\n");
				fseek(fp, pcap.len - (ETHERH + IP6H), SEEK_CUR);
				continue;
			}
		}
		
		// read udp header
		fread(&udp, 1, UDPH, fp);
		
		// check destination port for 3751, skip if not 3751
		if(htons(udp.dport) != 3751){
			fprintf(stderr, "Skipping wrong Dst Port packet\n\n");
			fseek(fp, pcap.len - (ETHERH + ip_len + UDPH),\
					      SEEK_CUR);
			continue;
		}
		
		// read zer header
		fread(&zerg, 1, ZERGH, fp);
		
		// check for version 1, skip if not version 1.
		if(zerg.ver != 1){
			fseek(fp, pcap.len - (ETHERH + ip_len + UDPH + ZERGH),\
					      SEEK_CUR);
			fprintf(stderr, "Skipping Bad Zerg Version packet\n\n");
			continue;
		}
		
		printf("Version: %d\n", zerg.ver);
		printf("Sequence: %u\n", htonl(zerg.seq));
		printf("From: %d\n", htons(zerg.src));
		printf("To: %d\n", htons(zerg.dst));
		
		// call zerg payload function based on zerg header type field
		switch(zerg.type){
		case(0):
			get_message(fp, zerg);
			break;
		case(1):
			get_status(fp, zerg);
			break;
		case(2):
			get_command(fp, zerg);
			break;
		case(3):
			get_gps(fp);
			break;
		}
		
		// newline for readability 
		puts("");
		
		// get position in file at end of loop
		end = ftell(fp);

		// check if read expected number of bytes, fseek to correct
		if(end != (unsigned long)(total)){
			
			// didn't read enough
			if(end < (unsigned long)(total)){
				fseek(fp, (total - end), SEEK_CUR);
				
			// read too much
			}else if(end > (unsigned long)(total)){
				fseek(fp, -(end - total), SEEK_CUR);
			}
		}
	}
	
	fclose(fp);
}

void get_status(FILE *fp, struct zerg_h zerg)
{
	char err_name[] = "**NO_NAME**", *name = NULL;
	
	struct status_h status;
	
	// read status header
	fread(&status, 1, STATH, fp);
	
	// get size of name
	int name_size = htonl(zerg.len << 8) - (ZERGH + STATH);
	
	// check if there is a name, display **NO_NAME** if no name
	if(name_size > 0){
		if((name = calloc(name_size + 1, sizeof(char))) == NULL){
			fclose(fp);
			puts("Out of memory");
			exit(1);
		}
		fread(name, 1, name_size, fp);
	}else{
		if((name = malloc(strlen(err_name)  * sizeof(char) + 1)) == NULL){
			fclose(fp);
			puts("Out of memory");
			exit(1);
		}
		strcpy(name, err_name);
	}
	
	// convert binary to float
	float speed;
	status.speed = (htonl(status.speed));
	memcpy(&speed, &status.speed, 4);
	
	char type[11] = {'\0'};
	
	// check for type
	switch(status.type){
	case(0):
		strcpy(type, "Overmind");
		break;
	case(1):
		strcpy(type, "Larva");
		break;
	case(2):
		strcpy(type, "Cerebrate");
		break;
	case(3):
		strcpy(type, "Overlord");
		break;
	case(4):
		strcpy(type, "Queen");
		break;
	case(5):
		strcpy(type, "Drone");
		break;
	case(6):
		strcpy(type, "Zergling");
		break;
	case(7):
		strcpy(type, "Lurker");
		break;
	case(8):
		strcpy(type, "Broodling");
		break;
	case(9):
		strcpy(type, "Hydralisk");
		break;
	case(10):
		strcpy(type, "Guardian");
		break;
	case(11):
		strcpy(type, "Scourge");
		break;
	case(12):
		strcpy(type, "Ultralisk");
		break;
	case(13):
		strcpy(type, "Mutalisk");
		break;
	case(14):
		strcpy(type, "Defiler");
		break;
	case(15):
		strcpy(type, "Devourer");
		break;
	default:
		strcpy(type, "BAD_TYPE");
		break;
	}
	
	printf("Name: %s\n", name);
	printf("HP: %d\n", check_24bit_sign(htonl(status.hp << 8)));
	printf("MaxHP: %d\n", htonl(status.mhp << 8));
	printf("Type: %s\n", type);
	printf("Armor: %d\n", status.armor);
	printf("MaxSpeed: %0.4fm/s\n", speed);
	
	// free name
	free(name);
}

void get_message(FILE *fp, struct zerg_h zerg)
{
	int size;
	
	// get message size
	size = htonl(zerg.len << 8) - ZERGH;
	
	struct message_h msg;
	
	// check if there is a message, display **NO_MESSAGE** if no message
	if(size > 0){
		if((msg.msg = calloc(size + 1, sizeof(char))) == NULL){
			fclose(fp);
			puts("Out of memory");
			exit(1);
		}
		fread(msg.msg, 1, size, fp);
		printf("Message: %s\n", msg.msg);
		free(msg.msg);
	}else{
		printf("Message: **NO_MESSAGE**\n");
	}
}

void get_command(FILE *fp, struct zerg_h zerg)
{
	int size;
	float orientation;
	
	// get command header size
	size = htonl(zerg.len << 8) - ZERGH;
	
	struct command_h command;
	
	// read command header
	fread(&command, 1, size, fp);
	
	// check command type
	switch(htons(command.cmnd)){
	case(0):
		printf("GET_STATUS\n");
		break;
	case(1):
		// convert binary to float
		command.p2 = (htonl(command.p2));
		memcpy(&orientation, &command.p2, 4);
		printf("GOTO\n");
		printf("Orient: %0.3f deg.\n", orientation);
		printf("Distance: %dm\n", htons(command.p1));
		break;
	case(2):
		printf("GET_GPS\n");
		break;
	case(3):
		break;
	case(4):
		printf("RETURN\n");
		break;
	case(5):
		printf("SET_GROUP\n");
		
		// check if add/remove
		switch(htons(command.p1)){
		case(1):
			printf("AddTo: %d\n", htonl(command.p2));
			break;
		case(0):
			printf("RmFrom: %d\n", htonl(command.p2));
			break;
		}
		break;
	case(6):
		printf("STOP\n");
		break;
	case(7):
		printf("REPEAT\n%u\n", command.p2);
		break;
	}
}

void get_gps(FILE *fp)
{
	double lon, lat;
	float alt, speed, bear, acc;
	char direction;
	
	struct gps_h gps;
	
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
	gps.bear = htonl(gps.bear);
	memcpy(&bear, &gps.bear, 4);

	// convert binary to float
	gps.speed = htonl(gps.speed);
	memcpy(&speed, &gps.speed, 4);

	// convert binary to float
	gps.acc = htonl(gps.acc);
	memcpy(&acc, &gps.acc, 4);
	
	// check for N or S
	(lat < 0) ? (direction = 'S') : (direction = 'N');
	
	// flip sign on lat if negative
	(lat < 0) ? lat = -lat : lat;
	
	int deg, min;
	double sec;
	
	// convert latitude to Degrees/minutes/seconds
	deg = (int)lat;
	min = (int)((lat - deg) * 60);
	sec = (lat - deg - (float)min / 60) * 3600;
	
	printf("Latitude: %d°%d'%0.3f\" %c\n", deg, min, sec, direction);
	
	// check for W or E
	(lon < 0) ? (direction = 'W') : (direction = 'E');
	
	// flip sign on lon if negative
	(lon < 0) ? lon = -lon : lon;
	
	// convert longitude to Degrees/minutes/seconds
	deg = (int)lon;
	min = (int)((lon - deg) * 60);
	sec = (lon - deg - (double)min / 60) * 3600;
	
	printf("Longitude: %d°%d'%0.3f\" %c\n", deg, min, sec, direction);
	
	printf("Altitude: %0.1fm\n", (alt * 1.8288));
	printf("Bearing: %0.9f deg.\n", bear);
	printf("Speed: %0.0fkm/h\n", (speed * 3.6));
	printf("Accuracy: %0.0fm\n", acc);
}

int check_24bit_sign(int num)
{
	// checks for negative 24 bit ints and returns them as 32 bit ints
	
	int bit = 0;
	
	// get sign bit
	bit = (num >> 23) & 1U;
	
	// check for negative, return negative 32 bit
	if(bit == 1){
		num += 0xff000000;
		return num;
	}
	
	// sign bit not set
	return num;
}
