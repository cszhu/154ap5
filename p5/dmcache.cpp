#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iomanip>
using namespace std;

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
		
	string address, op, data;
	while (!file.eof())
	{
		file >> address >> op >> data;
		cout << address << " " << op << " " << data << endl;
	}
}

