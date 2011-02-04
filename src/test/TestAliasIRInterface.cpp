#include "TestAliasIRInterface.hpp"

OA::OA_ptr<OA::IRProcIterator> 
TestAliasIRInterface::getProcIterator()
{
    OA::OA_ptr<TestIRProcSetIterator> retval;
    retval = new TestIRProcSetIterator( mSubIR->getProcSet() );
    return retval;
}

OA::OA_ptr<OA::IRStmtIterator> 
TestAliasIRInterface::getStmtIterator(OA::ProcHandle h) 
{ 
    OA::OA_ptr<TestIRStmtListIterator> retval;
    OA::OA_ptr<std::list<OA::StmtHandle> > listptr;
    listptr = new std::list<OA::StmtHandle>(mSubIR->getStmtList(h)) ; 
    retval = new TestIRStmtListIterator( listptr );
    return retval;
}

//! Return an iterator over all the memory reference handles that appear
//! in the given statement.
OA::OA_ptr<OA::MemRefHandleIterator> 
TestAliasIRInterface::getAllMemRefs(OA::StmtHandle stmt)
{ 
    OA::OA_ptr<TestIRMemRefSetIterator> retval;
    OA::OA_ptr<std::set<OA::MemRefHandle> > setptr;
    setptr = new std::set<OA::MemRefHandle>(mSubIR->getAllMemRefs(stmt)); 
    retval = new TestIRMemRefSetIterator( setptr );
    return retval;
}
  
//! Given a statement, return its Alias::IRStmtType
OA::Alias::IRStmtType 
TestAliasIRInterface::getAliasStmtType(OA::StmtHandle stmt)
{ 
    if (mSubIR->getPtrAssignPairs(stmt).empty()) { return OA::Alias::ANY_STMT; }
    else { return OA::Alias::PTR_ASSIGN_STMT; }
}

//! return an iterator over MemRefHandle
//! pairs where there is a source and target such that target = source
//! and both reference pointers
OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator> 
TestAliasIRInterface::getPtrAssignStmtPairIterator(OA::StmtHandle stmt)
{ 
    OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator> retval;
    OA::OA_ptr<std::set<PtrPair> > setptr;
    setptr = new std::set<PtrPair>( mSubIR->getPtrAssignPairs(stmt) );
    retval = new TestIRPtrAssignPairStmtIterator(setptr);
    return retval;
}

//! Given a procedure return associated SymHandle
OA::SymHandle TestAliasIRInterface::getSymHandle(OA::ProcHandle h)
{ return mSubIR->getProcSym(h); }
  
//! For the given symbol create a Location that indicates statically
//! overlapping locations and information about whether the location
//! is local or not
OA::OA_ptr<OA::Location::Location> 
TestAliasIRInterface::getLocation(OA::ProcHandle proc, OA::SymHandle s)
{ 
    OA::OA_ptr<OA::Location> loc = mSubIR->getLocation(proc,s); 
/*    if (loc.ptrEqual(0)) {
        std::cerr << "There is no location for symbol " << toString(s)
                  << " in procedure " << toString(proc) << std::endl;
        exit(0);
    }*/
    return loc;
}

//! Given a MemRefHandle return a list of all 
//! MemRefExprs that describe this memory reference
OA::OA_ptr<OA::MemRefExprIterator>
TestAliasIRInterface::getMemRefExprIterator(OA::MemRefHandle h)
{ 
    OA::OA_ptr<TestIRMemRefExprSetIterator> retval;
    OA::OA_ptr<std::set<OA::OA_ptr<OA::MemRefExpr> > > setptr;
    setptr = new std::set<OA::OA_ptr<OA::MemRefExpr> >(
                mSubIR->getMemRefExprs(h));
    retval = new TestIRMemRefExprSetIterator( setptr );

    return retval;
}

void TestAliasIRInterface::dump(std::ostream& os)
{
}

SymHandle TestAliasIRInterface::getFormalSym(ProcHandle proc, int num)
{
    return mSubIR->getSymHandle(proc, num);
}

OA_ptr<MemRefExpr> TestAliasIRInterface::getCallMemRefExpr(CallHandle h)
{
    return mSubIR->getMemRefExpr(h);
}

OA_ptr<IRCallsiteIterator> TestAliasIRInterface::getCallsites(StmtHandle h)
{
    OA::OA_ptr<TestIRCallSiteIterator> retval;
    OA::OA_ptr<std::set<OA::CallHandle> > setptr;
    setptr = new std::set<OA::CallHandle>( mSubIR->getAllCallHandles(h) );
    retval = new TestIRCallSiteIterator( setptr );
    return retval;
}

OA_ptr<ParamBindPtrAssignIterator>
    TestAliasIRInterface::getParamBindPtrAssignIterator(CallHandle call)
{
    OA::OA_ptr<TestIRParamBindPtrAssignIterator> retval;
    OA::OA_ptr<std::map<int, OA::OA_ptr<OA::MemRefExpr> > > mapptr;
    mapptr = new std::map<int, OA::OA_ptr<OA::MemRefExpr> >(
         mSubIR->getAllParamBinds(call) );

    retval = new TestIRParamBindPtrAssignIterator( mapptr );
    return retval;
}

ProcHandle TestAliasIRInterface::getProcHandle(SymHandle sym)
{
    return mSubIR->getSymProc(sym);
}
