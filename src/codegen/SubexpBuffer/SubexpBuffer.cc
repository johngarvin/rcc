#include "SubexpBuffer.h"

SubexpBuffer::SubexpBuffer(std::string pref /* = "v" */, bool is_c /* = false */)
  : prefix(pref), is_const(is_c) {
  has_i = FALSE;
  prot = 0;
  decls = edefs = "";
  encl_fn = this;
}

SubexpBuffer::~SubexpBuffer() {}

SubexpBuffer &SubexpBuffer::operator=(SubexpBuffer &sb) {
  return sb;
}

