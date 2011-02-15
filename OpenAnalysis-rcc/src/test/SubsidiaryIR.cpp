#include "SubsidiaryIR.hpp"

//*****************************************************************
// Implementation of IRHandlesIRInterface
//*****************************************************************
std::string SubsidiaryIR::toString(OA::ProcHandle h) 
{ return mProcString[h]; }

std::string SubsidiaryIR::toString(OA::StmtHandle h) 
{ return mStmtString[h]; }

std::string SubsidiaryIR::toString(OA::MemRefHandle h) 
{ return mMemRefString[h]; }

std::string SubsidiaryIR::toString(OA::SymHandle h) 
{ return mSymString[h]; }

std::string SubsidiaryIR::toString(OA::ExprHandle h) 
{ return mExprString[h]; }

std::string SubsidiaryIR::toString(OA::ConstSymHandle h) 
{ return mConstSymString[h]; }

std::string SubsidiaryIR::toString(OA::ConstValHandle h) 
{ return mConstValString[h]; }

std::string SubsidiaryIR::toString(OA::OpHandle h) 
{ return mOpString[h]; }

std::string SubsidiaryIR::toString(OA::CallHandle h)
{ return mCallString[h]; }

void SubsidiaryIR::dump(OA::MemRefHandle h, std::ostream& os)
{ os << "MemRefHandle(hval=" << h.hval() << ", " << toString(h) 
     << ")" << std::endl;
}

void SubsidiaryIR::dump(OA::StmtHandle h, std::ostream& os)
{ os << "StmtHandle(hval=" << h.hval() << ", " << toString(h) 
     << ")" << std::endl;
}
    
//*****************************************************************
// Query methods 
//*****************************************************************
// return the set of procedures
OA::OA_ptr<std::set<OA::ProcHandle> > SubsidiaryIR::getProcSet()
{
    std::map<OA::ProcHandle,std::string>::iterator procIter;
    OA::OA_ptr<std::set<OA::ProcHandle> > procSet;
    procSet = new std::set<OA::ProcHandle>;
    for (procIter=mProcString.begin();
         procIter!=mProcString.end(); procIter++ )
    {
        procSet->insert(procIter->first);
    }
    return procSet;
}
    
OA::ProcHandle SubsidiaryIR::getSymProc(OA::SymHandle sym)
{
    std::map<OA::ProcHandle,OA::SymHandle>::iterator mapIter;
    for (mapIter=mProcSym.begin(); mapIter!=mProcSym.end(); mapIter++ ) {
        if (mapIter->second == sym) {
            return mapIter->first;
        }
    }

    return OA::ProcHandle(0);
}

OA::SymHandle SubsidiaryIR::getProcSym(OA::ProcHandle proc)
{
    return mProcSym[proc];
}

std::list<OA::StmtHandle>& SubsidiaryIR::getStmtList(OA::ProcHandle proc)
{
    return mProcBody[proc];
}

std::set<OA::MemRefHandle>& SubsidiaryIR::getAllMemRefs(OA::StmtHandle stmt)
{
    return mStmtMemRefAll[stmt];
}

std::set<OA::CallHandle>&
SubsidiaryIR::getAllCallHandles(OA::StmtHandle stmt)
{
    return mStmtCallAll[stmt];
}

std::map<int, OA::OA_ptr<OA::MemRefExpr> >&
    SubsidiaryIR::getAllParamBinds(OA::CallHandle call)
{
    return mParamBindPtrPairs[call];
}

std::set<PtrPair>& SubsidiaryIR::getPtrAssignPairs(OA::StmtHandle stmt)
{
    return mStmtPtrPairs[stmt];
}

OA::OA_ptr<OA::MemRefExpr>&
SubsidiaryIR::getMemRefExpr(OA::CallHandle call, int formalIdx)
{
     return mParamBindPtrPairs[call][formalIdx];
}

OA::OA_ptr<OA::MemRefExpr>&
SubsidiaryIR::getMemRefExpr(OA::CallHandle h)
{
    return mCallToMRE[h];
}

std::set<OA::OA_ptr<OA::MemRefExpr> >& 
SubsidiaryIR::getMemRefExprs(OA::MemRefHandle ref)
{
    return mMemRefToMRE[ref];
}

OA::OA_ptr<OA::Location> SubsidiaryIR::getLocation(OA::ProcHandle proc,
                                           OA::SymHandle sym)
{
    return mSymToLoc[proc][sym];
}

//getSymHandle
OA::SymHandle& SubsidiaryIR::getSymHandle(OA::ProcHandle proc, int formalIdx)
{
    return mFormals[proc][formalIdx];
}

//*****************************************************************
// Construction methods 
//*****************************************************************
void SubsidiaryIR::addProcString(OA::ProcHandle proc,std::string str)
{
    mProcString[proc] = str;
}


void SubsidiaryIR::addStmtString(OA::StmtHandle stmt, std::string str)
{
    mStmtString[stmt] = str;
}

void SubsidiaryIR::addMemRefString(OA::MemRefHandle memref,std::string str)
{
    mMemRefString[memref] = str;
}

void SubsidiaryIR::addSymString(OA::SymHandle sym,std::string str)
{
    mSymString[sym] = str;
}

void SubsidiaryIR::addExprString(OA::ExprHandle expr,std::string str)
{
    mExprString[expr] = str;
}

void SubsidiaryIR::addOpString(OA::OpHandle h,std::string str)
{
    mOpString[h] = str;
}

void SubsidiaryIR::addConstValString(OA::ConstValHandle h, std::string str)
{
    mConstValString[h] = str;
}

void SubsidiaryIR::addCallString(OA::CallHandle h, std::string str)
{
    mCallString[h] = str;
}

void SubsidiaryIR::addConstSymString(OA::ConstSymHandle h, std::string str)
{
    mConstSymString[h] = str;
}

void SubsidiaryIR::startProc( OA::ProcHandle proc, OA::SymHandle sym )
{
    mCurrentProc = proc;
    mProcSym[proc] = sym;
    mSymToLoc[proc];
}

void SubsidiaryIR::addLocation(OA::OA_ptr<OA::Location> loc)
{
    if (loc->isaNamed()) {
        OA::OA_ptr<OA::NamedLoc> namedLoc = loc.convert<OA::NamedLoc>();
        mSymToLoc[mCurrentProc][namedLoc->getSymHandle()] = loc;
    }
}

void SubsidiaryIR::addStmt( OA::StmtHandle stmt )
{
    // if we haven't already added this stmt to the list
    if (mProcBody[mCurrentProc].back() != stmt) {
        mProcBody[mCurrentProc].push_back(stmt);
    }
}

void SubsidiaryIR::addMemRefExpr(OA::StmtHandle stmt,
    OA::MemRefHandle memref, 
    OA::OA_ptr<OA::MemRefExpr::MemRefExpr> mre)
{
    mStmtMemRefAll[stmt].insert(memref);
    mMemRefToMRE[memref].insert(mre);
}

void SubsidiaryIR::addCallSite(OA::StmtHandle stmt, OA::CallHandle call, 
                               OA::OA_ptr<OA::MemRefExpr::MemRefExpr> mre)
{
    mStmtCallAll[stmt].insert(call);
    mCallToMRE[call] = mre;
}

void SubsidiaryIR::addPtrAssignPair(
    OA::StmtHandle stmt,
    OA::OA_ptr<OA::MemRefExpr> target, 
    OA::OA_ptr<OA::MemRefExpr> source)
{
    mStmtPtrPairs[stmt].insert( PtrPair(target,source) );
}

void SubsidiaryIR::addParamBindPtrAssignPair(OA::CallHandle call, int param,
    OA::OA_ptr<OA::MemRefExpr> target)
{
    mParamBindPtrPairs[call][param] = target;
}

void SubsidiaryIR::addFormal(int param, OA::SymHandle sym)
{
    mFormals[mCurrentProc][param] = sym;
}

void SubsidiaryIR::dump(std::ostream& os)
{
    // loop through procedures
    std::map<OA::ProcHandle,std::string>::iterator procMapIter;
    for (procMapIter=mProcString.begin(); procMapIter!= mProcString.end();
         procMapIter++) 
    {
        OA::ProcHandle proc = procMapIter->first;
        os << "ProcHandle ( " << proc.hval();
        os << ", " << procMapIter->second << " ) => [" << std::endl;

        // loop through Locations
        std::map<OA::SymHandle,OA::OA_ptr<OA::Location> >::iterator symLocIter;
        for (symLocIter=mSymToLoc[proc].begin();
             symLocIter!=mSymToLoc[proc].end(); symLocIter++ )
        {
            OA::SymHandle sym = symLocIter->first;
            OA::OA_ptr<OA::Location> loc = symLocIter->second;
            os << "\tSymHandle( " << sym.hval();
            os << ", " << mSymString[sym] << " ) => [ ";
            loc->dump(std::cout);
            os << " ]" << std::endl;
        }

        // loop through stmts
        std::list<OA::StmtHandle>::iterator stmtIter;
        for (stmtIter=mProcBody[proc].begin(); stmtIter!=mProcBody[proc].end();
             stmtIter++)
        {
            OA::StmtHandle stmt = *stmtIter;
            os << "\tStmtHandle( " << stmt.hval();
            os << ", " << mStmtString[stmt] << " ) => [" << std::endl;

            // loop through MemRefs
            std::set<OA::MemRefHandle>::iterator memRefIter;
            for (memRefIter=mStmtMemRefAll[stmt].begin();
                 memRefIter!=mStmtMemRefAll[stmt].end(); memRefIter++ ) 
            {
                OA::MemRefHandle memref = *memRefIter;
                os << "\t\tMemRefHandle( " << memref.hval();
                os << ", " << mMemRefString[memref] << " ) => [";

                // loop through MemRefExprs for that MemRef
                std::set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
                for (mreIter=mMemRefToMRE[memref].begin();
                     mreIter!=mMemRefToMRE[memref].end(); mreIter++)
                {
                    OA::OA_ptr<OA::MemRefExpr> mre = *mreIter;
                    mre->dump(std::cout);
                }

                os << "]" << std::endl;
            }
/*
[NOTE:] I had to comment this code out. I'm not sure what it's used for.

            // loop through PtrAssignPairs
            std::cout << "\t\tPtrAssignPairs = [" << std::endl;
            std::set<PtrPair>::iterator pairIter;
            for (pairIter=mStmtPtrPairs[stmt].begin();
                 pairIter!=mStmtPtrPairs[stmt].end(); pairIter++ ) 
            {
                PtrPair pair = *pairIter;
                os << "\t\t\t< MemRefHandle( " << pair.getTarget().hval();
                os << "), MemRefHandle( " << pair.getSource().hval() << ") >"
                   << std::endl;
           }
            std::cout << "\t\t]" << std::endl;
*/

            os << "\t]" << std::endl;
        }

        os << "]" << std::endl;
    }

}


