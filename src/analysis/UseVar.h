#ifndef ANNOTATION_USE_VAR_H
#define ANNOTATION_USE_VAR_H

#include <include/R/R_RInternals.h>

#include <analysis/Var.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// UseVar: A variable reference that is a use
// ---------------------------------------------------------------------------
class UseVar
  : public Var
{
public:
  enum PositionT {
    UseVar_FUNCTION,
    UseVar_ARGUMENT
  };

public:
  UseVar();
  virtual ~UseVar();

  // function or argument position
  PositionT getPositionType() const
    { return mPositionType; }
  void setPositionType(PositionT x)
    { mPositionType = x; }

  virtual SEXP getName() const;

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual UseVar* clone() { return new UseVar(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;

private:
  PositionT mPositionType;
};

const std::string typeName(const UseVar::PositionT x);

}

#endif
