#ifndef ANNOTATION_DEF_VAR
#define ANNOTATION_DEF_VAR

#include <include/R/R_RInternals.h>

#include <analysis/Var.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// DefVar: A variable reference that is a def
// ---------------------------------------------------------------------------
class DefVar
  : public Var
{
public:
  enum SourceT {
    DefVar_ASSIGN,  // defined by an assignment statement
    DefVar_FORMAL,  // defined as a formal variable by a function
    DefVar_PHI      // defined by a phi function
  };
public:
  DefVar();
  virtual ~DefVar();

  // source of definition: assignment statement, formal argument, or phi function
  SourceT getSourceType() const
    { return mSourceType; }
  void setSourceType(SourceT x)
    { mSourceType = x; }

  virtual SEXP getName() const;

  SEXP getRhs_c() const
    { return mRhs_c; }
  void setRhs_c(SEXP x)
    { mRhs_c = x; }

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual DefVar* clone() { return new DefVar(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  SourceT mSourceType;
  SEXP mRhs_c;               // cell containing right side, if assignment
  // bool mIsBindingKnown;
  // set of uses reached?
};

const std::string typeName(const DefVar::SourceT x);

}

#endif
