#include <iostream>
#include <string.h>
 
using namespace std;
 
// Driver code
int main()
{
  // Input string
  string s = "DD";
 
  // Initializing the return integer 
  // value of the function to a new 
  // variable
  int ans = stoi(s, 0, 16);
 
  cout << ans << endl;
  return 0;
}
