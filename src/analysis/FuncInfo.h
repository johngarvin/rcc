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
  FuncInfo(FuncInfo *lexParent, SEXP name, SEXP defn);
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
  unsigned int getNumArgs() const 
     { return mNumArgs; }
  void setNumArgs(unsigned int x) 
     { mNumArgs = x; }
  SEXP getArgs(); 
  bool areAllValue();
  bool isArgValue(SEXP arg);
  SEXP getArg(int position);
  int findArgPosition(char* name);

  // definition
  SEXP getDefn() 
    { return mDefn; }

  // first name assigned
  SEXP getFirstName()
    { return mFirstName; }

  // has-variable-arguments
  bool getHasVarArgs() const
    { return mHasVarArgs; }
  void setHasVarArgs(bool x)
    { mHasVarArgs = x; }

  // c-linkage-name
  const std::string& getCName() const
    { return mCName; }
  void setCName(std::string& x)
    { mCName = x; }

  // context
  void setRequiresContext(bool requiresContext); 
  bool getRequiresContext();

  // insert methods into sets
  void insertMention(MentionT v);
  void insertCallSite(CallSiteT e);

  // CFG
  OA::OA_ptr<OA::CFG::Interface> getCFG() const
    { return mCFG; }
  void setCFG(OA::OA_ptr<OA::CFG::Interface> x)
    { mCFG = x; }

  // mention iterators
  mention_iterator beginMentions()
    { return mMentions.begin(); }
  const_mention_iterator beginMentions() const
    { return mMentions.begin(); }
  mention_iterator endMentions()
    { return mMentions.end(); }
  const_mention_iterator endMentions() const
    { return mMentions.end(); }

  // call site iterators
  call_site_iterator beginCallSites()
    { return mCallSites.begin(); }
  const_call_site_iterator beginCallSites() const
    { return mCallSites.begin(); }
  call_site_iterator endCallSites()
    { return mCallSites.end(); }
  const_call_site_iterator endCallSites() const
    { return mCallSites.end(); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  unsigned int mNumArgs;   // number of known arguments
  bool mHasVarArgs;        // variable number of arguments
  std::string mCName;      // C linkage name
  bool mRequiresContext;   // is an R context object needed for the function?

  OA::OA_ptr<OA::CFG::Interface> mCFG; // control flow graph

  MentionSetT mMentions; // uses and defs inside function (NOT including nested functions)
  CallSiteSetT mCallSites; // call sites inside function (NOT including nested functions)

  SEXP mDefn;            // function definition
  SEXP mFirstName;       // name of function at original definition 
  FuncInfo *mLexicalParent; // parent scope definition

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
