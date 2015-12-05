#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#define RAM_CAPACITY	0x10000
#define CACHE_CAPACITY	512
#define BLOCK_SIZE		8
#define NUM_LINES		CACHE_CAPACITY / BLOCK_SIZE
#define LINES_PER_SET	4
#define NUM_SETS		NUM_LINES / LINES_PER_SET
#define TAG_SHIFT		7
#define SET_SHIFT		3

struct address
{
	unsigned short tag;
	unsigned char set;
	unsigned char offset;

	address(unsigned short address)
	{
		tag = address >> 7;
		set = (address >> 3) & 0xF;	// 4 bit set (4 sets/line)
		offset = address & 0x7;		// 3 bit offset (8 bytes/line)
	}
};

struct cacheLine
{
	unsigned char dirty;
	unsigned short tag;
	unsigned char age;
	unsigned char data[BLOCK_SIZE];
};

struct cacheSet
{
	cacheLine lines[LINES_PER_SET];

	/*
	 * Gets the line with the matching tag or the oldest line in the set.
	 */
	cacheLine * getWriteLine(unsigned short tag)
	{
		cacheLine * oldestLine = NULL;
		for (int i = 0; i < LINES_PER_SET; i++)
		{
			if (lines[i].tag == tag)
				return &lines[i];

			if (oldestLine == NULL || lines[i].age > oldestLine->age)
				oldestLine = &lines[i];
		}
		return oldestLine;
	}
};

void initializeCache(cacheSet* cache)
{
	for (int i = 0; i < NUM_SETS; i++)
	{
		for (int j = 0; j < LINES_PER_SET; j++)
		{
			cache[i].lines[j].dirty = 0;
			cache[i].lines[j].tag = 0;
			cache[i].lines[j].age = j;
			for (int k = 0; k < BLOCK_SIZE; k++)
			{
				cache[i].lines[j].data[k] = 0;
			}
		}
	}
}

/*
 * Stores the current contents of writeLine into RAM,
 * then loads the address from RAM into writeLine.
 */
void cacheMiss(unsigned char RAM[65536], address address, cacheLine* writeLine)
{
	unsigned short storeAddress = (writeLine->tag << TAG_SHIFT) | (address.set << SET_SHIFT);
	for (int i = BLOCK_SIZE - 1; i >= 0; i--)
	{
		RAM[storeAddress + i] = writeLine->data[i];
	}

	unsigned short loadAddress = (address.tag << TAG_SHIFT) | (address.set << SET_SHIFT);
	for (int i = BLOCK_SIZE - 1; i >= 0; i--)
	{
		writeLine->data[i] = RAM[loadAddress + i];
	}
	writeLine->tag = address.tag;
	writeLine->dirty = 0;
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
	unsigned char RAM[RAM_CAPACITY] = { 0 };

	initializeCache(cache);

	unsigned short memaddress;
	unsigned short op, data;

	// Read line from file
	while (file >> hex >> memaddress >> op >> data)
	{
		address address(memaddress);

		// Oldest or matching tag line.
		cacheLine * writeLine = cache[address.set].getWriteLine(address.tag);
		char hit = writeLine->tag == address.tag;

		switch (op)
		{
			/*
			 * READ OPERATION
			 *
			 * Prints out requested cache line in the format
			 * [ADDRESS] [DATA] [HIT] [DIRTY BIT]
			 * Pulls data from RAM into cache on tag mismatched.
			 */
			case 0x00:
			{
				outputFile << hex << uppercase << setfill('0') << setw(4) << memaddress << " ";
				char wasDirty = writeLine->dirty;

				// If miss, store line to RAM, then load correct data to cache.
				if (!hit)
				{
					cacheMiss(RAM, address, writeLine);
				}

				// Age all lines and set current age to 0.
				for (int i = 0; i < LINES_PER_SET; i++)
				{
					cache[address.set].lines[i].age++;
				}
				writeLine->age = 0;

				for (int i = BLOCK_SIZE - 1; i >= 0; i--)
				{
					outputFile << setw(2) << +writeLine->data[i];
				}
				outputFile << " " << +hit << " " << +wasDirty << endl;
			}
			break;

			/*
			 * WRITE OPERATION
			 *
			 * Writes to cache and sets the dirty bit to 1.
			 * Pulls in correct cacheLine from RAM on tag mismatched.
			 */
			case 0xFF:
			{

				// If tags don't match, store line to RAM, then load correct data to cache.
				if (!hit)
				{
					cacheMiss(RAM, address, writeLine);
				}

				// Write tag & data and set dirty bit.
				writeLine->dirty = 1;
				writeLine->tag = address.tag;
				writeLine->data[address.offset] = data;

				// Age all lines and set current age to 0.
				for (int i = 0; i < LINES_PER_SET; i++)
				{
					cache[address.set].lines[i].age++;
				}
				writeLine->age = 0;
			}
			break;
		}
	}
}

