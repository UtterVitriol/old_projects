/* functions used to calculate the ipv4 & udp header checksum
 * for zerg packets
 */

#include <string.h>
#include <stdint.h>
#include "structs.h"
#include "chksm.h"


uint16_t sum_command(struct command_h command)
{
	/* calculates 1's complement 16 bit sum of the command header
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	
	uint32_t sum = 0, temp = 0;
	uint16_t carry = 0;
	
	sum += command.cmnd;
	
	sum += command.p1;
	
	
	sum += command.p2 >> 16;
	
	temp = command.p2 << 16;
	temp = temp >> 16;
	sum += temp;
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	return sum;
}

uint16_t sum_message(struct message_h message)
{
	/* calculates 1's complement 16 bit sum of the message header
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	
	uint32_t sum = 0;
	uint16_t carry = 0;
	
	int msg_len = 0, msg_flag = 0;
	
	msg_len = strlen(message.msg);
	
	
	/* check if the message length is even
	 * -1 from the length if odd
	 */
	if((msg_len % 2) != 0){
		msg_len = msg_len - 1;
		msg_flag = 1;
	}
	
	// get 16 bit sum of the message
	for(int i = 0; i < msg_len; i++){
		sum += (message.msg[i] << 8) + message.msg[i + 1];
		i++; 
	}
	
	/* artificially pad odd message lengths with zeros
	 * to calculate the right 16 bit sum
	 */
	if(msg_flag){
		sum += message.msg[msg_len] << 8;
	}
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	return sum;
}

uint16_t sum_gps(struct gps_h gps)
{
	/* calculates 1's complement 16 bit sum of the gps header
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	
	uint32_t sum = 0, temp = 0;
	uint64_t templ = 0;
	uint16_t carry = 0;
	
	
	sum += gps.lon >> 48;
	
	templ = gps.lon << 16;
	templ = templ >> 48;
	sum += templ;
	
	templ = gps.lon >> 16;
	templ = templ << 48;
	templ = templ >> 48;
	sum += templ;
	
	templ = gps.lon << 48;
	templ = templ >> 48;
	sum += templ;
	
	
	sum += gps.lat >> 48;
	
	templ = gps.lat << 16;
	templ = templ >> 48;
	sum += templ;
	
	templ = gps.lat >> 16;
	templ = templ << 48;
	templ = templ >> 48;
	sum += templ;
	
	templ = gps.lat << 48;
	templ = templ >> 48;
	sum += templ;
	
	
	sum += gps.alt >> 16;
	
	temp = gps. alt << 16;
	temp = temp >> 16;
	sum += temp;
	
	
	sum += gps.bear >> 16;
	
	temp = gps.bear << 16;
	temp = temp >> 16;
	sum += temp;
	
	
	sum += gps.speed >> 16;
	
	temp = gps.speed << 16;
	temp = temp >> 16;
	sum += temp;
	
	
	sum += gps.acc >> 16;
	
	temp = gps.acc << 16;
	temp = temp >> 16;
	sum += temp;
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	return sum;
}

uint16_t sum_status(struct status_h status, char *name)
{
	/* calculates 1's complement 16 bit sum of the status header
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	
	uint32_t sum = 0, temp = 0;
	int name_len = 0, name_flag = 0;
	uint16_t carry = 0;
	
	sum += status.hp >> 16;
	
	temp = status.hp << 16;
	temp = temp >> 8;
	temp += status.armor;
	sum += temp;

	
	sum += status.mhp >> 16;
	
	temp = status.mhp << 16;
	temp = temp >> 8;
	temp += status.type;

	
	sum += status.speed >> 16;
	
	temp = status.speed << 16;
	temp = temp >> 16;
	sum += temp;

	
	name_len = strlen(name);
	
	/* check if the name length is even
	 * -1 from the length if odd
	 */
	if((name_len % 2) != 0){
		name_len = name_len - 1;
		name_flag = 1;
	}
	
	// get 16 bit sum of the name
	for(int i = 0; i < name_len; i++){
		sum += (name[i] << 8) + name[i + 1];
		i++; 
	}
	
	/* artificially pad odd name lengths with zeros
	 * to calculate the right 16 bit sum
	 */
	if(name_flag){
		sum += name[name_len] << 8;
	}
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	return sum;
}

uint16_t udp_ipv6_checksum(struct udp_h udp, struct ip6_h ip6, struct zerg_h zerg, uint16_t data)
{
	/* calculate udp header checksum -
	 * 
	 *     calculate the 16-bit 1's complement of the
	 * 1's complement 16 bit sum of a "pseudo" ipv6 header
	 * the udp header and the data
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	 
	uint32_t sum = 0, temp = 0;
	uint16_t carry = 0, checksum = 0;
	
	sum += ((ip6.src[0] << 8) + ip6.src[1]);
	sum += ((ip6.src[2] << 8) + ip6.src[3]);
	sum += ((ip6.src[4] << 8) + ip6.src[5]);
	sum += ((ip6.src[6] << 8) + ip6.src[7]);
	sum += ((ip6.src[8] << 8) + ip6.src[9]);
	sum += ((ip6.src[10] << 8) + ip6.src[11]);
	sum += ((ip6.src[12] << 8) + ip6.src[13]);
	sum += ((ip6.src[14] << 8) + ip6.src[15]);
	
	sum += ((ip6.dst[0] << 8) + ip6.dst[1]);
	sum += ((ip6.dst[2] << 8) + ip6.dst[3]);
	sum += ((ip6.dst[4] << 8) + ip6.dst[5]);
	sum += ((ip6.dst[6] << 8) + ip6.dst[7]);
	sum += ((ip6.dst[8] << 8) + ip6.dst[9]);
	sum += ((ip6.dst[10] << 8) + ip6.dst[11]);
	sum += ((ip6.dst[12] << 8) + ip6.dst[13]);
	sum += ((ip6.dst[14] << 8) + ip6.dst[15]);
	
	
	sum += udp.len + ip6.prot + udp.sport + udp.dport + udp.len;
	
	sum += ((zerg.ver << 12) + (zerg.type << 8)) + (zerg.len >> 16);
	 
	 
	temp = zerg.len << 16;
	temp = temp >> 16;
	sum += temp;

	
	sum += zerg.seq >> 16;
	
	temp = zerg.seq << 16;
	temp = temp >> 16;
	sum += temp;

	
	sum += data;
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	// get one's complement of the sum
	checksum = (~sum);
	
	// checks for zero, get 1's complement if zero
	if(checksum == 0){
		checksum = ~checksum;
	}
	
	return checksum;
}

uint16_t udp_ipv4_checksum(struct udp_h udp, struct ip4_h ip4, struct zerg_h zerg, uint16_t data)
{
	/* calculate udp header checksum -
	 * 
	 *     calculate the 16-bit 1's complement of the
	 * 1's complement 16 bit sum of a "pseudo" ipv6 header,
	 * the udp header and the data
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	 
	uint32_t sum = 0, temp = 0;
	uint16_t carry = 0, checksum = 0;
	
	sum += ((ip4.src[0] << 8) + ip4.src[1]);
	sum += ((ip4.src[2] << 8) + ip4.src[3]);
	
	sum += ((ip4.dst[0] << 8) + ip4.dst[1]);
	sum += ((ip4.dst[2] << 8) + ip4.dst[3]);
	
	sum += ip4.prot + udp.len + udp.sport + udp.dport + udp.len;
	
	sum += ((zerg.ver << 12) + (zerg.type << 8)) + (zerg.len >> 16);
	 
	 
	temp = zerg.len << 16;
	temp = temp >> 16;
	sum += temp;

	
	sum += zerg.seq >> 16;
	
	temp = zerg.seq << 16;
	temp = temp >> 16;
	sum += temp;

	
	sum += data;
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	// get one's complement of the sum
	checksum = (~sum);
	
	// checks for zero, get 1's complement if zero
	if(checksum == 0){
		checksum = ~checksum;
	}
	
	return checksum;
}

uint16_t ipv4_checksum(struct ip4_h ip4)
{
	/* calculate ipv4 header checksum - 
	 * 
	 *     calculate the 16-bit 1's complement of the
	 * 1's complement 16 bit sum of the ipv4 header
	 * 
	 * Uses bit-shifting to single out 16-bits at a time
	 */
	
	uint32_t sum = 0;
	uint16_t carry, checksum;
	
	
	sum = (ip4.ver << 12) + (ip4.ihl << 8) + ip4.len + ip4.prot;
	
	sum += ((ip4.src[0] << 8) + ip4.src[1]);
	sum += ((ip4.src[2] << 8) + ip4.src[3]);
	
	sum += ((ip4.dst[0] << 8) + ip4.dst[1]);
	sum += ((ip4.dst[2] << 8) + ip4.dst[3]);
	
	// carry check for 1's complement additon
	if(sum > 0xffff){
		while(sum > 0xffff){
			carry = sum >> 16;
			
			sum = sum - (carry << 16);
			
			sum += carry;
		}
	}
	
	// get one's complement of the sum
	checksum = (~sum);
	
	return checksum;
}
