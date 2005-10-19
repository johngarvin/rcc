// -*- Mode: C++ -*-

#ifndef LOCALITY_DF_SOLVER_H
#define LOCALITY_DF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

enum LocalityType;
class OA::CFG::Interface;
class RAnnot::AnnotationSet;
class R_VarRef;
class R_VarRefSet;
class R_IRInterface;

namespace Locality {

class DFSet;

//! Implements the OpenAnalysis CFG data flow problem interface to
//! determine whether variable references refer to local or free
//! variables.
class LocalityDFSolver : private OA::DataFlow::CFGDFProblem {
public:
  LocalityDFSolver(OA::OA_ptr<R_IRInterface> _rir);
  ~LocalityDFSolver() {}
  void perform_analysis(OA::ProcHandle proc, OA::OA_ptr<OA::CFG::Interface> cfg);
  void dump_node_maps();
  void dump_node_maps(ostream &os);
  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
private:
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeTop();
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeBottom();

  void initializeNode(OA::OA_ptr<OA::CFG::Interface::Node> n);

  //! CFGDFProblem says: OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  meet (OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, OA::OA_ptr<OA::DataFlow::DataFlowSet> set2);

  //! CFGDFProblem says: OK to modify in set and return it again as result because
  //! solver clones the BB in sets
  OA::OA_ptr<OA::DataFlow::DataFlowSet> 
  transfer(OA::OA_ptr<OA::DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 

private:
  OA::OA_ptr<R_IRInterface> rir;

  void initialize_sets();

  OA::OA_ptr<DFSet> m_all_top;
  OA::OA_ptr<DFSet> m_all_bottom;
  OA::OA_ptr<DFSet> m_entry_values;
  OA::OA_ptr<OA::CFG::Interface> m_cfg;
  OA::ProcHandle m_proc;

};

}

#endif  // LOCALITY_DF_SOLVER_H
