#include "RccError.h"

#include <iostream>

void rcc_error(std::string message) {
  std::cerr << "Error: " << message << std::endl;
  exit(1);
}

void rcc_warn(std::string message) {
  std::cerr << "Warning: " << message << std::endl;
}
