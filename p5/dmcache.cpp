#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iomanip>
using namespace std;

struct cacheLine {
	int dirtyBit;
	char tag;
	char data[8];
};

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		cout << "PLZ ENTER A FILE!!" << endl;
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
		cout << hex << address << " " << +op << " " << +data << endl;
		
		int tag = (address >> 9) & 0x7F;
		int line = (address >> 3) & 0x3F;
		int offset = address & 0x7;
		cout << hex << tag << " " << line << " " << offset << endl;
	}
}

