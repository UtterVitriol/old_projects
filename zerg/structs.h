/* structs used for decode.c, encode.c and chksm.c to read/write zerg
 * .pcap files
 */

#pragma once

// header lengths
#define GLOBALH 24
#define PCAPH 16
#define ETHERH 14
#define IP6H 40
#define IP4H 20
#define UDPH 8
#define ZERGH 12
#define STATH 12
#define GPSH 32
#define CMNDH 8

// global header
struct global_h{
	uint32_t magic;
	uint32_t maj_ver : 16;
	uint32_t min_ver : 16;
	uint32_t gmt;
	uint32_t acc_delt;
	uint32_t len;
	uint32_t ll_type;
};

// pcap packet header
struct pcap_h{
	uint32_t time;
	uint32_t m_time;
	uint32_t len;
	uint32_t u_len;
};

// ethernet frame
struct ether_h{
	unsigned char dst[6];
	unsigned char src[6];
	uint16_t type;
};

// ipv4 header
struct ip4_h{
	uint8_t ihl : 4;
	uint8_t ver : 4;
	uint8_t dscp : 6;
	uint8_t ecn : 2;
	uint16_t len;
	uint16_t id;
	uint16_t flags : 3;
	uint16_t ffset : 13;
	uint8_t ttl;
	uint8_t prot;
	uint16_t chksm;
	uint8_t src[4];
	uint8_t dst[4];
};

// ipv6 header
struct ip6_h{
	uint8_t t_class1 : 4;
	uint8_t ver : 4;
	uint8_t t_class2 : 4;
	uint8_t f_label1 : 4;
	uint16_t f_label2;
	uint16_t len;
	uint8_t prot;
	uint8_t hop_l;
	unsigned char src[16];
	unsigned char dst[16];
};

// udp header
struct udp_h{
	uint16_t sport;
	uint16_t dport;
	uint16_t len;
	uint16_t chksm; 
};

// zerg header
struct zerg_h{
	uint32_t type : 4;
	uint32_t ver : 4;
	uint32_t len : 24;
	uint16_t src;
	uint16_t dst;
	uint32_t seq;	
};

// status header
struct status_h{
	uint32_t hp : 24;
	uint32_t armor : 8;
	uint32_t mhp : 24;
	uint32_t type : 8;
	uint32_t speed;
};

// message payload
struct message_h{
	char *msg;
};

// command header
struct command_h{
	uint16_t cmnd;
	uint16_t p1;
	int32_t p2;
};

// gps header
struct gps_h{
	uint64_t lon;
	uint64_t lat;
	uint32_t alt;
	uint32_t bear;
	uint32_t speed;
	uint32_t acc;
};
