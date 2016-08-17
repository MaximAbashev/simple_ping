# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <ctype.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <linux/if_ether.h>
# include <netpacket/packet.h>
# include <net/ethernet.h>
# include <net/if.h>
# include <linux/if_ether.h>
# include "../hdr/func.h"

# define ICMP_ECHOREPLY 0
# define ICMP_REQUEST 8
# define PCKT_LEN 92

int main ()
{
	int raw_sock, len, byte_size_buf;
	const int on = 1;
	struct ipv4_header *ip_hdr;
	struct ipv4_header *ip_reply_hdr;
	struct icmp_header *icmp_hdr;
	struct hostent host_name;
	struct sockaddr_in host_addr;
	struct sockaddr_in inet_ntoa_addr;
	char *packet;
	char *buf;
/*
 * Create and configure socket
 */
	raw_sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (raw_sock < 0)
	{
		perror ("Socket create error!");
		exit (1);
	}
	if (setsockopt (raw_sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)))
	{
		perror ("Setsockopt error!");
		exit (1);
	}
/*
 * Allocate memory for buffer and headers
 */ 
	ip_hdr = malloc (sizeof (struct ipv4_header));
	ip_reply_hdr = malloc (sizeof (struct ipv4_header));
	icmp_hdr = malloc (sizeof (struct icmp_header));
	packet = malloc (PCKT_LEN);
	buf = malloc (PCKT_LEN);

	ip_hdr = (struct ipv4_header *) packet;
	icmp_hdr = 
		(struct icmp_header *) (packet + sizeof (struct ipv4_header));
/*
 * Fabricate ip header
 */
	ip_hdr -> ihl = 0x5;
	ip_hdr -> version = 0x4;
	ip_hdr -> tos = 0x0;
	ip_hdr -> tot_len = 
		sizeof (struct ipv4_header) + sizeof (struct icmp_header);
	ip_hdr -> id = htons(12830);
	ip_hdr -> frag_off = 0x0;
	ip_hdr -> ttl = 255;
	ip_hdr -> protocol = IPPROTO_ICMP;
	ip_hdr -> check = 0x0;
	ip_hdr -> saddr = inet_addr ("192.168.2.13");
	ip_hdr -> daddr = inet_addr ("8.8.8.8");
/*
 * Fabricate icmp header
 */
	icmp_hdr -> type = ICMP_REQUEST;
	icmp_hdr -> code = 0;
	icmp_hdr -> un.echo.id = 0;
	icmp_hdr -> un.echo.sequence = 0;
	icmp_hdr -> checksum = 0;
	icmp_hdr -> checksum = 
		checksum ((u_short *) icmp_hdr, sizeof (struct icmp_header));
	ip_hdr -> check = checksum ((u_short *) ip_hdr, sizeof (*ip_hdr));
/*
 * Fabricate target ip-address and sending message
 */
	host_addr.sin_family = AF_INET;
	host_addr.sin_addr.s_addr = inet_addr ("8.8.8.8");
	byte_size_buf = 
		sendto (raw_sock, packet, PCKT_LEN, 0,
		(struct sockaddr *)&host_addr, sizeof (struct sockaddr_in));
	if (byte_size_buf < 0)
	{
		perror ("Sendto error!");
		exit (1);
	}
	memcpy (&inet_ntoa_addr.sin_addr, &ip_hdr -> saddr,
		sizeof (ip_hdr -> saddr));
	printf ("Send %d byte packet to %s\n", byte_size_buf,
		inet_ntoa (inet_ntoa_addr.sin_addr));
/*
 * Receiving and analysis reply message  
 */
	memset (&inet_ntoa_addr, 0, sizeof (struct sockaddr_in));
	len = sizeof (host_addr);
	byte_size_buf = 0;
	byte_size_buf = recvfrom(raw_sock, buf, PCKT_LEN, 0,
				(struct sockaddr *)&host_addr, &len);
	if (byte_size_buf < 0)
	{
		perror ("Recvfrom error!");
		exit (1);
	}
	else
	{
		memcpy (&inet_ntoa_addr.sin_addr, &ip_hdr -> daddr,
			sizeof (ip_hdr -> daddr));
		printf ("Received %d byte reply from %s\n", byte_size_buf,
			inet_ntoa (inet_ntoa_addr.sin_addr));
		ip_reply_hdr = (struct ipv4_header *) buf;
		printf ("ID: %d\n", ntohs (ip_reply_hdr -> id));
		printf ("TTL: %d\n", ip_reply_hdr -> ttl);
	}
	close (raw_sock);
	return 1;
}
