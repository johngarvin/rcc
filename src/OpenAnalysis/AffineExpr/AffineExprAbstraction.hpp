#ifndef OA_AffineExpr_H
#define OA_AffineExpr_H

#include <map>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/Location/NamedLoc.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>
using namespace std;

namespace OA {
namespace AffineExpr {

/*! An expression is an affine function of variables, v1, v2, ..., vN if it can
    be expressed as the sum of a known constant times each of the variables plus
    a constant. That is,

    c0 + c1*v1 + c2*v2 + ... + cN*vN, 
       for known integers, c0..cN.

    Definition from:
        http://www-cse.ucsd.edu/classes/sp02/cse231/scaledocs/scale/score/dependence/AffineExpr.html
*/
class AffineExprAbstraction : public virtual Annotation {
  public:
    AffineExprAbstraction();
    void setOffset(int offset) { mOffset = offset; }
    void addTerm(OA_ptr<NamedLoc> var, int value);

  // - [output methods] - (derived from Annotation)
    virtual void output(IRHandlesIRInterface &ir);

  private:
  OUTPUT
    GENOUT  map<OA_ptr<NamedLoc>, int> mCoefficients;
    GENOUT  int mOffset;
};

} } // end namespaces

#endif

