#include <iostream>

int main(int argc, char *argv[])
{
  std::pair<int,int> p(10,20);
  std::pair<int,int> q(10,20);
  std::pair<int,int> r(10,200);

  std::cout << "Testing...\n";
  if (p == q) std::cout << "p == q\n";
  if (q == r) std::cout << "q == r\n";
}
