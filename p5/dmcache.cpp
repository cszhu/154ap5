#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iomanip>
using namespace std;

struct cacheLine {
	int dirtyBit;
	int tag;
	unsigned char data[8];
};

/*
 * Concatenates y to x and returns it.
 * ex. concat(23, 43) -> 2343
 */
int concat(int x, int y) {
    int pow = 10;
    while(y >= pow)
        pow *= 10;
    return x * pow + y;        
}

/*
 * Prints the data held in the cache line.
 */
void printCacheLine(int line, cacheLine* cache) {
	for (int i = 7; i > -1; i--) {
		unsigned char output;
		output = toupper(cache[line].data[i]);
		printf("%02x", output);
	}
}

int main(int argc, char** argv)
{
	cacheLine *cache = new cacheLine[64];
	unsigned char RAM[65536];

	// Initializing cache to 0.
	for (int i = 0; i < 64; i++) {
		cache[i].dirtyBit = 0;
		cache[i].tag = 0;
		for (int j = 0; j < 8; j++) {
			cache[i].data[j] = 00;
		}
	}

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
	short address;
	unsigned char op, data;
	while (!file.eof())
	{
		file >> _address >> _op >> _data;
		address = strtol(_address.c_str(), NULL, 16);
		op = strtol(_op.c_str(), NULL, 16);
		data = strtol(_data.c_str(), NULL, 16);
		// cout << "address: " << +address << " op: " << +op << " data:" << +data << endl;

		int tag = (address >> 9) & 0x7F;
		int line = (address >> 3) & 0x3F;
		int offset = address & 0x7;
		// cout << "tag: " << tag << " line: " << line << " offset: " << offset << endl;

		/* 
		 * WRITE FUNCTION
		 *
		 * Writes to cache. Sets the dirty bit to 1.
		 * Pulls in correct cacheLine from RAM if tag is mismatched.
		 */
		if (op == 255) {
			// First write.
			if (cache[line].tag == 0) {
				cache[line].data[offset] = data;
				cache[line].tag = tag;
				// cout << "First write to cache " << line << " @ offset " << offset << " now has " << +cache[line].data[offset] << endl << endl;
			}
			// Mismatched tags.
			else if (cache[line].tag != tag) {
				// cout << "Cache tag is " << cache[line].tag << " but we want " << tag << endl;
				int storeAddress = concat(line, cache[line].tag);
				storeAddress = concat(storeAddress, 0); // Set offset to 0.
				for (int i = 0; i < 8; i++) {
					RAM[storeAddress + i] = cache[line].data[i];
				}
			
				// Replace current cache line data with the correct data we want from RAM.
				int pullAddress = concat(line, tag);
				pullAddress = concat(pullAddress, 0); // Set offset to 0.
				for (int i = 0; i < 8; i++) {
					cache[line].data[i] = RAM[pullAddress + i];
				}

				// Update tag.
				cache[line].tag = tag;

				// Now we have the correct cache line, so we write our data to it.
				cache[line].data[offset] = data;

				// cout << "Cache at line " << line << " tag " << tag << " @ offset " << offset << " now contains " << +cache[line].data[offset] << endl << endl;;
			}
			// Tag matches. Correct cache line.
			else {
				// cout << "Correct tag - line " << line << " @ offset " << offset << " now has " << +cache[line].data[offset] << endl << endl;
				cache[line].data[offset] = data;
			}

			// We did a write, so update the dirty bit.
			cache[line].dirtyBit = 1;
		}

		/* 
		 * READ FUNCTION
		 *
		 * Prints out requested cache line in the form 
		 * [HEX ADDRESS] [DATA] [HIT/MISS] [DIRTY BIT].
		 * Pulls data from RAM into cache if tag is mismatched.
		 */
			else if (op == 0) {
			cout << hex << address << " ";

			if (cache[line].tag != tag) {
				// First, store current cache line data into RAM.
				int storeAddress = concat(line, cache[line].tag);
				storeAddress = concat(storeAddress, 0); // Set offset to 0.
				for (int i = 0; i < 8; i++) {
					RAM[storeAddress + i] = cache[line].data[i];
				}

				// Replace current cache line data with the correct data we want from RAM.
				int pullAddress = concat(line, tag);
				pullAddress = concat(pullAddress, 0); // Set offset to 0.
				for (int i = 0; i < 8; i++) {
					cache[line].data[i] = RAM[pullAddress + i];
				}

				// Update tag.
				cache[line].tag = tag;

				// Print.
				printCacheLine(line, cache);
				cout << " 0 ";
				cout << cache[line].dirtyBit << endl;

				// Update dirty bit.
				cache[line].dirtyBit = 0;
			} 
			else { 
				printCacheLine(line, cache);
				cout << " 1 ";
				cout << cache[line].dirtyBit << endl;
			}
		}

	} // End of file
} // End of main

