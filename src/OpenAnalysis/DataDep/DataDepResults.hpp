#ifndef DataDepResults_H
#define DataDepResults_H

#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

namespace OA {
namespace DataDep {

class DataDepResults : public virtual Annotation {
  public:
    // TODO: Implement results class

    // - [output methods] - (derived from Annotation)
    virtual void output(IRHandlesIRInterface &ir);

  private:
  OUTPUT
    GENOUT int temp;
};

} } // end namespaces

#endif
