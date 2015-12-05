#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

struct address
{
	unsigned short tag;
	unsigned char line;
	unsigned char offset;

	address(unsigned short address)
	{
		tag = address >> 9;
		line = (address >> 3) & 0x3F;	// 7 bit line
		offset = address & 0x7;			// 3 bit offset
	}
};

struct cacheLine
{
	int dirtyBit;
	int tag;
	unsigned char data[8];
};

/*
 * Sets the contents of the cache to 0.
 */
void initializeCache(cacheLine* cache)
{
	for (int i = 0; i < 64; i++)
	{
		cache[i].dirtyBit = 0;
		cache[i].tag = 0;
		for (int j = 0; j < 8; j++)
		{
			cache[i].data[j] = 0;
		}
	}
}

/*
 * Prints all non-empty lines of cache for debugging purposes.
 */
void printCache(cacheLine* cache)
{
	for (int i = 0; i < 64; i++)
	{
		if (cache[i].tag == 0) continue;
		cout << "Line " << setfill('0') << setw(2) << i << ": ";
		cout << +cache[i].dirtyBit << " " << setw(2) << +cache[i].tag << " ";
		for (int j = 7; j >= 0; j--)
		{
			cout << setw(2) << +cache[i].data[j];
		}
		cout << endl;
	}
	cout << endl;
}

int main(int argc, char** argv)
{
	cacheLine *cache = new cacheLine[64];
	unsigned char RAM[65536] = { 0 };
	ofstream outputFile;
	outputFile.open("dm-out.txt");
	initializeCache(cache);
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

	unsigned short memaddress;
	unsigned short op, data;

	// Read line from file
	while (file >> hex >> memaddress >> op >> data)
	{
		address address(memaddress);

		/*
		 * WRITE OPERATION
		 *
		 * Writes to cache and sets the dirty bit to 1.
		 * Pulls in correct cacheLine from RAM on tag mismatched.
		 */
		if (op == 0xFF)
		{
			if (cache[address.line].tag == 0) // First write
			{
				// First write
				cache[address.line].data[address.offset] = data;
				cache[address.line].tag = address.tag;
				// cout << "First write to cache " << line << " @ offset " << offset << " now has " << +cache[line].data[offset] << endl << endl;
			}
			else if (cache[address.line].tag != address.tag) // Mismatched tags
			{
				// cout << "Cache tag is " << cache[line].tag << " but we want " << tag << endl;
				int storeAddress = (cache[address.line].tag << 9) | (address.line << 3);
				for (int i = 0; i < 8; i++)
				{
					RAM[storeAddress + i] = cache[address.line].data[i];
				}
				// Replace current cache line data with the correct data we want from RAM.
				int pullAddress = (address.tag << 9) | (address.line << 3);
				//cout << "Line is " << line << " and tag is " << tag << " and concat is " << pullAddress << endl;
				for (int i = 0; i < 8; i++)
				{
					cache[address.line].data[i] = RAM[pullAddress + i];
				}
				// Update tag
				cache[address.line].tag = address.tag;
				// Now we have the correct cache line, so we write our data to it.
				cache[address.line].data[address.offset] = data;
				// cout << "Cache at line " << line << " tag " << tag << " @ offset " << offset << " now contains " << +cache[line].data[offset] << endl << endl;;
			}
			else // Tag matches; correct cache line
			{
				// cout << "Correct tag - line " << line << " @ offset " << offset << " now has " << +cache[line].data[offset] << endl << endl;
				cache[address.line].data[address.offset] = data;
			}

			// We did a write, so update the dirty bit.
			cache[address.line].dirtyBit = 1;
		}
		/*
		 * READ OPERATION
		 *
		 * Prints out requested cache line in the format
		 * [ADDRESS] [DATA] [HIT] [DIRTY BIT]
		 * Pulls data from RAM into cache on tag mismatched.
		 */
		else if (op == 0)
		{
			outputFile << uppercase << hex << setfill('0') << setw(4) << memaddress << " ";
			int hit = cache[address.line].tag == address.tag;
			if (!hit)
			{
				// First, store current cache line data into RAM.
				int storeAddress = (cache[address.line].tag << 9) | (address.line << 3);
				for (int i = 0; i < 8; i++)
				{
					RAM[storeAddress + i] = cache[address.line].data[i];
				}
				// Replace current cache line data with the correct data we want from RAM.
				int pullAddress = (address.tag << 9) | (address.line << 3);
				for (int i = 0; i < 8; i++)
				{
					cache[address.line].data[i] = RAM[pullAddress + i];
				}
				// Update tag.
				cache[address.line].tag = address.tag;
			}
			// Print.
			for (int i = 7; i >= 0; i--)
			{
				outputFile << setfill('0') << setw(2) << uppercase << +cache[address.line].data[i];
			}
			outputFile << " " << hit << " " << cache[address.line].dirtyBit << endl;
			if (!hit)
			{
				// Update dirty bit.
				cache[address.line].dirtyBit = 0;
			}
		}
	}
}

