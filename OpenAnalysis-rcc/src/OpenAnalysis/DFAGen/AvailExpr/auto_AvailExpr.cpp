/* AvailExpr.cpp
   NOTE: This file was automatically generated by DFAGen.  It is the
         implementation of the class for AvailExpr analysis results.
*/


#include "auto_AvailExpr.hpp"

namespace OA {
  namespace AvailExpr {


OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > AvailExpr::getAvailExprIterator(StmtHandle s) const {
    OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > retval;

    if(mAvailExpr.find(s) == mAvailExpr.end()) {
        retval = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > ();
    } else {
        retval = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > (
            mAvailExpr.find(s)->second);
    }

    return retval;
}


OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > AvailExpr::getExitAvailExprIterator() const {
    OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > retval;

    retval = new DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > (mExitAvailExpr);
    return retval;
}


void AvailExpr::output(IRHandlesIRInterface& ir) const {
    sOutBuild->objStart("AvailExpr");

    sOutBuild->mapStart("mAvailExpr", "StmtHandle", "OA_ptr<AvailExprDFSet>");

    std::map<StmtHandle,OA_ptr<AvailExprDFSet> >::const_iterator mapIter;
    for (mapIter = mAvailExpr.begin(); mapIter != mAvailExpr.end(); mapIter++) {
        StmtHandle s = mapIter->first;
        OA_ptr<AvailExprDFSet> rdset = mapIter->second;
        if ( rdset.ptrEqual(0) ) continue;

        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(s, ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();


        sOutBuild->listStart();
        OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > setIter;
        setIter = getAvailExprIterator(s);
        for ( ; setIter->isValid(); (*setIter)++ ) {
             sOutBuild->listItemStart();
             setIter->current()->output(ir);
             sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mAvailExpr");
    sOutBuild->objEnd("AvailExpr");
}


void AvailExpr::dump(
    std::ostream& os,
    OA_ptr<IRHandlesIRInterface> ir,
    Alias::Interface& aliasResults) const
{
    os << "AvailExpr Analysis results." << endl;
    std::map<StmtHandle, OA_ptr<AvailExprDFSet> >::const_iterator mapIter;

    for (mapIter = mAvailExpr.begin(); mapIter != mAvailExpr.end(); mapIter++) {
        os << "\tstmt = " << ir->toString(mapIter->first) << std::endl;
        OA_ptr<DataFlow::DataFlowSetImplIterator<OA_ptr<ExprTree> > > iter;
        iter = getAvailExprIterator(mapIter->first);

        for ( ; iter->isValid(); (*iter)++) {
            os << "\t\t";
        iter->current()->dump(cout, mIR);
            os << std::endl;
        }
    }
}


  } // end of AvailExpr namespace
} // end of OA namespace
