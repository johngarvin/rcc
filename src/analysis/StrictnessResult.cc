#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>

#include "StrictnessResult.h"

namespace Strictness {

void StrictnessResult::dump(std::ostream & os) {
  os << "{ Strictness result: ";

  os << "args on exit: ";
  m_args_on_exit->dump(os, R_Analyst::get_instance()->get_interface());

  os << "debuts: ";
  for(NameMentionMultiMap::const_iterator it = m_debuts->begin(); it != m_debuts->end(); ++it) {
    Rf_PrintValue(it->first);
    Rf_PrintValue(it->second);
  }
  os << std::endl;

  os << "post-debut statements: ";
  for(NameStmtMultiMap::const_iterator it = m_post_debut_stmts->begin(); it != m_post_debut_stmts->end(); ++it) {
    Rf_PrintValue(it->first);
    Rf_PrintValue(HandleInterface::make_sexp(it->second));
  }
  os << "}" << std::endl;
}

}  // end namespace Strictness
