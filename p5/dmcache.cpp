#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iomanip>
using namespace std;

int main(int argc, char** argv)
{
  if (argc == 0) {
    cout << "PLZ ENTER A FILE!!" << endl;
    return 0;
  }

  char* filename;
  filename = argv[1];

  cout << filename << endl;

  ifstream file(filename);

  char line[80];
  while (file.getline(line, 80))
  {
    cout << line << endl;
   //do stuff
  } // Read Loo
}