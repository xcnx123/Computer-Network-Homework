#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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

struct pack_buf{
	int flag;
	int serial_n;
	char data[1000];
};

int main(int argc, char *argv[])
{
	struct sockaddr_in agent_addr, myaddr, tmp_addr;
	int s1, s2, i, slen = sizeof(myaddr);
	
	s1 = UDPsocket_recv(&myaddr, atoi(argv[1]));
	s2 = UDPsocket_send(&agent_addr, argv[3], atoi(argv[4]));
	fprintf(stderr, "server IP is %s\n", inet_ntoa(myaddr.sin_addr));
	fprintf(stderr, "server port is %d\n", ntohs(myaddr.sin_port));

	struct packet pack;
	memset(&pack, 0, sizeof(pack));
	//struct pack_buf pbuf[33];
	struct pack_buf *pbuf;
	pbuf = (struct pack_buf*) malloc( sizeof(struct pack_buf) * 33);
	memset(pbuf, 0, sizeof(struct pack_buf) * 33);
	int serial_n = 0;
	int count = 0;
	int fd;
	int index=0;

	fd = open(argv[2], O_WRONLY | O_CREAT, 0777);
	fprintf(stderr,"err = %d, fd = %d\n",errno,fd);
	perror("");
	lseek(fd, 0, SEEK_SET);

	while(1)
	{
		if(recvfrom(s1, &pack, sizeof(pack), 0, (struct sockaddr*) &tmp_addr, &slen) > 0)
		{
			if(pack.serial_n == 0)
			{
				for(i=1;pbuf[i+1].flag == 1;i++)
				{
					write(fd, pbuf[i].data, 1000);
				}
				write(fd, pbuf[i].data, pack.len);
				fprintf(stderr,"recv   fin\n");
				sendto(s1, &pack.serial_n, sizeof(int), 0, (struct sockaddr*) &agent_addr, slen);
				fprintf(stderr,"send   finack\n");
				fprintf(stderr,"flush\n");
				close(fd);
				exit(0);
			}
			if((pack.serial_n - serial_n) > 32)
			{
				fprintf(stderr,"drop   data   #%d\n", pack.serial_n);
				for(i=1;i<=32;i++)
				{
					if(pbuf[i].flag == 1)
					{
						count++;
					}
				}
				if(count == 32)
				{
					for(i=1;i<=32;i++)
					{
						write(fd, pbuf[i].data, 1000);
					}
					memset(pbuf, 0, sizeof(struct pack_buf) * 33);
					fprintf(stderr,"flush\n");
					serial_n += 32;
				}
				count = 0;
			}
			if(pbuf[pack.serial_n - serial_n].flag == 0)
			{
				fprintf(stderr,"recv   data   #%d\n", pack.serial_n);
				index = (pack.serial_n - serial_n);
				pbuf[index].flag = 1;
				pbuf[index].serial_n = pack.serial_n;
				memcpy(pbuf[index].data, pack.data, 1000);
				sendto(s1, &pack.serial_n, sizeof(int), 0, (struct sockaddr*) &agent_addr, slen);
				fprintf(stderr,"send   ack    #%d\n", pack.serial_n);
			}
			else 
			{
				sendto(s1, &pack.serial_n, sizeof(int), 0, (struct sockaddr*) &agent_addr, slen);
				fprintf(stderr,"ignr   data   #%d\n", pack.serial_n);
			}
		}			
	}
}