#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

#define PAGE_TABLE_SIZE	16
#define FRAME_SIZE		4

struct clock
{
	unsigned int pages[FRAME_SIZE];
	unsigned short r[FRAME_SIZE];
	unsigned short pointer;
};

struct pageTable
{
	unsigned short entryNumber;
	unsigned short present;
	unsigned int address;
	unsigned short physicalPage;
};

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "Please enter a file." << endl;
		return -1;
	}

	char* filename = argv[1];
	ifstream file(filename);
	if (!file)
	{
		cout << "Unable to open file." << endl;
		return -1;
	}

	ofstream outputFile("vm-out.txt");
	if (!outputFile)
	{
		cout << "Unable to create output file." << endl;
		return -1;
	}

	pageTable pageTable[PAGE_TABLE_SIZE];

	unsigned int address;
	
	// get page addresses
	for (int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		file >> hex >> address;
		pageTable[i].address = address;
	}

	while (file >> hex >> address)
	{
		cout << hex << address << endl;
		
		// check if page is in ram
		unsigned int pageIndex = -1;
		unsigned int prefix = address & 0xFFFFF000;
		for (int i = 0; i < FRAME_SIZE; i++)
		{
			if (pageTable[i].address == prefix)
			{
				pageIndex = i;
				break;
			}
		}

		if (pageIndex == -1)
		{
			continue;
		}

		if (pageTable[pageIndex].present)
		{
			cout << "Page is present" << endl;
		}
		else
		{
			cout << "Page is not present" << endl;
		}
	}
}