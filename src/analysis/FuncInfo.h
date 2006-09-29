#ifndef ANNOTATION_FUNC_INFO_H
#define ANNOTATION_FUNC_INFO_H

#include <list>

#include <include/R/R_RInternals.h>

#include <OpenAnalysis/CFG/Interface.hpp>

#include <support/trees/NonUniformDegreeTreeTmpl.h>

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>
#include <analysis/Var.h>

namespace RAnnot {

//****************************************************************************
// Annotations: Function info
//****************************************************************************


// ---------------------------------------------------------------------------
// FuncInfo: 'Definition' information about a function
// ---------------------------------------------------------------------------
class FuncInfo : public NonUniformDegreeTreeNodeTmpl<FuncInfo>, 
		 public AnnotationBase
{
public:
  typedef Var*                                             MentionT;

  typedef std::list<MentionT>                              MentionSetT;
  typedef MentionSetT::iterator                            mention_iterator;
  typedef MentionSetT::const_iterator                      const_mention_iterator;

  typedef SEXP                                             CallSiteT;
  typedef std::list<CallSiteT>                             CallSiteSetT;
  typedef CallSiteSetT::iterator                           call_site_iterator;
  typedef CallSiteSetT::const_iterator                     const_call_site_iterator;
public:
  FuncInfo(FuncInfo* parent, SEXP name, SEXP defn);
  virtual ~FuncInfo();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual AnnotationBase* clone() { return 0; } // don't support this since it is linked! 

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // member data manipulation
  // -------------------------------------------------------
  
  // arguments
  unsigned int get_num_args() const;
  void set_num_args(unsigned int x);
  SEXP get_args(); 
  bool is_arg(SEXP sym);
  bool are_all_value();
  bool is_arg_value(SEXP arg);
  SEXP get_arg(int position);
  int find_arg_position(char* name);

  SEXP get_defn();

  /// first R name assigned
  SEXP get_first_name();

  // has variable arguments
  bool get_has_var_args() const;
  void set_has_var_args(bool x);

  /// name of C function
  const std::string& get_c_name();

  /// name of C variable storing the closure (CLOSXP)
  const std::string& get_closure();

  // context
  void set_requires_context(bool requires_context); 
  bool requires_context();

  // add information about mentions and call sites
  void insert_mention(MentionT v);
  void insert_call_site(CallSiteT e);

  // CFG
  OA::OA_ptr<OA::CFG::Interface> get_cfg() const;
  void set_cfg(OA::OA_ptr<OA::CFG::Interface> x);
  
  // mention iterators
  mention_iterator begin_mentions();
  const_mention_iterator begin_mentions() const;
  mention_iterator end_mentions();
  const_mention_iterator end_mentions() const;
  
  // call site iterators
  call_site_iterator begin_call_sites();
  const_call_site_iterator begin_call_sites() const;
  call_site_iterator end_call_sites();
  const_call_site_iterator end_call_sites() const;

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  unsigned int m_num_args;    // number of known arguments
  bool m_has_var_args;        // variable number of arguments
  std::string m_c_name;       // C linkage name
  std::string m_closure;      // C closure (CLOSXP) name
  bool m_requires_context;    // is an R context object needed for the function?

  OA::OA_ptr<OA::CFG::Interface> m_cfg; // control flow graph

  MentionSetT m_mentions; // uses and defs inside function (NOT including nested functions)
  CallSiteSetT m_call_sites; // call sites inside function (NOT including nested functions)

  SEXP m_defn;         // function definition
  SEXP m_first_name;   // name of function at original definition 
  FuncInfo *m_parent;  // parent scope definition

  // argument description: types, strict?
};

class FuncInfoChildIterator: public NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo> {
public:
  FuncInfoChildIterator(const FuncInfo *fi, bool firstToLast = true) :
	NonUniformDegreeTreeNodeChildIteratorTmpl<FuncInfo>(fi, firstToLast) {};
};

class FuncInfoIterator: public NonUniformDegreeTreeIteratorTmpl<FuncInfo> {
public:
  FuncInfoIterator(const FuncInfo* fi, TraversalOrder torder = PreOrder,
				   NonUniformDegreeTreeEnumType how =
				     NON_UNIFORM_DEGREE_TREE_ENUM_ALL_NODES) :
	NonUniformDegreeTreeIteratorTmpl<FuncInfo>(fi, torder, how) {};
};

}

#endif
