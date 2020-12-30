/* Encodes a .pcap file containing zerg packets.
    
    Takes file containing one or more zerg payload descriptions.
    
    (See below for examples)
    
    Program will try to inform user of input errors.
    
    Accepts utf-8 characters in status and message payloads.
    
    ------------------------------
    status payload eg.
    ------------------------------
    
    Version: 1
    Sequence: 9
    From: 1337
    To: 7890
    Name: ZaLgO
    HP: 17
    MaxHP: 35
    Type: Zergling
    Armor: 0
    MaxSpeed: 12.9531m/s

    ------------------------------
    gps payload eg.
    ------------------------------
    
    Version: 1
    Sequence: 81
    From: 1337
    To: 6600
    Latitude: 87°55'5.833" N
    Longitude: 32°14'4.284" W
    Altitude: 43.5m
    Bearing: 47.833332062 deg.
    Speed: 100km/h
    Accuracy: 5m
    
    ------------------------------
    message payload eg.
    ------------------------------
    
    Version: 1
    Sequence: 81
    From: 7890
    To: 1234
    Message: Hello World!
    
    ------------------------------
    command GET_STATUS payload eg.
    ------------------------------
    
    Version: 1
    Sequence: 73387868
    From: 871
    To: 6600
    GET_STATUS
    
    ------------------------------
    command GOTO eg.
    ------------------------------
    
    Version: 1
    Sequence: 1401146863
    From: 2525
    To: 6600
    GOTO
    Orient: 88.888 deg.
    Distance: 87m

    ------------------------------
    command SET_GROUP eg1.
    ------------------------------
    
    Version: 1
    Sequence: 3852717534
    From: 870
    To: 6600
    SET_GROUP
    AddTo: -17
    
    ------------------------------
    command SET_GROUP eg2.
    ------------------------------
    
    Version: 1
    Sequence: 3781864571
    From: 20210
    To: 6600
    SET_GROUP
    RmFrom: 17
    
    ------------------------------
    command REPEAT eg.
    ------------------------------
    
    Version: 1
    Sequence: 1483537264
    From: 871
    To: 6600
    REPEAT
    3982459904
    
    ------------------------------
    ------------------------------
    
    command GET_GPS, RETURN and STOP follow the same format as GET_STATUS
 */
 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>


#include "structs.h"
#include "chksm.h"


void create_pcap(char *, char *);

void init_headers(struct global_h *, struct pcap_h *pcap, struct ether_h *,\
		  struct ip4_h *, struct ip6_h *, struct udp_h *);

int create_zerg(FILE *, struct zerg_h *, char *);

int create_message(FILE *, struct message_h *);

char *create_status(FILE *, struct status_h *);

int get_status_type(char *);

int create_command(FILE *, struct command_h *, char *);

int create_gps(FILE *, struct gps_h *);

int handle_args(int , char **);

void print_help(void);


int ip_version = 0;
int big_endian = 0;


int main(int argc, char **argv)
{
	// calls handle_args and create_pcap
	
	setlocale(LC_ALL, "");
	
	int arg = 0;
	
	if(argc < 3){
		print_help();
	}
	
	arg = handle_args(argc, argv);
	
	// check if .pcap is in outfile name
	if(strstr(argv[arg + 1], ".pcap") == NULL){
		print_help();
	}
	
	create_pcap(argv[arg], argv[arg + 1]);
}

void print_help(void)
{
	// prints help message and exits the program
	puts("Usage: ./encode [-6bh] [filename] [filename.pcap]");
	puts("-6 makes ipv6 packet");
	puts("-b make the pcap big endian");
	puts("-h prints this help message");
	puts("makes ipv4 by default");
	puts("See man page for example file formats");
	exit(1);
}
int handle_args(int argc, char **argv)
{
	// handles flags, exits if bad args

	int opt = 0, count = 0;
	
	while((opt = getopt(argc, argv, "6bh")) != -1){
		
		// exits if more than expected number of flags is supplied
		if(count > 1){
			print_help();
		}
		
		// sets flags for encoding
		switch(opt){
		case('6'):
			ip_version = 1;
			count += 1;
			break;
		case('b'):
			big_endian = 1;
			count += 1;
			break;
		case('h'):
			print_help();
		default:
			print_help();
		}
		
	}
	
	// exits if no flags and more than three args
	if(count == 0 && argc > 3){
		print_help();
	}
	
	// exits if more than two file was supplied
	if(count > 0 && argc > count + 3){
		print_help();
	}
	
	return optind;
}

void create_pcap(char *file_in, char *file_out)
{
	/* main driving function
	 * calls functions to read description file and write pcap
	 */

	int ip_len = 0;
	
	// check ip_version flag and sets the ip_len to appropriate size
	if(ip_version == 0){
		ip_len = IP4H;
	}else{
		ip_len = IP6H;
	}
	
	struct global_h global;
	struct pcap_h pcap;
	struct ether_h ether;
	struct ip4_h ip4;
	struct ip6_h ip6;
	struct udp_h udp;
	struct zerg_h zerg;
	
	init_headers(&global, &pcap, &ether, &ip4, &ip6, &udp);
	

	// open descriptor file
	FILE *in;
	in = fopen(file_in, "r");

	if(!in){
		printf("File: '%s', does not exits....", file_in);
		exit(1);
	}
	
	// open out.pcap file
	FILE *out;
	
	out = fopen(file_out, "wb");
	
	if(out == NULL){
		puts("Error creating pcap file...");
		exit(1);
	}
	
	// write global header
	fwrite(&global, 1, GLOBALH, out);
	
	int temp_int, count = 0;
	char temp_str[10] = {'\0'};
	
	// loop through descriptions
	while(1){
		count ++;
		
		// read Version:
		if(fscanf(in, "%10s", temp_str) != 1){
			
			// file is empty
			if(ftell(in) == 0){
				puts("File is empty...");
			}
			break;
		}
		
		// create_zerg
		if(create_zerg(in, &zerg, temp_str) == 1){
			printf("\nDescription: %d\n", count);
			break;
		}
		
		// read next field
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("File missing arguments after \"To:\" line...");
			printf("\nDescription: %d\n", count);
			break;
		}
		
		/* check if field read matches the first field for any  of the
		 *    four payloads
		 * 
		 * calls appropriate function for the corresponding payload
		 * 
		 * if the field doesn't match any playload field,
		 *    the create_command function will catch it.
		 */
		if(strcmp(temp_str, "Message:") == 0){
			struct message_h message;
			
			// create_message
			if(create_message(in, &message) == 1){
				fclose(in);
				fclose(out);
				exit(1);
			}
			
			// set zerg payload type and zer length
			zerg.type = 0;
			zerg.len = (ZERGH + strlen(message.msg));
			
			int pcap_len = (ETHERH + ip_len + UDPH + zerg.len);
			
			// set pcap length
			if(big_endian){
				pcap.len = ntohl(pcap_len);
				pcap.u_len = ntohl(pcap_len);
			}else{
				pcap.len = pcap_len;
				pcap.u_len = pcap_len;
			}
			
			// set ip header lengths
			ip4.len = htons(IP4H + UDPH + zerg.len);
			ip6.len = htons(UDPH + zerg.len);
			
			// calculate ipv4 checksum
			if(ip_version == 0){
				ip4.chksm = htons(ipv4_checksum(ip4));
			}
			
			// set udp header length
			udp.len = htons(UDPH + zerg.len);
			
			// write pcap header and ethernet frame
			fwrite(&pcap, 1, PCAPH, out);
			fwrite(&ether, 1, ETHERH, out);
			
			// write appropriate ip header
			if(ip_version == 0){
				fwrite(&ip4, 1, IP4H, out);
			}else{
				fwrite(&ip6, 1, IP6H, out);	
			}
			
			//swap bytes
			zerg.len = htonl(zerg.len << 8);
			
			// calculate udp checksum
			if(ip_version == 0){
				udp.chksm = htons(udp_ipv4_checksum(udp, ip4,\
						  zerg, sum_message(message)));
				
			}else{
				udp.chksm = htons(udp_ipv6_checksum(udp, ip6,\
						  zerg, sum_message(message)));
			}
			
			// write udp header
			fwrite(&udp, 1, UDPH, out);
			
			// write zerg header
			fwrite(&zerg, 1, ZERGH, out);
			
			// write and free message
			fprintf(out, "%s", message.msg);
			free(message.msg);
			
		}else if(strcmp(temp_str, "Name:") == 0){
			struct status_h status;
			
			char *name = NULL;
			
			// create_status
			name = create_status(in, &status);
			
			// checks for error in create_status
			if(name == NULL){
				printf("Description: %d\n", count);
				fclose(in);
				fclose(out);
				exit(1);
			}
			
			// set zerg payload type and zer length
			zerg.type = 1;
			zerg.len = (ZERGH + STATH + strlen(name) - 1);
			
			int pcap_len = (ETHERH + ip_len + UDPH + zerg.len);
			
			// set pcap length
			if(big_endian){
				pcap.len = ntohl(pcap_len);
				pcap.u_len = ntohl(pcap_len);
			}else{
				pcap.len = pcap_len;
				pcap.u_len = pcap_len;
			}
			
			// set ip header lengths
			ip4.len = htons(IP4H + UDPH + zerg.len);
			ip6.len = htons(UDPH + zerg.len);
			
			// calculate ipv4 checksum
			if(ip_version == 0){
				ip4.chksm = htons(ipv4_checksum(ip4));
			}
			
			// set udp header length
			udp.len = htons(UDPH + zerg.len);
			
			// write pcap header and ethernet frame
			fwrite(&pcap, 1, PCAPH, out);
			fwrite(&ether, 1, ETHERH, out);
			
			// write appropriate ip header
			if(ip_version == 0){
				fwrite(&ip4, 1, IP4H, out);
			}else{
				fwrite(&ip6, 1, IP6H, out);	
			}
			
			// remove newline from name
			name[strlen(name) - 1] = '\0';
			
			// swap bytes
			zerg.len = htonl(zerg.len << 8);
			
			// calculate udp checksum
			if(ip_version == 0){
				udp.chksm = htons(udp_ipv4_checksum(udp, ip4,\
						  zerg, sum_status(status,\
								   name)));
			}else{
				udp.chksm = htons(udp_ipv6_checksum(udp, ip6,\
						  zerg, sum_status(status,\
								   name)));
			}
			
			// write udp header
			fwrite(&udp, 1, UDPH, out);
			
			// write zerg header
			fwrite(&zerg, 1, ZERGH, out);
			
			// write status header
			fwrite(&status, 1, STATH, out);
			
			// write and free name
			fprintf(out, "%s", name);
			free(name);
			
		}else if(strcmp(temp_str, "Latitude:") == 0){
			struct gps_h gps;
			
			// create_gps
			if(create_gps(in, &gps) == 1){
				printf("Description: %d\n", count);
				fclose(in);
				fclose(out);
				exit(1);
			}
			
			// set zerg payload type and zer length
			zerg.type = 3;
			zerg.len = (ZERGH + GPSH);
			
			int pcap_len = (ETHERH + ip_len + UDPH + zerg.len);
			
			// set pcap length
			if(big_endian){
				pcap.len = ntohl(pcap_len);
				pcap.u_len = ntohl(pcap_len);
			}else{
				pcap.len = pcap_len;
				pcap.u_len = pcap_len;
			}
			
			// set ip header lengths
			ip4.len = htons(IP4H + UDPH + zerg.len);
			ip6.len = htons(UDPH + zerg.len);
			
			// calculate ipv4 checksum
			if(ip_version == 0){
				ip4.chksm = htons(ipv4_checksum(ip4));
			}
			
			// set udp header length
			udp.len = htons(UDPH + zerg.len);
			
			// write pcap header and ethernet frame
			fwrite(&pcap, 1, PCAPH, out);
			fwrite(&ether, 1, ETHERH, out);
			
			// write appropriate ip header
			if(ip_version == 0){
				fwrite(&ip4, 1, IP4H, out);
			}else{
				fwrite(&ip6, 1, IP6H, out);	
			}
			
			// swap bytes
			zerg.len = htonl(zerg.len << 8);
			
			// calculate udp checksum
			if(ip_version == 0){
				udp.chksm = htons(udp_ipv4_checksum(udp, ip4,\
						  zerg, sum_gps(gps)));
			}else{
				udp.chksm = htons(udp_ipv6_checksum(udp, ip6,\
						  zerg, sum_gps(gps)));
			}
			
			// write udp header
			fwrite(&udp, 1, UDPH, out);
			
			// write zerg header
			fwrite(&zerg, 1, ZERGH, out);
			
			// write gps header
			fwrite(&gps, 1, GPSH, out);
			
		}else{
			struct command_h command;
			
			// call create_command and exits if error
			if(create_command(in, &command, temp_str) == 1){
				printf("Description: %d\n", count);
				fclose(in);
				fclose(out);
				exit(1);
			}
			
			// set zerg payload type and zer length
			zerg.type = 2;
			zerg.len = (ZERGH + CMNDH);
			
			int pcap_len = (ETHERH + ip_len + UDPH + zerg.len);
			
			// set pcap length
			if(big_endian){
				pcap.len = ntohl(pcap_len);
				pcap.u_len = ntohl(pcap_len);
			}else{
				pcap.len = pcap_len;
				pcap.u_len = pcap_len;
			}
			
			// set ip header lengths
			ip4.len = htons(IP4H + UDPH + zerg.len);
			ip6.len = htons(UDPH + zerg.len);
			
			// calculate ipv4 checksum
			if(ip_version == 0){
				ip4.chksm = htons(ipv4_checksum(ip4));
			}
			
			// set udp header length
			udp.len = htons(UDPH + zerg.len);
			
			// write pcap header and ethernet frame
			fwrite(&pcap, 1, PCAPH, out);
			fwrite(&ether, 1, ETHERH, out);
			
			// write appropriate ip header
			if(ip_version == 0){
				fwrite(&ip4, 1, IP4H, out);
			}else{
				fwrite(&ip6, 1, IP6H, out);	
			}
			
			// swap bytes
			zerg.len = htonl(zerg.len << 8);
			
			// calculate udp checksum
			if(ip_version == 0){
				udp.chksm = htons(udp_ipv4_checksum(udp, ip4,\
						  zerg, sum_command(command)));
			}else{
				udp.chksm = htons(udp_ipv6_checksum(udp, ip6,\
						  zerg, sum_command(command)));
			}
			
			// write udp header
			fwrite(&udp, 1, UDPH, out);
			
			// write zerg header
			fwrite(&zerg, 1, ZERGH, out);
			
			//write command header
			fwrite(&command, 1, CMNDH, out);
		}
	}
	
	// close files
	fclose(in);
	fclose(out);
}

void init_headers(struct global_h *global, struct pcap_h *pcap,\
		  struct ether_h *ether, struct ip4_h *ip4,\
		  struct ip6_h *ip6, struct udp_h *udp)
{
	/* handles initializing headers */
	
	// initializing global header
	if(big_endian){
		global->magic = 0xd4c3b2a1;
		global->maj_ver = 0x200;
		global->min_ver = 0x400;
		global->ll_type = 0x1000000;
	}else{
		global->magic = 0xa1b2c3d4;
		global->maj_ver = 0x2;
		global->min_ver = 0x4;
		global->ll_type = 0x1;
	}
	
	global->gmt = 0x0;
	global->acc_delt = 0x0;
	global->len = 0x0;//0x10000;
	
	
	// initializing pcap header
	pcap->time = 0x0;
	pcap->m_time = 0x0;
	pcap->len = 0x0;
	pcap->u_len = 0x0;
	
	// initializing ethernet frame
	ether->dst[0] = 'K';
	ether->dst[1] = 'O';
	ether->dst[2] = 'W';
	ether->dst[3] = 'X';
	ether->dst[4] = 'X';
	ether->dst[5] = 'X';

	ether->src[0] = 'C';
	ether->src[1] = 'I';
	ether->src[2] = 'L';
	ether->src[3] = 'L';
	ether->src[4] = 'E';
	ether->src[5] = 'R';
	
	// checks ip_version and sets the ethernet type to the appropriate value
	if(ip_version == 0){
		ether->type = htons(0x800);
	}else{
		ether->type = htons(0x86dd);
	}
	
	
	// initalize appropriate ip header depending on ip_version flag
	if(ip_version == 0){
		ip4->ihl = 0x5;
		ip4->ver = 0x4;
		ip4->dscp = 0x0;
		ip4->ecn = 0x0;
		ip4->len = 0x0;
		ip4->id = 0x0;
		ip4->flags = 0x0;
		ip4->ffset = 0x0;
		ip4->ttl = 0x0;
		ip4->prot = 0x11;
		ip4->chksm = 0x0;

		ip4->src[0] = 'D';
		ip4->src[1] = 'E';
		ip4->src[2] = 'A';
		ip4->src[3] = 'D';

		ip4->dst[0] = 'B';
		ip4->dst[1] = 'E';
		ip4->dst[2] = 'E';
		ip4->dst[3] = 'F';

	}else{
		ip6->ver = 0x6;
		ip6->t_class1 = 0x0;
		ip6->t_class2 = 0x0;
		ip6->f_label1 = 0x0;
		ip6->f_label2 = 0x0;
		ip6->len = 0x0;
		ip6->prot = 17;
		ip6->hop_l = 0x0;

		ip6->src[0] = 0xde;
		ip6->src[1] = 0xad;
		ip6->src[2] = 0xbe;
		ip6->src[3] = 0xef;
		ip6->src[4] = 0xca;
		ip6->src[5] = 0xfe;
		ip6->src[6] = 0x0;
		ip6->src[7] = 0x0;
		ip6->src[8] = 0xbe;
		ip6->src[9] = 0xee;
		ip6->src[10] = 0xee;
		ip6->src[11] = 0xee;
		ip6->src[12] = 0xee;
		ip6->src[13] = 0xee;
		ip6->src[14] = 0xee;
		ip6->src[15] = 0xef;

		ip6->dst[0] = 0xde;
		ip6->dst[1] = 0xad;
		ip6->dst[2] = 0xbe;
		ip6->dst[3] = 0xef;
		ip6->dst[4] = 0xca;
		ip6->dst[5] = 0xfe;
		ip6->dst[6] = 0x0;
		ip6->dst[7] = 0x0;
		ip6->dst[8] = 0xbe;
		ip6->dst[9] = 0xee;
		ip6->dst[10] = 0xee;
		ip6->dst[11] = 0xee;
		ip6->dst[12] = 0xee;
		ip6->dst[13] = 0xee;
		ip6->dst[14] = 0xee;
		ip6->dst[15] = 0xef;
	}

	// initializing UDP header
	udp->sport = htons(0xbeef);
	udp->dport = htons(0xEA7);
	udp->len = 0x0;
	udp->chksm = htons(0x0);
}

int create_zerg(FILE *in, struct zerg_h *zerg, char *version)
{
	/* create zerg header */
	
	char temp_str[11] = {'\0'};
	int temp_int = 0;
	
	// check if actually read Version:
	if(strcmp(version, "Version:") == 0){
		
		// read version number
		if(fscanf(in, "%d", &temp_int) != 1){
			puts("No Version: <num>");
			return 1;
		}
		
		// check if version number is 1
		if(temp_int != 1){
			puts("Expected '1' for Version: <num>");
		}
	}else{
		puts("No Version:");
		return 1;
	}
	
	// set zerg version
	zerg->ver = temp_int;
	
	// read Sequence:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No Sequence:");
		return 1;
	}
	
	// check if actually read Sequence:
	if(strcmp(temp_str, "Sequence:") == 0){
		
		// read sequence number
		if(fscanf(in, "%d", &temp_int) != 1){
			puts("No Sequence: <num>");
			return 1;
		}
	}else{
		puts("No Sequence:");
		return 1;
	}
	
	// set sequence number
	zerg->seq = htonl(temp_int);
	
	// read From:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No From:");
		return 1;
	}
	
	// check if actually read From:
	if(strcmp(temp_str, "From:") == 0){
		
		// read from number
		if(fscanf(in, "%d", &temp_int) != 1){
			puts("No From: <num>");
			return 1;
		}
	}else{
		puts("No From:");
		return 1;
	}
	
	// set from number
	zerg->src = htons(temp_int);
	
	// read To:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("To: not there");
		return 1;
	}
	
	// check if actually read To:
	if(strcmp(temp_str, "To:") == 0){
		
		// read to number
		if(fscanf(in, "%d", &temp_int) != 1){
			puts("No To: <num>");
			return 1;
		}
	}else{
		puts("No To:");
		return 1;
	}
	
	// set to number
	zerg->dst = htons(temp_int);
	
	return 0;
}

int get_status_type(char *type_name)
{
	/* takes a string, returns a number if it matches any string below
	 * returns 666 if the string doesn't match any
	 */
	if(strcmp(type_name, "Overmind") == 0){
		return 0;
	}else if(strcmp(type_name, "Larva") == 0){
		return 1;
	}else if(strcmp(type_name, "Cerebrate") == 0){
		return 2;
	}else if(strcmp(type_name, "Overlord") == 0){
		return 3;
	}else if(strcmp(type_name, "Queen") == 0){
		return 4;
	}else if(strcmp(type_name, "Drone") == 0){
		return 5;
	}else if(strcmp(type_name, "Zergling") == 0){
		return 6;
	}else if(strcmp(type_name, "Lurker") == 0){
		return 7;
	}else if(strcmp(type_name, "Broodling") == 0){
		return 8;
	}else if(strcmp(type_name, "Hydralisk") == 0){
		return 9;
	}else if(strcmp(type_name, "Guardian") == 0){
		return 10;
	}else if(strcmp(type_name, "Scourge") == 0){
		return 11;
	}else if(strcmp(type_name, "Ultralisk") == 0){
		return 12;
	}else if(strcmp(type_name, "Mutalisk") == 0){
		return 13;
	}else if(strcmp(type_name, "Defiler") == 0){
		return 14;
	}else if(strcmp(type_name, "Devourer") == 0){
		return 15;
	}else{
		puts("Bad Type:");
		return 666;
	}
}

int create_command(FILE *in, struct command_h *command, char *cmnd)
{
	// creates command payload header

	int temp_sint = 0;
	char temp_str[10] = {'\0'};
	unsigned int temp_int = 0;
	unsigned short temp_short = 0;
	float temp_float = 0;
	
	/* check if cmnd matches any command types
	 * returns 1 if it doesn't
	 */
	if(strcmp(cmnd, "GET_STATUS") == 0){
		
		// set command type to a GET_STATUS
		command->cmnd = 0;
		
		// initialize unused fields to 0
		command->p1 = 0;
		command->p2 = 0;
		
	}else if(strcmp(cmnd, "GOTO") == 0){
		
		// set command type to GOTO
		command->cmnd = htons(0x1);
		
		// read Orient:
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("Expected Orient: <num> deg.");
			return 1;
		}
		
		// check if actually read Orient:
		if(strcmp(temp_str, "Orient:") == 0){
			
			// read orient number
			if(fscanf(in, "%f", &temp_float) != 1){
				puts("Missing Orient: <num>");
				return 1;
			}
			
			// convert float to integer
			memcpy(&temp_int, &temp_float, 4);
			
			// swap bytes
			temp_int = htonl(temp_int);
			
			// set command param 2
			command->p2 = temp_int;
			
			// read deg.
			if(fscanf(in, "%10s", temp_str) != 1){
				puts("Expected 'deg.' after Orient <num>");
				return 1;
			}
			
			// check if actually read deg.
			if(strcmp(temp_str, "deg.") != 0){
				puts("Expected 'deg.' after Orient <num>");
				return 1;
			}
		}else{
			puts("Missing Orient: <num>");
			return 1;
		}
		
		// read Distance:
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("Expected Distance: <num>m");
			return 1;
		}
		
		// check if actually read Distance:
		if(strcmp(temp_str, "Distance:") == 0){
			
			// read distance number
			if(fscanf(in, "%hud", &temp_short) != 1){
				puts("Missing Distance: <num>m");
				return 1;
			}
			
			// read m
			if(fscanf(in, "%1s", temp_str) != 1){
				puts("expected m after Distance: <num>");
				return 1;
			}
			
			// check if actually read m
			if(temp_str[0] != 'm'){
				puts("expected m after Distance: <num>");
				return 1;
			}
			
			// swap bytes
			temp_short = htons(temp_short);
			
			// set command param 1
			command->p1 = temp_short;
		}else{
			puts("Missing Distance: <num>m");
			return 1;
		}
		
	}else if(strcmp(cmnd, "GET_GPS") == 0){
		
		// set command type to GET_GPS
		command->cmnd = htons(0x2);
		
		// initialize unused fields to 0
		command->p1 = 0;
		command->p2 = 0;
		
	}else if(strcmp(cmnd, "RESERVED") == 0){
		
		// returns 1 for unused command type
		puts("RESERVED is an invalid command");
		return 1;
		
	}else if(strcmp(cmnd, "RETURN") == 0){
		
		// set command type to RETURN
		command->cmnd = htons(0x4);
		
		// initialize unused fields to 0
		command->p1 = 0;
		command->p2 = 0;
		
	}else if(strcmp(cmnd, "SET_GROUP") == 0){
		
		// set command type to SET_GROUP
		command->cmnd = htons(0x5);
		
		// read Addto: or RmFrom:
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("Missing AddTo: <num> or RmFrom: <num>");
			return 1;
		}
		
		// check if actually read AddTo: or RmFrom:
		if(strcmp(temp_str, "AddTo:") == 0){
			
			// set command param 1 to 1
			command->p1 = htons(0x1);
		}else if(strcmp(temp_str, "RmFrom:") == 0){
			
			// set command param 1 to 0
			command->p1 = 0;
		}else{
			puts("Missing AddTo: <num> or RmFrom: <num>");
			return 1;
		}
		
		// read AddTo: or RmFrom: number
		if(fscanf(in, "%d", &temp_sint) != 1){
			puts("Missing AddTo: <num> or RmFrom: <num>");
			return 1;
		}
		
		// set command param 2
		command->p2 = htonl(temp_sint);
		
	}else if(strcmp(cmnd, "STOP") == 0){
		
		// set command type to STOP
		command->cmnd = htons(0x6);
		
		// initialize unused fields to 0
		command->p1 = 0;
		command->p2 = 0;
		
	}else if(strcmp(cmnd, "REPEAT") == 0){
		
		// set command type to REPEAT
		command->cmnd = htons(0x7);
		
		// read repeat number
		if(fscanf(in, "%d", &temp_sint) != 1){
			puts("Expected sequence number after RETURN");
			return 1;
		}
		
		// swap bytes
		temp_int = htonl(temp_sint);
		
		// set command param 2
		command->p2 = temp_sint;
		
		// initialize unused field to 0
		command->p1 = 0;
		
	}else{
		/* next field after the zerg descriptors didn't match
		 * any payload field
		 */
		puts("Missing required field...see manpage");
		return 1;
	}
	return 0;
}

int create_gps(FILE *in, struct gps_h *gps)
{
	double temp_double, temp_double2;
	float temp_float;
	unsigned int temp_int, temp_int2;
	unsigned long temp_long;
	char temp_str[11] = {'\0'}, temp_char;
	wchar_t temp_wchar;
	
	// read latitude degrees
	if(fscanf(in, "%d", &temp_int) != 1){
		puts("Bad Latitude:");
		return 1;
	}else{
		// read °
		if(fscanf(in, "%lc", &temp_wchar) != 1){
			puts("Expected ° after Latitude degrees");
			return 1;
		}
		
		// check if actually read °
		if(temp_wchar != L'°'){
			puts("Exptected ° after Latitude degrees");
			return 1;
		}
		
		// read latitude minutes
		if(fscanf(in, "%d",&temp_int2) != 1){
			puts("Bad Latitude:");
			return 1;
		}
		
		// read '
		if(fscanf(in, "%c", &temp_char) != 1){
			puts("Expected ' after Latitude minutes");
			return 1;
		}
		
		// check if actually read '
		if(temp_char != '\''){
			puts("Expected ' after Latitude minutes");
			return 1;
		}
		
		// read seconds
		if(fscanf(in, "%lf", &temp_double2) != 1){
			puts("Bad Latitude:");
			return 1;
		}
		
		// read "
		if(fscanf(in, "%c", &temp_char) != 1){
			puts("Expected \" after Latitude seconds");
			return 1;
		}
		
		// check if actually read "
		if(temp_char != '"'){
			puts("Expected \" after Latitude seconds");
			return 1;
		}
		
		// convert DMS to decimal degrees
		temp_double = (temp_int + (double)temp_int2/60 +\
			       temp_double2/3600);
		
		// read N or S
		if(fscanf(in, "%1s", &temp_char) != 1){
			puts("Expected 'N' or 'S' after Latitude: <num> deg.");
		}
		
		// check if actually read N or S
		if(temp_char == 'S'){
			
			// flip sign on double
			temp_double = -temp_double;
			
		}else if(temp_char == 'N'){
			;
		}else{
			puts("Expected 'N' or 'S' after Latitude: <num> deg.");
			return 1;
		}
		
		// convert double to long
		memcpy(&temp_long, &temp_double, 8);
		
		// swap bytes
		temp_long = __builtin_bswap64(temp_long);
		
		// set gps latitude
		gps->lat = temp_long;
	}
	
	// read Longitude:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("Missing Longitude <num> deg. <direction>");
		return 1;
	}
	
	// check if actually read Longitude:
	if(strcmp(temp_str, "Longitude:") == 0){
		
		// read longitude degrees
		if(fscanf(in, "%d", &temp_int) != 1){
			puts("Bad Longitude:");
			return 1;
		}
		
		// read °
		if(fscanf(in, "%lc", &temp_wchar) != 1){
			puts("Expected ° after Longitude degrees");
			return 1;
		}
		
		// check if actually read °
		if(temp_wchar != L'°'){
			puts("Exptected ° after Longitude degrees");
			return 1;
		}
		
		// read minutes
		if(fscanf(in, "%d",&temp_int2) != 1){
			puts("Bad Longitude:");
			return 1;
		}
		
		// read '
		if(fscanf(in, "%c", &temp_char) != 1){
			puts("Expected ' after Longitude minutes");
			return 1;
		}
		
		// check if actually read '
		if(temp_char != '\''){
			puts("Expected \' after Longitude minutes");
			return 1;
		}
		
		// read seconds
		if(fscanf(in, "%lf", &temp_double2) != 1){
			puts("Bad Longitude:");
			return 1;
		}
		
		// read "
		if(fscanf(in, "%c", &temp_char) != 1){
			puts("Expected \" after Longitude seconds");
			return 1;
		}
		
		// check if actually read "
		if(temp_char != '"'){
			puts("Expected \" after Longitude seconds");
			return 1;
		}
		
		// convert DMS to decimal degrees
		temp_double = (temp_int + (double)temp_int2/60 +\
			       temp_double2/3600);
		
		// read W or E
		if(fscanf(in, "%1s", &temp_char) != 1){
			puts("Expected 'W' or 'E' after Longitude: <num> deg.");
			return 1;
		}
		
		// check if actually read W or E
		if(temp_char == 'W'){
			
			// flip sign on double
			temp_double = -temp_double;
			
		}else if(temp_char == 'E'){
			;
		}else{
			puts("Expected 'W' or 'E' after Longitude: <num> deg.");
			return 1;
		}
		
		// convert double to long
		memcpy(&temp_long, &temp_double, 8);
		
		// swap bytes
		temp_long = __builtin_bswap64(temp_long);
		
		// set gps longitude
		gps->lon = temp_long;
	}else{
		puts("No Longitude:");
		return 1;
	}
	
	// read Altitude:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("Expected Altitude: <num>m");
		return 1;
	}
	
	// check if actually read Altitude:
	if(strcmp(temp_str, "Altitude:") == 0){
		
		// read altitude number
		if(fscanf(in, "%f", &temp_float) != 1){
			puts("No Altitude value...");
			return 1;
		}
		
		// convert altitude to fathoms
		temp_float = temp_float / 1.8288;
		
		// convert float to int
		memcpy(&temp_int, &temp_float, 4);
		
		// swap bytes
		temp_int = htonl(temp_int);
		
		// set gps altitude
		gps->alt = temp_int;
		
		// read m
		if(fscanf(in, "%c", &temp_char) != 1){
			puts("Expected 'm' after Altitude: <num>");
			return 1;
		}
		
		//check if actually read m
		if(temp_char != 'm'){
			puts("Expected 'm' after Altitude: <num>");
			return 1;
		}
	}else{
		puts("No Altitude:");
		return 1;
	}
	
	// read Bearing:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("Expected Bearing: <num> deg.");
		return 1;
	}
	
	// check if actually read Bearing:
	if(strcmp(temp_str, "Bearing:") == 0){
		
		// read bearing number
		if(fscanf(in, "%f", &temp_float) != 1){
			puts("No Bearing..");
			return 1;
		}
		
		// convert float to int
		memcpy(&temp_int, &temp_float, 4);
		
		// swap bytes
		temp_int = htonl(temp_int);
		
		// set gps bearing
		gps->bear = temp_int;
		
		// read deg.
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("Expected 'deg.' after Bearing: <num>");
			return 1;
		}
		
		// check if actually read deg.
		if(strcmp(temp_str, "deg.") != 0){
			puts("Expected 'deg.' after Bearing: <num>");
			return 1;
		}
	}else{
		puts("No Bearing:");
		return 1;
	}
	
	// read Speed:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("Expected Speed: <num>km/h");
		return 1;
	}
	
	// check if actually read Speed:
	if(strcmp(temp_str, "Speed:") == 0){
		
		// read speed number
		if(fscanf(in, "%f", &temp_float) != 1){
			puts("No Speed...");
			return 1;
		}
		
		// convert speed to m/s
		temp_float = temp_float / 3.6;
		
		// convert float to int
		memcpy(&temp_int, &temp_float, 4);
		
		//swap bytes
		temp_int = htonl(temp_int);
		
		// set gps speed
		gps->speed = temp_int;
		
		// read km/h
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("Expected 'km/h' after Speed: <num>");
			return 1;
		}
		
		// check if actually read km/h
		if(strcmp(temp_str, "km/h") != 0){
			puts("Expected 'km/h' after Speed: <num>");
			return 1;
		}
	}else{
		puts("No Speed:");
		return 1;
	}
	
	// read Accuracy:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("Expected Accuracy: <num>m");
		return 1;
	}
	
	// check if actually read Accuracy:
	if(strcmp(temp_str, "Accuracy:") == 0){
		
		// read accuracy number
		if(fscanf(in, "%f", &temp_float) != 1){
			puts("No Accuracy...");
			return 1;
		}
		
		// convert float to int
		memcpy(&temp_int, &temp_float, 4);
		
		// swap bytes
		temp_int = htonl(temp_int);
		
		// set gps accuracy
		gps->acc = temp_int;
		
		// read m
		if(fscanf(in, "%c", &temp_char) != 1){
			puts("Expected 'm' after Accuracy: <num>");
			return 1;
		}
		
		// check if actually read m
		if(temp_char != 'm'){
			puts("Expected 'm' after Accuracy: <num>");
			return 1;
		}
	}else{
		puts("No Accuracy:");
		return 1;
	}
	return 0;
}

int create_message(FILE *in, struct message_h *message)
{
	// reads message
	
	char temp_char;
	
	message->msg = NULL;

	size_t len = 0;
	
	// skip space
	temp_char = fgetc(in);
	
	
	
	// read message
	if(getline(&message->msg, &len, in) == -1){
		puts("No Message: <message>");
		return 1;
	}
	
	// check message
	if(temp_char == '\n'){
		puts("No Message: <message>");
		free(message->msg);
		return 1;
	}
	
	// remove newline
	message->msg[strlen(message->msg) - 1] = '\0';
	return 0;
}

char *create_status(FILE *in, struct status_h *status)
{
	// make sure all the values are the right size
	
	unsigned int temp_int = 0, type = 0;
	int temp_int2 = 0;
	float temp_float = 0;
	char temp_str[10] = {'\0'}, *name = NULL, temp_char;
	size_t len = 0;
	
	// skip space
	temp_char = fgetc(in);
	
	// read name
	if(getline(&name, &len, in) == -1){
		puts("No Name: <name>");
		return NULL;
	}
	
	// check name
	if(temp_char == '\n'){
		puts("No Name: <name>");
		free(name);
		return NULL;
	}
	
	// read HP:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No HP: <num>");
		free(name);
		return NULL;
	}
	
	// check if actually read HP:
	if(strcmp(temp_str, "HP:") == 0){
		
		// read hp number
		if(fscanf(in, "%d", &temp_int2) != 1){
			puts("No HP: <num>");
			free(name);
			return NULL;
		}
		
		// check if hp fits in bits
		if(temp_int2 > 8388607 || temp_int2 < -8388608){
			puts("Bad HP: <num> (-8388608 - 8388607)");
			free(name);
			return NULL;
		}
		
		
		// set hp
		status->hp = htonl(temp_int2 << 8);
		
	}else{
		puts("No HP: <num>");
		free(name);
		return NULL;
	}
	
	// read MaxHP:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No MaxHP: <num>");
		free(name);
		return NULL;
	}
	
	// check if actually read MaxHP:
	if(strcmp(temp_str, "MaxHP:") == 0){
		
		// read maxhp 
		if(fscanf(in, "%u", &temp_int) != 1){
			puts("No MaxHP: <num>");
			free(name);
			return NULL;
		}
		
		// check if maxhp fits in allotted bits
		if(temp_int > 16777215){
			puts("Bad MaxHP: <num> (0 - 16777215)");
			free(name);
			return NULL;
		}
		
		// set status maxhp
		status->mhp = htonl(temp_int << 8);
	}else{
		puts("No MaxHP: <num>");
		free(name);
		return NULL;
	}
	
	// read Type:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No Type: <type>");
		free(name);
		return NULL;
	}
	
	// check if actually read Type:
	if(strcmp(temp_str, "Type:") == 0){
		
		// read type
		if(fscanf(in, "%10s", temp_str) != 1){
			puts("No Type: <type>");
			free(name);
			return NULL;
		}else{
			
			// get type number from type string
			type = get_status_type(temp_str);
			
			// bad type
			if(type == 666){
				free(name);
				return NULL;
			}
			
			// set status type
			status->type = type;
		}
	}else{
		puts("No Type: <type>");
		free(name);
		return NULL;
	}
	
	// read Armor:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No Armor: <num>");
		free(name);
		return NULL;
	}
	
	// check if actually read Armor:
	if(strcmp(temp_str, "Armor:") == 0){
		
		// read armor number
		if(fscanf(in, "%u", &temp_int) != 1){
			puts("No Armor: <num>");
			free(name);
			return NULL;
		}
		
		// check if armor number fits in allotted bits
		if(temp_int > 255){
			puts("Bad Armor: <num> (0 - 255)");
			free(name);
			return NULL;
		}
		
		// set status armor
		status->armor = htons(temp_int << 8);
	}else{
		puts("No Armor: <num>");
		free(name);
		return NULL;
	}
	
	// read MaxSpeed:
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("No MaxSpeed: <num>m/s");
		free(name);
		return NULL;
	}
	
	// check if actually read MaxSpeed:
	if(strcmp(temp_str, "MaxSpeed:") == 0){
		
		// read maxspeed number
		if(fscanf(in, "%f", &temp_float) != 1){
			puts("No MaxSpeed: <num>m/s");
			free(name);
			return NULL;
		}
		
		// convert float to int
		memcpy(&temp_int, &temp_float, 4);
		
		// swap bytes
		temp_int = htonl(temp_int);
		
		// set status speed
		status->speed = temp_int;
	}else{
		puts("No MaxSpeed: <num>m/s");
		free(name);
		return NULL;
	}
	
	// read m/s
	if(fscanf(in, "%10s", temp_str) != 1){
		puts("Expected m/s after MaxSpeed: <num>");
		free(name);
		return NULL;
	}
	
	// check if actually read m/s
	if(strcmp(temp_str, "m/s") != 0){
		puts("Expected m/s after MaxSpeed: <num>");
		free(name);
		return NULL;
	}

	return name;
}
