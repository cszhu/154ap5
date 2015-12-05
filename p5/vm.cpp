#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

#define PAGE_TABLE_SIZE	16
#define FRAME_SIZE		4

struct RAM
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

void printRAM(ofstream& output, RAM RAM)
{
	for (int i = 0; i < FRAME_SIZE; i++)
	{
		if (RAM.pages[i])
		{
			if (i > 0)
				output << " ";
			output << hex << setfill('0') << setw(8) << (RAM.pages[i] & 0xFFFFF000);
		}
	}
	output << endl;
}

unsigned int getPageIndex(pageTable pageTable[PAGE_TABLE_SIZE], unsigned int address)
{
	unsigned int pageIndex = -1;
	unsigned int prefix = address & 0xFFFFF000; // first 12 bits are offset
	for (int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		if (pageTable[i].address == prefix)
		{
			pageIndex = i;
			break;
		}
	}
	return pageIndex;
}

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
	for (int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		pageTable[i].entryNumber = 0;
		pageTable[i].present = 0;
		pageTable[i].address = 0;
		pageTable[i].physicalPage = 0;
	}

	RAM RAM;
	for (int i = 0; i < FRAME_SIZE; i++)
	{
		RAM.pages[i] = 0;
		RAM.r[i] = 0;
	}
	RAM.pointer = 0;

	unsigned int address;

	// get page addresses
	for (int i = 0; i < PAGE_TABLE_SIZE; i++)
	{
		file >> hex >> address;
		pageTable[i].address = address;
	}

	while (file >> hex >> address)
	{
		// check if page is in ram
		unsigned int pageIndex = getPageIndex(pageTable, address);

		if (pageIndex == -1)
		{
			continue;
		}

		unsigned int entryNumber;
		if (pageTable[pageIndex].present)
		{
			entryNumber = pageTable[pageIndex].entryNumber;
			RAM.r[entryNumber] = 1;
		}
		else
		{
			while (1)
			{
				RAM.r[RAM.pointer] = !RAM.r[RAM.pointer];
				if (RAM.r[RAM.pointer])
				{
					// clear present bit of old page
					unsigned int oldPagePrefix = getPageIndex(pageTable, RAM.pages[RAM.pointer]);
					pageTable[oldPagePrefix].present = 0;

					// set present bit of new page and place page in RAM
					pageTable[pageIndex].present = 1;
					pageTable[pageIndex].entryNumber = RAM.pointer;
					RAM.pages[RAM.pointer] = address;

					// increment pointer and break
					RAM.pointer = (RAM.pointer + 1) % FRAME_SIZE;
					break;
				}
				else
				{
					RAM.r[RAM.pointer] = 0;
					// increment pointer
					RAM.pointer = (RAM.pointer + 1) % FRAME_SIZE;
				}
			}
		}

		printRAM(outputFile, RAM);
	}
}