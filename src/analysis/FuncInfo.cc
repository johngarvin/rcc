#include <include/R/R_RInternals.h>

#include <ParseInfo.h>
#include <codegen/SubexpBuffer/SubexpBuffer.h>

#include <support/DumpMacros.h>
#include <support/StringUtils.h>

#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/FormalArgInfo.h>
#include <analysis/FuncInfoAnnotationMap.h>
#include <analysis/RequiresContext.h>
#include <analysis/Utils.h>
#include <analysis/VarBinding.h>

#include "FuncInfo.h"

namespace RAnnot {

//****************************************************************************
// FuncInfo
//****************************************************************************

typedef FuncInfo::mention_iterator mention_iterator;
typedef FuncInfo::const_mention_iterator const_mention_iterator;
typedef FuncInfo::call_site_iterator call_site_iterator;
typedef FuncInfo::const_call_site_iterator const_call_site_iterator;

FuncInfo::FuncInfo(FuncInfo* parent, SEXP name, SEXP defn) :
  m_parent(parent),
  m_first_name(name),
  m_defn(defn),
  m_c_name(""),
  m_closure(""),
  NonUniformDegreeTreeNodeTmpl<FuncInfo>(parent)
{
  m_requires_context = functionRequiresContext(defn);
  SEXP args = get_args();
  for (SEXP e = args; e != R_NilValue; e = CDR(e)) {
    FormalArgInfo * formal_info = new FormalArgInfo();
    putProperty(FormalArgInfo, e, formal_info, false);
  }
}

FuncInfo::~FuncInfo()
{
}

unsigned int FuncInfo::get_num_args() const 
{
  return m_num_args;
}

void FuncInfo::set_num_args(unsigned int x) 
{
  m_num_args = x;
}

SEXP FuncInfo::get_defn()
{
  return m_defn;
}

SEXP FuncInfo::get_first_name()
{
  return m_first_name;
}

bool FuncInfo::get_has_var_args() const
{
  return m_has_var_args;
}

void FuncInfo::set_has_var_args(bool x)
{
  m_has_var_args = x;
}

void FuncInfo::set_requires_context(bool requires_context) 
{ 
  m_requires_context = requires_context; 
}


bool FuncInfo::requires_context() 
{ 
  return m_requires_context;
}

SEXP FuncInfo::get_args() 
{ 
  return CAR(fundef_args_c(m_defn)); 
}

int FuncInfo::find_arg_position(char* name)
{
  SEXP args = get_args();
  int pos = 1;
  SEXP e;
  for (e = args; e != R_NilValue; e = CDR(e), pos++) {
    char* arg_name = CHAR(PRINTNAME(INTERNAL(e)));
    if (!strcmp(arg_name, name)) break;
  }
  assert (e != R_NilValue);
  return pos;
}

SEXP FuncInfo::get_arg(int position)
{
  SEXP args = get_args();
  int p = 1;
  SEXP e;
  for (e = args; e != R_NilValue && p != position; e = CDR(e), p++);
  assert (e != R_NilValue);
  return e;
}

bool FuncInfo::is_arg_value(SEXP arg)
{
  FormalArgInfo* formal_info = getProperty(FormalArgInfo, arg);
  bool isvalue = formal_info->isValue();
  return isvalue;
}

bool FuncInfo::are_all_value()
{
  bool allvalue = true;
  SEXP args = get_args();
  for (SEXP e = args; e != R_NilValue && allvalue; e = CDR(e)) {
    if (!is_arg_value(e)) allvalue = false;
  }
  return allvalue;
}

const std::string& FuncInfo::get_c_name()
{
  if (m_c_name == "") {
    SEXP name_sym = get_first_name();
    if (name_sym == R_NilValue) {
      m_c_name = make_c_id("anon" + ParseInfo::global_fundefs->new_var_unp());
    } else {
      m_c_name = make_c_id(ParseInfo::global_fundefs->new_var_unp_name(var_name(name_sym)));
    }
  }
  return m_c_name;
}

const std::string& FuncInfo::get_closure()
{
  if (m_closure == "") {
    m_closure = ParseInfo::global_fundefs->new_sexp_unp();
  }
  return m_closure;
}

OA::OA_ptr<OA::CFG::Interface> FuncInfo::get_cfg() const
{
  return m_cfg;
}

void FuncInfo::set_cfg(OA::OA_ptr<OA::CFG::Interface> x)
{
  m_cfg = x;
}

// mention iterators

mention_iterator FuncInfo::begin_mentions()
{
  return m_mentions.begin();
}

const_mention_iterator FuncInfo::begin_mentions() const
{
  return m_mentions.begin();
}

mention_iterator FuncInfo::end_mentions()
{
  return m_mentions.end();
}

const_mention_iterator FuncInfo::end_mentions() const
{
  return m_mentions.end();
}

// call site iterators

call_site_iterator FuncInfo::begin_call_sites()
{
  return m_call_sites.begin();
}

const_call_site_iterator FuncInfo::begin_call_sites() const
{
  return m_call_sites.begin();
}

call_site_iterator FuncInfo::end_call_sites()
{
  return m_call_sites.end();
}

const_call_site_iterator FuncInfo::end_call_sites() const
{
  return m_call_sites.end();
}


void FuncInfo::insert_mention(FuncInfo::MentionT v)
{
  m_mentions.push_back(v);
}

void FuncInfo::insert_call_site(FuncInfo::CallSiteT cs)
{
  m_call_sites.push_back(cs);
}

std::ostream& FuncInfo::dump(std::ostream& os) const
{
  beginObjDump(os, FuncInfo);
  dumpPtr(os, this);
  dumpSEXP(os, m_first_name);
  dumpVar(os, m_num_args);
  dumpVar(os, m_has_var_args);
  dumpVar(os, m_c_name);
  dumpVar(os, m_requires_context);
  R_Analyst::get_instance()->dump_cfg(os, m_defn); // can't call CFG::dump; it requires the IRInterface
  dumpSEXP(os, m_defn);
  dumpPtr(os, m_parent);
  os << "Begin mentions:" << std::endl;
  for(const_mention_iterator i = begin_mentions(); i != end_mentions(); ++i) {
    Var * v = *i;
    v->dump(os);
    VarBinding * vb = getProperty(VarBinding, (*i)->getMention_c());
    vb->dump(os);
  }
  os << "End mentions" << std::endl;
  endObjDump(os, FuncInfo);
}

PropertyHndlT FuncInfo::handle() {
  return FuncInfoAnnotationMap::handle();
}

}
