#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <strings.h>
#define num 2

int main(int argc, char *argv[])
{
	int sockfd, i;
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

	listen(sockfd, num);
	length = sizeof(client_addr);
	int player[num];

	for (i = 0; i < num; ++i)
	{
		
		printf("Wait for connect...\n");
		player[i] = accept(sockfd, (struct sockaddr*)&client_addr, &length);
		if(player[i] < 0) printf("Accept Failed\n");
		else printf("Accept new client\n");
	}
	
	while(1)
	{
		printf("--------------------division----------------------\n");
		printf("Game start\n");
		printf("Wait for deal\n");
		char buf[10];
		bzero(buf, 10);
		for(i = 0; i < num; ++i)
		{
			read(player[i], buf, 1);
		}
		printf("The players had betted\n");

		int card[52], tmp, pos;
		srand(time(NULL));
		for(i=0;i<52;i++)
		{
			card[i] = i / 4 + 1;
		}
		for(i=0;i<52;i++)
		{
			pos = rand() % 52;
			tmp = card[i];
			card[i] = card[pos];
			card[pos] = tmp;
		}

		int handheld[num][5], dealer[5], heldcount[num], sum[num], dealer_sum = 0, c = 0, offset = 0;
		int split[num][5], splitcount[num], spsum[num];
		bzero(handheld, sizeof(int)*num*5);
		bzero(heldcount, sizeof(int)*num);
		bzero(dealer, sizeof(int)*5);
		bzero(sum, sizeof(int)*num);
		bzero(split, sizeof(int)*num*5);
		bzero(splitcount, sizeof(int)*num);
		bzero(spsum, sizeof(int)*num);

		dealer[0] = card[offset];
		c++;
		if(card[offset] / 10 == 1) dealer_sum += 10;
		else dealer_sum += card[offset];
		offset++;
		dealer[1] = card[offset];
		c++;
		if(card[offset] / 10 == 1) dealer_sum += 10;
		else dealer_sum += card[offset];
		offset++;

		printf("The dealer's handheld cards are %d %d\n", dealer[0], dealer[1]);

		for (i = 0; i < num; ++i)
		{
			handheld[i][0] = card[offset];
			heldcount[i]++;
			if(card[offset] / 10 == 1) sum[i] += 10;
			else sum[i] += card[offset];
			offset++;
			handheld[i][1] = card[offset];
			heldcount[i]++;
			if(card[offset] / 10 == 1) sum[i] += 10;
			else sum[i] += card[offset];
			offset++;
		}

		for(i = 0; i < num; ++i)
		{
			write(player[i], dealer, sizeof(int));
			write(player[i], handheld[i], sizeof(int)*2);
		}

		int bj, over[num];
		bzero(over, sizeof(int)*num);
		for(i=0;i<num;++i)
		{
			if((handheld[i][0] == 1 && (handheld[i][1] == 11 || handheld[i][1] == 12 || handheld[i][1] == 13)) || (handheld[i][1] == 1 && (handheld[i][0] == 11 || handheld[i][0] == 12 || handheld[i][0] == 13)))
			{
				if((dealer[0] == 1 && (dealer[1] == 11 || dealer[1] == 12 || dealer[1] == 13)) || (dealer[1] == 1 && (dealer[0] == 11 || dealer[0] == 12 || dealer[0] == 13)))
				{
					bj = 3;
					write(player[i], &bj, sizeof(int));
					over[i] = 1;
				}
				else
				{
					bj = 1;
					write(player[i], &bj, sizeof(int));
					over[i] = 1;
				}
			}
			else
			{
				if((dealer[0] == 1 && (dealer[1] == 11 || dealer[1] == 12 || dealer[1] == 13)) || (dealer[1] == 1 && (dealer[0] == 11 || dealer[0] == 12 || dealer[0] == 13)))
				{
					bj = 2;
					write(player[i], &bj, sizeof(int));
					over[i] = 1;
				}
				else
				{
					bj = 0;
					write(player[i], &bj, sizeof(int));
				}
			}
		}
		if((dealer[0] == 1 && (dealer[1] == 11 || dealer[1] == 12 || dealer[1] == 13)) || (dealer[1] == 1 && (dealer[0] == 11 || dealer[0] == 12 || dealer[0] == 13)))
		{
			continue;
		}

		int action = 0, spl;
		for (i = 0; i < num; ++i)
		{
			spl = 0;
			if(over[i] == 0)
			{
				while(1)
				{
					read(player[i], &action, sizeof(int));
					if(spl == 0)
					{
						if(action == 2)//Hit me
						{
							handheld[i][heldcount[i]] = card[offset];
							heldcount[i]++;
							if(card[offset] / 10 == 1) sum[i] += 10;
							else sum[i] += card[offset];
							write(player[i], &card[offset], sizeof(int));
							offset++;
							/*if(sum[i] > 21)
							{
								over[i] = 1;
								break;
							}
							else if(heldcount[i] == 5)
							{
								break;
							}*/
						}
						else if(action == 3)//Stand
						{
							printf("player%d - 1P have:", i+1);
							for(int j = 0; j < heldcount[i]; j++)
							{
								printf(" %d\n", handheld[i][j]);
							}
							printf("\n");
							spl = 1;
						}
						else if(action == 1)//Double
						{
							handheld[i][heldcount[i]] = card[offset];
							heldcount[i]++;
							if(card[offset] / 10 == 1) sum[i] += 10;
							else sum[i] += card[offset];
							write(player[i], &card[offset], sizeof(int));
							offset++;
							/*if(sum[i] > 21)
							{
								over[i] = 1;
								break;
							}
							else
							{
								break;
							}*/
						}
						else if(action == 4)
						{
							split[i][0] = handheld[i][1];
							handheld[i][1] = 0;
							if(handheld[i][0] / 10 == 1) sum[i] = 10;
							else sum[i] = handheld[i][0];
							if(split[i][0] / 10 == 1) spsum[i] += 10;
							else spsum[i] = split[i][0];
							heldcount[i] = 1;
							splitcount[i] = 1;
						}
						else if(action == 5)//player done
						{
							break;
						}
					}
					else if(spl == 1)
					{
						if(action == 2)//Hit me
						{
							split[i][splitcount[i]] = card[offset];
							splitcount[i]++;
							if(card[offset] / 10 == 1) spsum[i] += 10;
							else spsum[i] += card[offset];
							write(player[i], &card[offset], sizeof(int));
							offset++;
							/*if(sum[i] > 21)
							{
								over[i] = 1;
								break;
							}
							else if(heldcount[i] == 5)
							{
								break;
							}*/
						}
						else if(action == 3)//Stand
						{
							printf("player%d - 2P have:", i+1);
							for(int j = 0; j < splitcount[i]; j++)
							{
								printf(" %d\n", split[i][j]);
							}
							printf("\n");
						}
						else if(action == 1)//Double
						{
							split[i][splitcount[i]] = card[offset];
							splitcount[i]++;
							if(card[offset] / 10 == 1) spsum[i] += 10;
							else spsum[i] += card[offset];
							write(player[i], &card[offset], sizeof(int));
							offset++;
							/*if(sum[i] > 21)
							{
								over[i] = 1;
								break;
							}
							else
							{
								break;
							}*/
						}
						else if(action == 5)//player done
						{
							break;
						}
					}
				}
			}
		}

		int act = 0;
		while(1)
		{
			printf("1.Hit me or 2.Stand\n");
			scanf("%d", &act);

			if(act == 1)
			{
				dealer[c] = card[offset];
				c++;

				if(card[offset] / 10 == 1) dealer_sum += 10;
				else dealer_sum += card[offset];

				offset++;
				if(dealer_sum > 21)
				{
					printf("The cards in hand:");
					for(i = 0; i < c; i++)
					{
						printf(" %d", dealer[i]);
					}
					printf("\n");
					printf("Burst\n");
					break;
				}
				else if(c == 5)
				{
					printf("The cards in hand:");
					for(i = 0; i < c; i++)
					{
						printf(" %d", dealer[i]);
					}
					printf("\n");
					printf("Handheld five cards\n");
					break;
				}
				else
				{
					printf("The cards in hand:");
					for(i = 0; i < c; i++)
					{
						printf(" %d", dealer[i]);
					}
					printf("\n");
				}
			}
			else if(act == 2)
			{
				printf("The cards in hand:");
				for(i = 0; i < c; i++)
				{
					printf(" %d", dealer[i]);
				}
				printf("\n");
				break;
			}
		}
		for (i = 0; i < num; ++i)
		{
			write(player[i], &c, sizeof(int));
			write(player[i], dealer, sizeof(int)*5);
			write(player[i], &dealer_sum, sizeof(int));
		}
	}
}
