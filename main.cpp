#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <bitset>
#include <chrono>
#include <iomanip>
#include <pthread.h>

#include <future>

#define NTHREADS 6
static int countFromFile;

using namespace std;

struct Addresses
{
	int ipaddress[4];
	int mask[4];
	int networkAdd[4];
	int broadcastAdd[4];
	int minHost[4];
	int maxHost[4];
	int numHost;
};

struct octets
{
	int ip;
	int mask;
	int net;
	int broad;
};

void *GettingNetBroad(void *netBroad)
{
	struct octets *NetB = (struct octets *)netBroad;

	NetB->net = (NetB->ip & NetB->mask);
	uint8_t temp;
	temp = ~(NetB->mask);
	NetB->broad = NetB->net | temp;

	return 0;
}

// Convert from a array to a string
string ConvertToString(int add[])
{
	ostringstream actetString;
	for (int i = 0; i < 4; i++)
	{
		if (i > 0)
			actetString << '.';

		actetString << add[i];
	}

	return actetString.str();
}

void GetMinHost(int netAdd[], int minHost[])
{

	for (int i = 0; i < 4; i++)
	{
		if (i == 3)
		{
			minHost[i] = netAdd[i] + 1;
		}
		else
			minHost[i] = netAdd[i];
	}
}

void GetMaxHost(int broadcastAdd[], int maxHost[])
{

	for (int i = 0; i < 4; i++)
	{
		if (i == 3)
		{
			maxHost[i] = broadcastAdd[i] - 1;
		}
		else
			maxHost[i] = broadcastAdd[i];
	}
}

// Get hosts per subnet
int NumberOfHost(int decimalMask[])
{
	int hostBits = 0;
	for (int i = 0; i < 4; i++)
	{
		if (decimalMask[i] == 255)
		{
			if (i == 3)
				return 0;
			hostBits += 0;
			continue;
		}
		else if (decimalMask[i] == 254)
		{
			hostBits += 1;
			continue;
		}
		else if (decimalMask[i] == 252)
		{
			hostBits += 2;
			continue;
		}
		else if (decimalMask[i] == 248)
		{
			hostBits += 3;
			continue;
		}
		else if (decimalMask[i] == 240)
		{
			hostBits += 4;
			continue;
		}
		else if (decimalMask[i] == 224)
		{
			hostBits += 5;
			continue;
		}
		else if (decimalMask[i] == 192)
		{
			hostBits += 6;
			continue;
		}
		else if (decimalMask[i] == 128)
		{
			hostBits += 7;
			continue;
		}
		else if (decimalMask[i] == 0)
		{
			hostBits += 8;
			continue;
		}
		else
		{
			hostBits = 0;
			break;
		}
	}
	int hostsPerSubnet = pow(2.0, hostBits) - 2;
	return hostsPerSubnet;
}

void print(Addresses add)
{
	std::cout << "IP Address: " << ConvertToString(add.ipaddress) << endl;
	std::cout << "Subnet: " << ConvertToString(add.mask) << endl;
	std::cout << "Network: " << ConvertToString(add.networkAdd) << endl;
	std::cout << "Broadcast: " << ConvertToString(add.broadcastAdd) << endl;
	std::cout << "HostMin: " << ConvertToString(add.minHost) << endl;
	std::cout << "HostMax: " << ConvertToString(add.maxHost) << endl;
	std::cout << "# Hosts: " << add.numHost << endl;
	std::cout << endl;
}

void *parentFunction(void *ipAdd)
{
	struct Addresses *adds = (struct Addresses *)ipAdd;

	octets oct[4];
	for (int i = 0; i < 4; i++)
	{
		oct[i].ip = adds->ipaddress[i];
		oct[i].mask = adds->mask[i];
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	THERE FOUR FUNCTIONS HAVE TO RECIVE ONE OCTET AND CALCULATE EACH IDIVIDUAL PART OF THEIR RESPECTIVE PART
	// /////////////////////////////////////////////////////////////////////////////////////////////////////////

	pthread_t tid2[4];
	for (int i = 0; i < 4; i++)
	{
		pthread_create(&tid2[i], NULL, GettingNetBroad, &oct[i]);
	}

	for (int i = 0; i < 4; i++)
		pthread_join(tid2[i], NULL);

	for (int i = 0; i < 4; i++)
	{
		adds->networkAdd[i] = oct[i].net;
		adds->broadcastAdd[i] = oct[i].broad;
	}
	GetMinHost(adds->networkAdd, adds->minHost);
	GetMaxHost(adds->broadcastAdd, adds->maxHost);
	adds->numHost = NumberOfHost(adds->mask);
	return 0;
}

int main()
{

	string temp;
	countFromFile = 0;

	string ip;
	string mask;

	
	string temp1, temp2;
	vector <string> ipV;
	vector <string> maskV;
	while (cin >> ip >> mask)
	{

		stringstream ipstr(ip);
		stringstream maskstr(mask);


		ipV.push_back(ip);
		maskV.push_back(mask);

		countFromFile++;
	}

	static struct Addresses *adds;
	adds = new struct Addresses[countFromFile];
	int j = 0;
	for(int i = 0; i < countFromFile; i++){
		stringstream getVecIP(ipV[i]);
		stringstream getVecMask(maskV[i]);

		while (getline(getVecIP, temp1, '.'))
		{
			adds[i].ipaddress[j] = stoi(temp1);
			j++;
		}
		j = 0;
		while (getline(getVecMask, temp2, '.'))
		{
			adds[i].mask[j] = stoi(temp2);
			j++;
		}
		j = 0;
	}
	


	int NUM_OF_THREADS = countFromFile;
	pthread_t *tid = new pthread_t[NUM_OF_THREADS];

	for (int i = 0; i < NUM_OF_THREADS; i++)
	{
		if (pthread_create(&tid[i], NULL, parentFunction, &adds[i]))
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
		}
	}

	for (int i = 0; i < NUM_OF_THREADS; i++)
		pthread_join(tid[i], NULL);

	for (int i = 0; i < NUM_OF_THREADS; i++)
		print(adds[i]);

	delete[] adds;
	delete[] tid;
	return 0;
}