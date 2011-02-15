#ifndef ManagerDataDepGCD_H
#define ManagerDataDepGCD_H

#include "ManagerDataDep.hpp"

#include <OpenAnalysis/AffineExpr/ManagerAffineExpr.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>

namespace OA {
namespace DataDep { 

class ManagerDataDepGCD : public ManagerDataDep {
  public:
    ManagerDataDepGCD(
        OA_ptr<DataDepIRInterface> ir,
        OA_ptr<Alias::Interface> aliasResults);

    virtual OA_ptr<DataDepResults> performAnalysis(
        ProcHandle p, OA_ptr<LoopAbstraction> loop);

  private:
    OA_ptr<Alias::Interface> mAliasResults;
};

} } // end namespaces

#endif
