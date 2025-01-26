#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

struct icmp_packet{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t identifier;
	uint16_t sequence;
};

struct ip_packet {
	uint8_t ihl:4;
	uint8_t version:4;
	uint8_t tos;
	uint16_t total_length;
	uint16_t identification;
	uint16_t fragment_offset;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	uint32_t source_ip;
	uint32_t destination_ip;
};

unsigned short checksum(void *b, int len){
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;
	for(sum = 0; len>1; len -= 2){
		sum += *buf++;
	}
	if(len == 1){
		sum += *(unsigned char *)buf;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

uint32_t get_local_ip(){
    //TODO implement function
    return(inet_addr("192.168.50.215"));
}


int main(int argc, char *argv[]){
    //check if proper arguments supplied
    if(argc != 2){
        printf("\nProper Usage: %s <ip address>\n", argv[0]);
        return(0);
    }

    //set up ping destination ip

   	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &dest_addr.sin_addr);
	uint32_t source_ip = get_local_ip();
	uint32_t destination_ip = inet_addr(argv[1]);

    //create raw socket
	int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (raw_socket < 0){
		printf("Raw socket unable to be created");
		return 0;
		} else {
			printf("Raw socket created on file descriptor %d\n", raw_socket);
	}
	//construct ICMP packet
	struct icmp_packet icmp_header;
	memset(&icmp_header, 0, sizeof(icmp_header));
	icmp_header.type = 8;
	icmp_header.code = 0;
	icmp_header.identifier = 25565;
	icmp_header.sequence = 1;
	icmp_header.checksum = checksum(&icmp_header, sizeof(icmp_header));

	//construct IP packet
	struct ip_packet ip_header;
	memset(&ip_header, 0, sizeof(ip_header));
	ip_header.version = 4;
	ip_header.ihl = 5;
	ip_header.tos = 0;
	ip_header.total_length = htons(28);
	ip_header.identification = htons(rand() % 65536);
	ip_header.fragment_offset = htons(0);
	ip_header.ttl = 64;
	ip_header.protocol = 1;
	ip_header.checksum = 0;
	ip_header.source_ip = source_ip;
	ip_header.destination_ip = destination_ip;
	ip_header.checksum = checksum(&ip_header, sizeof(ip_header));



	//construct complete packet
	char packet[1024];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, &ip_header, sizeof(struct ip_packet));
	memcpy(packet+sizeof(struct ip_packet), &icmp_header, sizeof(struct icmp_packet));

	if(sendto(raw_socket, packet, sizeof(struct ip_packet) + sizeof(struct icmp_packet), 0,
	(struct sockaddr *) &dest_addr, sizeof(dest_addr)) < 0) {
		printf("packet failed to send\n");
		return 0;
	}
	printf("packet sent\n");

	struct timeval timeout;


	shutdown(raw_socket, SHUT_RDWR);
	close(raw_socket);


	return 0;
}
