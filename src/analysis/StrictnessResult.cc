#include <analysis/Analyst.h>
#include <analysis/HandleInterface.h>

#include "StrictnessResult.h"

namespace Strictness {

StrictnessResult::StrictnessResult(OA::OA_ptr<Strictness::DFSet> args_on_exit,
			    OA::OA_ptr<NameMentionMultiMap> debuts,
			    OA::OA_ptr<NameStmtMultiMap> post_debut_stmts)
    : m_args_on_exit(args_on_exit),
      m_debuts(debuts),
      m_post_debut_stmts(post_debut_stmts)
{
}

OA::OA_ptr<Strictness::DFSet> StrictnessResult::get_args_on_exit() const {
  return m_args_on_exit;
}

OA::OA_ptr<NameMentionMultiMap> StrictnessResult::get_debuts() const {
  return m_debuts;
}
  

OA::OA_ptr<NameStmtMultiMap> StrictnessResult::get_post_debut_stmts() const {
  return m_post_debut_stmts;
}

void StrictnessResult::dump(std::ostream & os) {
  os << "{ Strictness result: ";

  os << "args on exit: ";
  m_args_on_exit->dump(os, R_Analyst::get_instance()->get_interface());

  os << "debuts: ";
  for(NameMentionMultiMap::const_iterator it = m_debuts->begin(); it != m_debuts->end(); ++it) {
    Rf_PrintValue(CAR(it->first));
  }
  os << std::endl;

  os << "post-debut statements: ";
  for(NameStmtMultiMap::const_iterator it = m_post_debut_stmts->begin(); it != m_post_debut_stmts->end(); ++it) {
    Rf_PrintValue(TAG(it->first));
    Rf_PrintValue(CAR(HandleInterface::make_sexp(it->second)));
  }
  os << "}" << std::endl;
}

}  // end namespace Strictness
