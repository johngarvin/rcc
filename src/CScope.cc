#include "CScope.h"

#include <support/StringUtils.h>

CScope::CScope(std::string _prefix) : m_prefix(_prefix)
  {}

const std::string CScope::new_label() const {
  return m_prefix + "_" + i_to_s(m_id++);
}

unsigned int CScope::m_id = 0;
