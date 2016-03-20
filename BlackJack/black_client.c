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
	int serverfd, i;
	socklen_t length;
	struct sockaddr_in ser_addr;

	serverfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[2]));
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	inet_pton(AF_INET, argv[1], &ser_addr.sin_addr);

	if(connect(serverfd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) < 0)
	{
		printf("Connect Failed\n");
	}
	else
	{
		printf("Connect Success\n");
		printf(":\n");
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
	int money = 1000;
	while(1)
	{
		int bet = 0;
		printf("--------------------division----------------------\n");
		printf("You have %d dollars\n", money);
		printf("Please bet:\n");
		scanf("%d", &bet);

		int dealer[5], handheld[2][5], heldcount[2], sum[2], bj;
		bzero(dealer, sizeof(int)*5);
		bzero(handheld, sizeof(int)*10);
		bzero(heldcount, sizeof(int)*2);
		bzero(sum, sizeof(int)*2);
		write(serverfd, buf, 1);

		read(serverfd, dealer, sizeof(int));
		printf("dealer's cards in hand: %d ?\n", dealer[0]);

		read(serverfd, handheld[0], sizeof(int)*2);
		heldcount[0] += 2;
		if(handheld[0][0] / 10 == 1) sum[0] += 10;
		else sum[0] += handheld[0][0];
		if(handheld[0][1] / 10 == 1) sum[0] += 10;
		else sum[0] += handheld[0][1];
		printf("Your cards in hand : %d %d\n", handheld[0][0], handheld[0][1]);

		read(serverfd, &bj, sizeof(int));
		if(bj == 0)
		{

		}
		else if(bj == 1)
		{
			printf("You have black jack, so you win\n");
			money += (bet * 2);
			printf("You have %d dollars!!\n", money);
			continue;
		}
		else if(bj == 2)
		{
			printf("The dealer has black jack, so you lose\n");
			money -= bet;
			printf("You have %d dollars\n", money);
			continue;
		}
		else if(bj == 3)
		{
			printf("Both you and the dealer have black jack, that is ""push""\n");
			printf("You still have %d dollars\n", money);
			continue;
		}

		int action = 0, card = 0, keep[2], n = 1, flag = 0, p = 0;
		keep[0] = 1;
		keep[1] = 1;
		if((handheld[0][0] == handheld[0][1]) || ((handheld[0][0] / 10 == 1) && (handheld[0][1] / 10 == 1)))
		{
			printf("1.Double  2.Hit Me  3.Stand  4.Split\n");
			scanf("%d", &action);
			write(serverfd, &action, sizeof(int));
			if(action == 4)
			{
				n = 2;
			}
			flag = 1;
		}
		for (int p = 0; p < n; ++p)
		{
			while(1)
			{
				if(flag == 0)
				{
					if(n == 2 && heldcount[p] == 1)
					{
						printf("The cards in 2P: %d\n", handheld[p][0]);
					}
					printf("1.Double  2.Hit Me  3.Stand\n");
					scanf("%d", &action);
					write(serverfd, &action, sizeof(int));
				}
				else flag = 0;
				
				if(action == 1)
				{
					read(serverfd, &card, sizeof(int));
					handheld[p][heldcount[p]] = card;
					heldcount[p]++;

					printf("The cards in %dP:", p + 1);
					for (i = 0; i < heldcount[p]; ++i)
					{
						printf(" %d", handheld[p][i]);
					}
					printf("\n");

					if(card / 10 == 1) sum[p] += 10;
					else sum[p] += card;

					if (sum[p] > 21)
					{
						printf("Burst!! you lose\n");
						money -= bet;
						printf("You have %d dollars\n", money);
						keep[p] = 0;
						break;
					}
					else if(heldcount[p] == 5)
					{
						printf("Five cards in hand!! you get fourfold reward\n");
						money += (bet * 4);
						printf("You have %d dollars\n", money);
						keep[p] = 0;
					}
					break;
				}
				else if(action == 2)
				{
					read(serverfd, &card, sizeof(int));
					handheld[p][heldcount[p]] = card;
					heldcount[p]++;

					printf("The cards in %dP:", p + 1);
					for (i = 0; i < heldcount[p]; ++i)
					{
						printf(" %d", handheld[p][i]);
					}
					printf("\n");

					if(card / 10 == 1) sum[p] += 10;
					else sum[p] += card;

					if (sum[p] > 21)
					{
						printf("Burst!! you lose\n");
						money -= bet;
						printf("You have %d dollars\n", money);
						keep[p] = 0;
						break;
					}
					else if(heldcount[p] == 5)
					{
						printf("Five cards in hand!! you get double reward\n");
						money += (bet * 2);
						printf("You have %d dollars\n", money);
						keep[p] = 0;
						break;
					}
				}
				else if(action == 3)
				{
					break;
				}
				else if(action == 4)
				{
					handheld[1][0] = handheld[0][1];
					handheld[0][1] = 0;
					if(handheld[0][0] / 10 == 1) sum[0] = 10;
					else sum[0] = handheld[0][0];
					if(handheld[1][0] / 10 == 1) sum[1] += 10;
					else sum[1] = handheld[1][0];
					heldcount[0] = 1;
					heldcount[1] = 1;

					printf("The cards in 1P: %d\n", handheld[0][0]);
					printf("The cards in 2P: %d\n", handheld[1][0]);
				}
			}
		}
		action = 5;
		write(serverfd, &action, sizeof(int));
		
		int dealer_sum = 0, c;
		read(serverfd, &c, sizeof(int));
		read(serverfd, dealer, sizeof(int)*5);
		read(serverfd, &dealer_sum, sizeof(int));

		printf("The dealer's card in hand:");
		for(i=0;i<c;i++)
		{
			printf(" %d", dealer[i]);
		}
		printf("\n");
		for(i=0;i<c;i++)
		{
			if((dealer[i] == 1) && (dealer_sum + 10 <= 21))
			{
				dealer_sum += 10;
				break;
			}
		}



		for(p = 0;p < n;p++)
		{
			if(keep[p])
			{
				for(i = 0; i < heldcount[p]; i++)
				{
					if((handheld[p][i] == 1) && (sum[p] + 10 <= 21))
					{
						sum[p] += 10;
						break;
					}
				}
				if(dealer_sum > 21)
				{
					money += bet;
					printf("You win\n");
					printf("You have %d dollars!!\n", money);
				}
				else if(c == 5)
				{
					money -= bet;
					printf("You lose, because the dealer has five cards in hand\n");
					printf("You have %d dollars\n", money);
				}
				else if((dealer_sum > sum[p]) && (dealer_sum <= 21))
				{
					money -= bet;
					printf("You lose\n");
					printf("You have %d dollars\n", money);
				}
				else if(dealer_sum == sum[p])
				{
					printf("Push happened\n");
					printf("You still have %d dollars\n", money);
				}
				else if(dealer_sum < sum[p])
				{
					money += bet;
					printf("You win\n");
					printf("You have %d dollars!!\n", money);
				}
			}
		}
	}	

}