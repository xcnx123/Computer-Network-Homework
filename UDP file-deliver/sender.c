#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
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
	struct sockaddr_in agent_addr, myaddr, tmp_addr;
	int s1, s2, i, slen = sizeof(agent_addr);
	
	s1 = UDPsocket_recv(&myaddr, atoi(argv[1]));
	s2 = UDPsocket_send(&agent_addr, argv[3], atoi(argv[4]));
	fprintf(stderr, "agent IP is %s\n", inet_ntoa(agent_addr.sin_addr));
	fprintf(stderr, "agent port is %d\n", ntohs(agent_addr.sin_port));

	struct packet pack;
	int serial_n, ack, fd, total_pack, n;
	int threshold, winsize;
	struct stat stat_tmp;
	struct timeval ack_t[34], now_t;
	memset(ack_t, 0, sizeof(struct timeval) * 34);
	memset(&now_t, 0, sizeof(struct timeval));
	int check_flag[34];
	memset(check_flag, 0, sizeof(int) * 34);
	long timeout, timeuse;
	int count = 0, flag = 0;

	fprintf(stderr,"read file...\n");
	fd = open(argv[2], O_RDONLY);

	lstat(argv[2], &stat_tmp);
	total_pack = stat_tmp.st_size/1000 + 1;

	threshold = 16;
	winsize = 1;
	timeout = 500000;

	memset(&pack, 0, sizeof(pack));
	serial_n = 0;

	lseek(fd, 0, SEEK_SET);
	while(1)
	{
		if(serial_n < total_pack)
		{
			lseek(fd, (serial_n) * 1000, SEEK_SET);
		}
		else
		{
			while(1)
			{
				pack.serial_n = 0;
				pack.len = stat_tmp.st_size % 1000;
				sendto(s1, &pack, PACKET_LEN, 0, (struct sockaddr*) &agent_addr, slen);
				gettimeofday(&ack_t[0], 0);
				fprintf(stderr,"send   fin\n");
				while(1)
				{
					gettimeofday(&now_t, 0);
					if(recvfrom(s1, &ack, sizeof(ack), MSG_DONTWAIT, (struct sockaddr*) &tmp_addr, &slen) > 0)
					{
						fprintf(stderr,"recv   finack\n");
						exit(0);
					}
					else if(timeout < (now_t.tv_usec - ack_t[0].tv_usec))
					{
						break;
					}
				}
			}
			
		}
		for(i=1;i <= winsize;i++)
		{
			if(read(fd, pack.data, 1000) > 0)
			{
				pack.serial_n = serial_n + i;
				sendto(s1, &pack, PACKET_LEN, 0, (struct sockaddr*) &agent_addr, slen);
				gettimeofday(&ack_t[i], 0);
				fprintf(stderr,"send   data   #%d,    winSize = %d\n", serial_n + i, winsize);
				memset(&pack, 0, sizeof(pack));
			}
			else
			{
				winsize = i-1;
				break;
			}
		}

		while(1)
		{
			memset(check_flag, 0, sizeof(int) * 34);
			memset(ack_t, 0, sizeof(struct timeval) * 34);
			gettimeofday(&now_t, 0);
			if(recvfrom(s1, &ack, sizeof(ack), MSG_DONTWAIT, (struct sockaddr*) &tmp_addr, &slen) > 0)
			{
				timeuse = now_t.tv_usec - ack_t[ack - serial_n].tv_usec;
				if(timeuse < timeout)
				{
					check_flag[ack - serial_n] = 1;
					fprintf(stderr,"recv   ack    #%d\n", ack);
				}
			}
			for(i = 1;i <= winsize;i++)
			{
				if(check_flag[i] == 0)
				{	
					if(timeout < (now_t.tv_usec - ack_t[i].tv_usec))
					{
						serial_n += (i-1);
						flag = 1;
						fprintf(stderr,"time   out,          threshold = %d\n", threshold);
						break;
					}
				}
				else
				{
					count++;
				}
			}
			if(count == winsize)
			{
				memset(check_flag, 0, sizeof(int) * 34);
				serial_n += winsize;
				count = 0;
				if(winsize < threshold)
				{
					winsize = winsize * 2;
				}
				else
				{
					winsize++;
				}
				break;
			}
			else if(flag == 1)
			{
				memset(check_flag, 0, sizeof(int) * 34);
				if(winsize < 4)
				{
					threshold = 1;
				}
				else
				{
					threshold = winsize/2;
				}
				winsize = 1;
				flag = 0;
				break;
			}
		}
		
		
	}
			
}