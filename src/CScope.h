// -*- Mode: C++ -*-
#ifndef C_SCOPE_H
#define C_SCOPE_H

#include <string>

class CScope {
public:
  explicit CScope(std::string _prefix);
  const std::string new_label();
private:
  const std::string m_prefix;
  static unsigned int m_id;
};

#endif
