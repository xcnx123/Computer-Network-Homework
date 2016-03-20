#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <strings.h>

int main(int argc, char *argv[])
{
	int serverfd;
	socklen_t length;
	struct sockaddr_in ser_addr;

	serverfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &ser_addr.sin_addr);

	if(connect(serverfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) < 0)
	{
		printf("Connect Failed\n");
	}
	else
	{
		printf("Connect Success\n");
	}

	int button = 0;
	char buf[100];
	bzero(buf, 100);
	while(1)
	{
		printf("1.Play  2.Option  3.About us  4.Story\n");
		scanf("%d", &button);
		if(button == 1)
		{
			break;
		}
		else if(button == 2)
		{
			printf("Back to menu?\n");
			scanf("%s", buf);
		}
		else if(button == 3)
		{
			printf("Back to menu?\n");
			scanf("%s", buf);
		}
		else if(button == 4)
		{
			printf("Back to menu?\n");
			scanf("%s", buf);
		}
		else
		{
			printf("Back to menu?\n");
			scanf("%s", buf);
		}
	}

	int guess[4], result[2];
	int rnd, legality, win, money = 1000;
	while(1)
	{
		win = 0;
		rnd = 0;
		printf("--------------------New round--------------------\n");
		printf("You have %d dollars\n", money);
		while(1)
		{
			legality = 1;
			bzero(guess, sizeof(int)*4);
			bzero(result, sizeof(int)*2);
			while(1)
			{
				printf("Please guess number:\n");
				scanf("%d %d %d %d", &guess[0], &guess[1], &guess[2], &guess[3]);
				for (int i = 0; i < 4; ++i)
				{
					if(guess[i] > 9 || guess[i] < 0)
					{
						legality = 0;
					}
					for (int j = i+1; j < 4; ++j)
					{
						if(guess[i] == guess[j])
						{
							legality = 0;
						}
					}
				}
				if(legality == 1)
				{
					break;
				}
				else
				{
					printf("illegal input\n");
				}
			}
			
			send(serverfd, guess, sizeof(int)*4, 0);
			rnd++;
			recv(serverfd, result, sizeof(int)*2, 0);
			if(result[0] == 4)
			{
				printf("You get it!!\n");
				break;
			}
			else
			{
				if(rnd == 10)
				{
					printf("%d A %d B\n", result[0], result[1]);
					printf("Already 10th round\n");
					break;
				}
				else
				{
					printf("%d A %d B\n", result[0], result[1]);
				}
			}
		}
		read(serverfd, &win, sizeof(int));
		if(win)
		{
			printf("You win\n");
			printf("You get one million dollars!!!\n");
			money += 1000000;
		}
		else
		{
			printf("You lose\n");
		}
	}
}