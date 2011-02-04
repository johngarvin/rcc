
 // DFAGenReachingDefinitionsIRInterface.hpp

#ifndef DFAGenReachingDefinitionsIRInterface_h
#define DFAGenReachingDefinitionsIRInterface_h

#include <iostream>
#include <list>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

namespace OA {
  namespace DFAGenReachingDefinitions {

class DFAGenReachingDefinitionsIRInterface : public virtual IRHandlesIRInterface {
 public:
  DFAGenReachingDefinitionsIRInterface() { }
  virtual ~DFAGenReachingDefinitionsIRInterface() { }
 
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
  
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;

  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
   
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

};  

  } // end of namespace DFAGenReachingDefinitions
} // end of namespace OA

#endif 