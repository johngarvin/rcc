// LivenessIRInterface.hpp

#ifndef LivenessIRInterface_h
#define LivenessIRInterface_h

#include <iostream>
#include <list>
#include <string>
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

#include <OpenAnalysis/IRInterface/auto_LivenessIRInterface.hpp>

namespace OA {
  namespace Liveness {
/*
class LivenessIRInterface : public virtual IRHandlesIRInterface {
 public:
  LivenessIRInterface() { }
  virtual ~LivenessIRInterface() { }
 
  virtual OA_ptr<IRStmtIterator> getStmtIterator(ProcHandle h) = 0; 

  virtual OA_ptr<MemRefHandleIterator> getAllMemRefs(StmtHandle stmt) = 0;
  
  virtual OA_ptr<MemRefHandleIterator> getDefMemRefs(StmtHandle stmt) = 0;

  virtual OA_ptr<MemRefHandleIterator> getUseMemRefs(StmtHandle stmt) = 0;
   
  virtual OA_ptr<IRCallsiteIterator> getCallsites(StmtHandle h) = 0;

};  
*/
  } // end of namespace Liveness
} // end of namespace OA

#endif 
