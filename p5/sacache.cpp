#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
using namespace std;

#define RAM_CAPACITY	0x10000
#define CACHE_CAPACITY	512
#define BLOCK_SIZE		8
#define NUM_LINES		CACHE_CAPACITY / BLOCK_SIZE
#define LINES_PER_SET	4
#define LINE_SIZE		BLOCK_SIZE / LINES_PER_SET
#define NUM_SETS		NUM_LINES / LINES_PER_SET

struct cacheLine
{
	unsigned char dirty;
	unsigned short tag;
	unsigned char data[LINE_SIZE];
};

struct cacheSet
{
	cacheLine lines[LINES_PER_SET];
};

void initializeCache(cacheSet* cache)
{
	for (int i = 0; i < NUM_SETS; i++)
	{
		for (int j = 0; j < LINES_PER_SET; j++)
		{
			cache[i].lines[j].dirty = 0;
			cache[i].lines[j].tag = 0;
			for (int k = 0; k < LINE_SIZE; k++)
			{
				cache[i].lines[j].data[k] = 0;
			}
		}
	}
}

void printCache(cacheSet* cache)
{
	cout << hex;
	for (int i = 0; i < NUM_SETS; i++)
	{
		for (int j = 0; j < LINES_PER_SET; j++)
		{
			cout << +cache[i].lines[j].dirty << " " << cache[i].lines[j].tag << " ";
			for (int k = 0; k < LINE_SIZE; k++)
			{
				cout << setfill('0') << setw(2) << +cache[i].lines[j].data[k];
			}
			cout << " ";
		}
		cout << endl;
	}
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
	
	ofstream outputFile("sa-out.txt");
	if (!outputFile)
	{
		cout << "Unable to create output file." << endl;
		return -1;
	}

	cacheSet * cache = new cacheSet[NUM_SETS];
	unsigned char RAM[RAM_CAPACITY];

	initializeCache(cache);
	printCache(cache);

	unsigned short address;
	unsigned short op, data;
	
	// Read line from file
	while (file >> hex >> address >> op >> data)
	{
		cout << "address: " << hex << uppercase << setfill('0') << setw(4) << address << " op: " << setw(2) << op << " data: " << setw(2) << data << endl;

		// Split addres to cache line parts
		int tag = (address >> 9) & 0x7F;
		int line = (address >> 3) & 0x3F;
		int offset = address & 0x7;
		//cout << "tag: " << tag << " line: " << line << " offset: " << offset << endl;

		switch (op)
		{
			/*
			 * READ OPERATION
			 *
			 * Prints out requested cache line in the format
			 * [ADDRESS] [DATA] [HIT] [DIRTY BIT]
			 * Pulls data from RAM into cache on tag mismatched.
			 */
			case 0x0:

				break;

				/*
				 * WRITE OPERATION
				 *
				 * Writes to cache and sets the dirty bit to 1.
				 * Pulls in correct cacheLine from RAM on tag mismatched.
				 */
			case 0xFF:
				break;
		}
	}
}

