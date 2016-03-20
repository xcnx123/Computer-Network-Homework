#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <strings.h>
#define player 2

int main(int argc, char *argv[])
{
	int sockfd, pfd[player], i;
	socklen_t length;
	struct sockaddr_in ser_addr, client_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[1]));
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) < 0)
	{
		printf("Bind error\n");
	}

	listen(sockfd, 1);

	length = sizeof(client_addr);

	for(i = 0; i < player; i++)
	{
		printf("Wait for connect...\n");
		pfd[i] = accept(sockfd, (struct sockaddr*)&client_addr, &length);
		if(pfd[i] < 0) printf("Accept Failed\n");
		else printf("Accept new client\n");
	}

	srand(time(NULL));
	//new round roop
	while(1)
	{
		printf("----------------------New round---------------------\n");
		printf("Game start\n");
		// generate random number
		int tmp, num[10], pos;
		for(i=0;i<10;i++)
		{
			num[i] = i;
		}
		for(i=0;i<10;i++)
		{
			pos = rand() % 10;
			tmp = num[i];
			num[i] = num[pos];
			num[pos] = tmp;
		}
		int guess[4];
		int result[2];
		int j, p, rnd[player],rcount;
		bzero(rnd, sizeof(int)*player);
		p = 0;
		rcount = 0;
		int over[player];
		bzero(over, sizeof(int)*player);
		int win[player], winner = -1, lessrnd = 0;
		bzero(win, sizeof(int)*player);

		printf("The number is %d %d %d %d\n", num[0], num[1], num[2], num[3]);

		//recv guess number
		while(1)
		{
			p++;
			if(p == player) p = 0;
			bzero(guess, sizeof(int)*4);
			bzero(result, sizeof(int)*2);
			if(over[p] == 0)
			{
				if(recv(pfd[p], guess, sizeof(int)*4, MSG_DONTWAIT) > 0)
				{
					for(i=0;i<4;i++)
					{
						for(j=0;j<4;j++)
						{
							if(guess[i] == num[j])
							{
								if(i == j) result[0]++;
								else result[1]++;
							}
						}
					}

					send(pfd[p], result, sizeof(int)*2, 0);
					rnd[p]++;

					if(result[0] == 4)
					{
						rcount++;
						break;
					}
					else
					{
						if(rnd[p] == 10)
						{
							over[p] = 1;
							rnd[p]++;
							rcount++;
						}
					}
					if(rcount == player)
					{
						break;
					}
				}
			}
		}
		for(i = 0; i < player; i++)
		{
			if(rnd[i] < lessrnd)
			{
				lessrnd = rnd[i];
			}
		}
		for (i = 0; i < player; ++i)
		{
			if(rnd[i] == lessrnd)
			{
				win[i] = 1;
			}
		}
		for (i = 0; i < player; ++i)
		{
			write(pfd[i], &win[i], sizeof(int));
		}
		printf("The winner is:");
		for (i = 0; i < player; ++i)
		{
			if(win[i])
			{
				printf(" player%d", i+1);
			}
		}
		printf("\n");
	}
}