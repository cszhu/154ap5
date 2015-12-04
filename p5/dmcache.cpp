#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iomanip>
using namespace std;

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
	unsigned char RAM[65536];
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
	string _address, _op, _data;
	unsigned short address;
	unsigned char op, data;

	// Read line from file
	while (file >> _address >> _op >> _data;)
	{
		
		// Convert string -> integral value
		address = strtol(_address.c_str(), NULL, 16);
		op = strtol(_op.c_str(), NULL, 16);
		data = strtol(_data.c_str(), NULL, 16);
		//cout << "address: " << hex << + address << " op: " << + op << " data: " << + data << endl;
		
		// Split addres to cache line parts
		int tag = (address >> 9) & 0x7F;
		int line = (address >> 3) & 0x3F;
		int offset = address & 0x7;
		//cout << "tag: " << tag << " line: " << line << " offset: " << offset << endl;
		
		/*
		 * WRITE OPERATION
		 *
		 * Writes to cache and sets the dirty bit to 1.
		 * Pulls in correct cacheLine from RAM on tag mismatched.
		 */
		if (op == 0xFF)
		{
			if (cache[line].tag == 0) // First write
			{
				// First write
				cache[line].data[offset] = data;
				cache[line].tag = tag;
				// cout << "First write to cache " << line << " @ offset " << offset << " now has " << +cache[line].data[offset] << endl << endl;
			}
			else if (cache[line].tag != tag) // Mismatched tags
			{
				// cout << "Cache tag is " << cache[line].tag << " but we want " << tag << endl;
				int storeAddress = (cache[line].tag << 9) | (line << 3);
				for (int i = 0; i < 8; i++)
				{
					RAM[storeAddress + i] = cache[line].data[i];
				}
				// Replace current cache line data with the correct data we want from RAM.
				int pullAddress = (tag << 9) | (line << 3);
				//cout << "Line is " << line << " and tag is " << tag << " and concat is " << pullAddress << endl;
				for (int i = 0; i < 8; i++)
				{
					cache[line].data[i] = RAM[pullAddress + i];
				}
				// Update tag
				cache[line].tag = tag;
				// Now we have the correct cache line, so we write our data to it.
				cache[line].data[offset] = data;
				// cout << "Cache at line " << line << " tag " << tag << " @ offset " << offset << " now contains " << +cache[line].data[offset] << endl << endl;;
			}
			else // Tag matches; correct cache line
			{
				// cout << "Correct tag - line " << line << " @ offset " << offset << " now has " << +cache[line].data[offset] << endl << endl;
				cache[line].data[offset] = data;
			}
			
			// We did a write, so update the dirty bit.
			cache[line].dirtyBit = 1;
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
			outputFile << uppercase << hex << address << " ";
			int hit = cache[line].tag == tag;
			if (!hit)
			{
				// First, store current cache line data into RAM.
				int storeAddress = (cache[line].tag << 9) | (line << 3);
				for (int i = 0; i < 8; i++)
				{
					RAM[storeAddress + i] = cache[line].data[i];
				}
				// Replace current cache line data with the correct data we want from RAM.
				int pullAddress = (tag << 9) | (line << 3);
				for (int i = 0; i < 8; i++)
				{
					cache[line].data[i] = RAM[pullAddress + i];
				}
				// Update tag.
				cache[line].tag = tag;
			}
			// Print.
			for (int i = 7; i >= 0; i--)
			{
				outputFile << setfill('0') << setw(2) << uppercase << +cache[line].data[i];
			}
			outputFile << " " << hit << " " << cache[line].dirtyBit << endl;
			if (!hit)
			{
				// Update dirty bit.
				cache[line].dirtyBit = 0;
			}
		}
	}
}

