#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define PACKET_LEN (sizeof( struct packet))

int UDPsocket_recv(struct sockaddr_in *addr, int port)
{
	int sd;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("cannot create socket\n");
	}

	memset((char *) addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(sd, (struct sockaddr*) addr, sizeof(*addr)) == -1)
	{
		perror("bind failed\n");
	}

	return sd;
}

int UDPsocket_send(struct sockaddr_in *addr, char *ip,int port)
{
	int sd;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("cannot create socket\n");
	}

	memset((char*) addr, 0, sizeof(*addr));
	addr->sin_family=AF_INET;
	addr->sin_port=htons(port);
	if(inet_aton(ip, &addr->sin_addr) == 0)
	{
		perror("inet_aton() failed\n");
	}
	return sd;
}
struct packet{
	int serial_n;
	int len;
	char data[1000];
};

int main(int argc, char *argv[])
{
	struct sockaddr_in receiver_addr, sender_addr, myaddr, tmp_addr;
	int s1, s2, i, slen = sizeof(myaddr);
	
	s1 = UDPsocket_recv(&myaddr, atoi(argv[3]));
	s2 = UDPsocket_send(&receiver_addr, argv[4], atoi(argv[5]));

	memset((char*) &sender_addr, 0, sizeof(sender_addr));
	sender_addr.sin_family = AF_INET;
	sender_addr.sin_port = htons(atoi(argv[2]));
	if(inet_aton(argv[1], &sender_addr.sin_addr) == 0)
	{
		perror("inet_aton() failed\n");
	}

	fprintf(stderr, "receiver IP is %s\n", inet_ntoa(receiver_addr.sin_addr));
	fprintf(stderr, "receiver port is %d\n", ntohs(receiver_addr.sin_port));
	
	struct packet pack;
	memset(&pack, 0, sizeof(pack));
	char buf[2000];
	memset(buf, 0, 2000);
	memset((char*) &tmp_addr, 0, sizeof(tmp_addr));
	int ack = 0;
	srand(time(NULL));
	int r = atoi(argv[6]);
	int count = 0, loss = 0, n=0;

	while(1)
	{
		n = recvfrom(s1, buf, 1500, 0, (struct sockaddr*) &tmp_addr, &slen);
		if(n < 0)perror("\n");
		if(n == sizeof(pack))
		{
			if((rand() % 100 + 1) > r)
			{
				count++;
				memcpy(&pack, buf, PACKET_LEN);
				memset(buf, 0, 2000);
				sendto(s2, &pack, PACKET_LEN, 0, (struct sockaddr*) &receiver_addr, slen);
				if(pack.serial_n > 0)
				{
					fprintf(stderr, "get    data   #%d\n", pack.serial_n);
					fprintf(stderr, "fwd    data   #%d,      loss rate = %f\n", pack.serial_n, loss / count);
				}
				else
				{
					fprintf(stderr, "get    fin\n");
					fprintf(stderr, "fwd    fin\n");
				}
				memset(&pack, 0, sizeof(pack));
			}
			else
			{
				count++;
				loss++;
				memcpy(&pack, buf, PACKET_LEN);
				memset(buf, 0, 2000);
				if(pack.serial_n > 0)
				{
					fprintf(stderr, "drop   data   #%d,      loss rate = %f\n", pack.serial_n, loss / count);
				}
				else
				{
					fprintf(stderr, "drop    fin\n");
				}
				memset(&pack, 0, sizeof(pack));
			}
			
		}
		else if(n == sizeof(int))
		{
			memcpy(&ack, buf, sizeof(int));
			memset(buf, 0, 2000);
			sendto(s2, &ack, sizeof(int), 0, (struct sockaddr*) &sender_addr, slen);
			if(ack > 0)
			{
				fprintf(stderr, "get    ack   　#%d\n", ack);
				fprintf(stderr, "fwd    ack   　#%d\n", ack);
			}
			else
			{
				fprintf(stderr, "get    finack\n");
				fprintf(stderr, "fwd    finack\n");
				exit(0);
			}
			ack = 0;
		}
		
	}


	
}