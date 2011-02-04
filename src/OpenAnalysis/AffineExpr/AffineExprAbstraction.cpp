#include "AffineExprAbstraction.hpp"

#include <iostream>
using namespace std;

namespace OA {
namespace AffineExpr {

AffineExprAbstraction::AffineExprAbstraction() { }

void AffineExprAbstraction::addTerm(OA_ptr<NamedLoc> var, int value) {
   mCoefficients.insert(make_pair(var, value));
}

} } // end namespaces
