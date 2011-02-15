/*! \file
    
  \brief Subsidiary IR for storing mappings between IRHandles and OA abstractions
        
  \authors Michelle Strout, Andy Stone
  \version $Id: SubsidiaryIR.hpp,v 1.1.2.2 2006/01/05 22:52:25 mstrout Exp $
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef SubsidiaryIR_HPP
#define SubsidiaryIR_HPP

#include <string>
#include <list>
#include <map>
#include <iostream>
using namespace std;
#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/IRInterface/IRHandles.hpp>
#include <OpenAnalysis/Location/Location.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/OABase/Annotation.hpp>
#include <OpenAnalysis/Utils/GenOutputTool.hpp>

/*
   target = source;
 */
class PtrPair 
{
  public:
    PtrPair(OA::OA_ptr<OA::MemRefExpr> target, OA::OA_ptr<OA::MemRefExpr> source) 
        : mTarget(target), mSource(source) {}
    ~PtrPair() {}

    bool operator< (const PtrPair &other ) const
    {
        if (*mTarget < *other.mTarget
            || (*mTarget == *other.mTarget && *mSource < *other.mSource) )
        {
            return true;
        } else {
            return false;
        }
    }

    void output(OA::OA_ptr<OA::IRHandlesIRInterface> ir)
    {
        // output this datastructure
    }

    OA::OA_ptr<OA::MemRefExpr> getTarget() { return mTarget; }
    OA::OA_ptr<OA::MemRefExpr> getSource() { return mSource; }
  private:
    OA::OA_ptr<OA::MemRefExpr> mTarget, mSource;
};

/*! 
    The subsidiary IR holds mappings of IRHandles to each other and to
    abstractions within OA.  For example, a mapping of ProcHandles to 
    StmtHandles, StmtHandles to MemRefHandles, and MemRefHandles to
    MemRefExprs.
*/
class SubsidiaryIR  : public OA::Annotation
{ 
  public:
    SubsidiaryIR() {}
    ~SubsidiaryIR() {}

    //*****************************************************************
    // Implementation of IRHandlesIRInterface
    // which is the base class for all the other abstract IRInterfaces
    //*****************************************************************
    std::string toString(OA::ProcHandle h); 
    std::string toString(OA::StmtHandle h); 
    std::string toString(OA::MemRefHandle h); 
    std::string toString(OA::SymHandle h); 
    std::string toString(OA::ExprHandle h); 
    std::string toString(OA::ConstSymHandle h) ;
    std::string toString(OA::ConstValHandle h) ;
    std::string toString(OA::OpHandle h) ;
    std::string toString(OA::CallHandle h) ;
    void dump(OA::MemRefHandle h, std::ostream& os);
    void dump(OA::StmtHandle h, std::ostream& os);

    //*****************************************************************
    // Implementation of Annotation interface
    //*****************************************************************
    //! prints info using persistent handles
    void output(OA::OA_ptr<OA::IRHandlesIRInterface> ir);

    //*****************************************************************
    // Query methods 
    //*****************************************************************
    OA::SymHandle getProcSym(OA::ProcHandle);
    OA::ProcHandle getSymProc(OA::SymHandle);

    // return the set of procedures
    OA::OA_ptr<std::set<OA::ProcHandle> > getProcSet();

    // list of statements in a proc
    std::list<OA::StmtHandle>& getStmtList(OA::ProcHandle);

    // set of mem refs for stmt
    std::set<OA::MemRefHandle>& getAllMemRefs(OA::StmtHandle);

    // set of CallHandles for a stmt
    std::set<OA::CallHandle>& getAllCallHandles(OA::StmtHandle);

    std::map<int, OA::OA_ptr<OA::MemRefExpr> >&
        getAllParamBinds(OA::CallHandle);

    std::set<PtrPair>& getPtrAssignPairs(OA::StmtHandle);

    OA::OA_ptr<OA::MemRefExpr>& getMemRefExpr(OA::CallHandle,int);

    std::set<OA::OA_ptr<OA::MemRefExpr> >& getMemRefExprs(OA::MemRefHandle);

    OA::OA_ptr<OA::MemRefExpr>& getMemRefExpr(OA::CallHandle);

    OA::OA_ptr<OA::Location> getLocation(OA::ProcHandle,OA::SymHandle);

    OA::SymHandle& getSymHandle(OA::ProcHandle,int);

    //*****************************************************************
    // Construction methods 
    //*****************************************************************
    void addProcString(OA::ProcHandle proc,std::string str);

    void addStmtString(OA::StmtHandle stmt, std::string str);

    void addMemRefString(OA::MemRefHandle memref,std::string str);

    void addSymString(OA::SymHandle sym,std::string str);

    void addExprString(OA::ExprHandle expr,std::string str);

    void addOpString(OA::OpHandle op,std::string str);

    void addConstSymString(OA::ConstSymHandle op,std::string str);

    void addConstValString(OA::ConstValHandle op,std::string str);

    void addCallString(OA::CallHandle op, std::string str);

    // all new handles belong in the given procedure until another one is
    // given
    void startProc(OA::ProcHandle, OA::SymHandle);

    void addLocation(OA::OA_ptr<OA::Location>);
    void addStmt(OA::StmtHandle);
    void addMemRefExpr(OA::StmtHandle, OA::MemRefHandle,
                       OA::OA_ptr<OA::MemRefExpr::MemRefExpr>);
    void addCallSite(OA::StmtHandle, OA::CallHandle, 
                     OA::OA_ptr<OA::MemRefExpr::MemRefExpr>);
    void addPtrAssignPair(OA::StmtHandle, OA::OA_ptr<OA::MemRefExpr> target, 
                          OA::OA_ptr<OA::MemRefExpr> source);
    void addParamBindPtrAssignPair(OA::CallHandle, int param,
                                   OA::OA_ptr<OA::MemRefExpr> target);
    void addFormal(int param, OA::SymHandle sym);

    void dump(std::ostream& os);

  private:
    OUTPUT

    OA::ProcHandle mCurrentProc;
    GENOUT std::map<OA::ProcHandle,std::string> mProcString;
    GENOUT std::map<OA::StmtHandle,std::string > mStmtString;
    GENOUT std::map<OA::MemRefHandle,std::string > mMemRefString;
    GENOUT std::map<OA::SymHandle,std::string > mSymString;
    GENOUT std::map<OA::ExprHandle,std::string > mExprString;
    GENOUT std::map<OA::OpHandle,std::string > mOpString;
    GENOUT std::map<OA::CallHandle,std::string > mCallString;
    GENOUT std::map<OA::ConstValHandle,std::string > mConstValString;
    GENOUT std::map<OA::ConstSymHandle,std::string > mConstSymString;

    // query with getProcSym
    GENOUT std::map<OA::ProcHandle,OA::SymHandle> mProcSym;

    // query with getStmtList
    GENOUT std::map<OA::ProcHandle,std::list<OA::StmtHandle> > mProcBody;

    // query with getAllMemRefs
    GENOUT std::map<OA::StmtHandle,std::set<OA::MemRefHandle> > mStmtMemRefAll;

    // query with getAllCallHandles
    GENOUT std::map<OA::StmtHandle,std::set<OA::CallHandle> > mStmtCallAll;

    // query with getPtrAssignPairs
    GENOUT std::map<OA::StmtHandle,std::set<PtrPair> > mStmtPtrPairs;

    // query with getMemRefExpr
    GENOUT std::map<OA::CallHandle, std::map<int, OA::OA_ptr<OA::MemRefExpr> > >
        mParamBindPtrPairs;

    // query with getMemRefExprs
    GENOUT std::map<OA::MemRefHandle,
                 std::set<OA::OA_ptr<OA::MemRefExpr> > > mMemRefToMRE;

    // query with getMemRefExprs
    GENOUT std::map<OA::CallHandle, OA::OA_ptr<OA::MemRefExpr> > mCallToMRE;
    
    // query with getLocation
    GENOUT std::map<OA::ProcHandle,
                 std::map<OA::SymHandle, 
                          OA::OA_ptr<OA::Location> > > mSymToLoc;
    
    // query with getSymHandle
    GENOUT std::map<OA::ProcHandle, std::map<int, OA::SymHandle> > mFormals;
};

#endif
