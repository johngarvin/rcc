#ifndef ANNOTATION_FORMAL_ARG_INFO_H
#define ANNOTATION_FORMAL_ARG_INFO_H

#include <analysis/AnnotationBase.h>
#include <analysis/PropertyHndl.h>

namespace RAnnot {

// ---------------------------------------------------------------------------
// FormalArgInfo
// ---------------------------------------------------------------------------
class FormalArgInfo
  : public AnnotationBase
{
public:
  FormalArgInfo();
  virtual ~FormalArgInfo();

  static PropertyHndlT handle();

  // -------------------------------------------------------
  // cloning: return a shallow copy... 
  // -------------------------------------------------------
  virtual FormalArgInfo* clone() { return new FormalArgInfo(*this); }

  // -------------------------------------------------------
  // debugging
  // -------------------------------------------------------
  virtual std::ostream& dump(std::ostream& os) const;
  
  bool isValue() { return isvalue; }
  void setIsValue(bool _isvalue) { isvalue = _isvalue; }

private:
  bool isvalue; // value/promise
};


}

#endif
