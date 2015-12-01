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

int main(int argc, char** argv)
{
	cacheLine *cache = new cacheLine[64];

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

		//WRITE
		if (op == 255) {
			cout << "Previously cache at line " << line << " @ offset " << offset << " contains " << +cache[line].data[offset];
			cache[line].data[offset] = data;
			cache[line].tag = tag;
			cout << ", now has " << +cache[line].data[offset] << endl;
		}

		//READ 
		else if (op == 0) {
			//[HEX] [DATA] [HIT/MISS] [DIRTYBIT]
			cout << "Querying cache line " << +line << ": ";
			cout << hex << address << " ";
			for (int i = 0; i < 8; i++) {
				cout << +cache[line].data[i] << " ";
			}
			if (cache[line].tag == tag) {
				cout << " 1 ";
			} 
			else { 
				cout << " 0 "; 
			}
			cout << cache[line].dirtyBit << endl;
		}
	}
}

