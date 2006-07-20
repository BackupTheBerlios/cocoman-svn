#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char ** argv)
{
  if (argc != 3)
  {
    cerr << "Usage: mydiff [file1] [file2]\n";
    return 2;
  }

  ifstream file1, file2;
  file1.open(argv[1]);
  file2.open(argv[2]);

  if (!file1.is_open() || !file2.is_open())
  {
    cerr << "Error: could not open specified files\n";
    return 3;
  } 

  bool b1 = false;
  bool b2 = false;

  string s1, s2;

  b1 = (file1 >> s1);
  b2 = (file2 >> s2);

  while (b1 && b2)
  {
    if (s1 != s2)
    {
      cout << "Files differ: " << s1 << " != " << s2 << "\n";
      return 1;
    }

    b1 = (file1 >> s1);
    b2 = (file2 >> s2);
  }

  if (b1 != b2)
  {
    cout << "Files differ: end-of-file reached in one file but not the other\n";
    return 1;
  }

  file1.close();
  file2.close();

  return 0;
}
