/*! \file
  
  \brief The AnnotationManager that generates SideEffectStandard
         which maps each procedure to the sets of tags belonging to
         LMOD, MOD, LUSE, USE, LDEF, DEF, LREF, and REF.  

  \authors Michelle Strout, Andy Stone (alias tag update)
  \version $Id: ManagerSideEffectStandard.cpp,v 1.14 2005/08/08 20:03:52 mstrout Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "ManagerSideEffectStandard.hpp"
#include <Utils/Util.hpp>

namespace OA {
  namespace SideEffect {

using namespace std;
using namespace Alias;

static bool debug = false;

ManagerSideEffectStandard::ManagerSideEffectStandard(
    OA_ptr<SideEffectIRInterface> _ir)
    :
    mIR(_ir)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_ManagerSideEffectStandard:ALL", debug);
}


/*!
    \brief    Used to perform analysis when not using AQM

    Currently just does an intraprocedural side-effect analysis.
    We conservatively assume that function calls define and use the
    UnknownLoc, which overlaps with all possible tags.

*/
OA_ptr<SideEffectStandard> 
ManagerSideEffectStandard::performAnalysis(
    ProcHandle proc, 
    OA_ptr<Alias::Interface> alias,
    OA_ptr<InterSideEffectInterface> inter)
{
    mProc = proc;
    if (debug) {
        cout << "ManagerSideEffectStandard: proc = " << mIR->toString(proc)
             << endl;
    }

    // create a new (waiting to be filled) SideEffectStandard as member
    OA_ptr<SideEffectStandard> retSideEffect;
    retSideEffect = new SideEffectStandard(alias);

    // empty out all the sets
    retSideEffect->emptyLMOD();
    retSideEffect->emptyMOD();
    retSideEffect->emptyLDEF();
    retSideEffect->emptyDEF();
    retSideEffect->emptyLUSE();
    retSideEffect->emptyUSE();
    retSideEffect->emptyLREF();
    retSideEffect->emptyREF();

    // loop through statements
    OA_ptr<IRStmtIterator> sIt = mIR->getStmtIterator(proc);
    for ( ; sIt->isValid(); (*sIt)++) {
        StmtHandle stmt = sIt->current();
        if (debug) {
            cout << "\tstmt = ";
            mIR->dump(stmt, cout);
        }

        // for each use memory reference in the stmt
        OA_ptr<MemRefHandleIterator> mrIterPtr = mIR->getUseMemRefs(stmt);
        for (; mrIterPtr->isValid(); (*mrIterPtr)++ )
        {
            MemRefHandle memref = mrIterPtr->current();
            if (debug) {
                cout << "\tmemref = ";
                mIR->dump(memref, cout);
            }

            OA::OA_ptr<OA::MemRefExprIterator> mreIter
              = mIR->getMemRefExprIterator(memref);
            for ( ; mreIter->isValid(); (*mreIter)++ ) {
                OA::OA_ptr<OA::MemRefExpr::MemRefExpr> mre = mreIter->current();

                // for each tag the memory reference may reference
                OA_ptr<AliasTagSet> tagSet = alias->getAliasTags(mre);
                OA_ptr<AliasTagIterator> tagIter = tagSet->getIterator();
                for (; tagIter->isValid(); ++(*tagIter)) {
                    AliasTag tag = tagIter->current();

                    if (debug) {
                        cout << "useLoc = " << tag << endl;
                    }

                    // put each location used through a memory 
                    // reference in LREF and REF

                    //! All Uses are REFs
                    retSideEffect->insertLREF(tag);
                    retSideEffect->insertREF(tag);

                    retSideEffect->insertLUSE(tag);
                    retSideEffect->insertUSE(tag);
                }
            } 
        }

        // for each def memory reference in the stmt
        mrIterPtr = mIR->getDefMemRefs(stmt);
        for (; mrIterPtr->isValid(); (*mrIterPtr)++ )
        {
            MemRefHandle memref = mrIterPtr->current();
            if (debug) {
                cout << "\tmemref = ";
                mIR->dump(memref, cout);
            }


            OA::OA_ptr<OA::MemRefExprIterator> mreIter
              = mIR->getMemRefExprIterator(memref);
            for ( ; mreIter->isValid(); (*mreIter)++ ) {
                 OA::OA_ptr<OA::MemRefExpr::MemRefExpr> mre = mreIter->current();

                 // put each tag defined into LMOD and MOD
                 OA_ptr<AliasTagSet> tagSet = alias->getAliasTags(mre);
                 OA_ptr<AliasTagIterator> tagIter = tagSet->getIterator();
                 for (; tagIter->isValid(); ++(*tagIter)) {
                     AliasTag tag = tagIter->current();
                     if (debug) {
                         cout << "mayDef = " << tag;
                     }

                     retSideEffect->insertLMOD(tag);
                     retSideEffect->insertMOD(tag);


                     //! All Defs are REFs
                     retSideEffect->insertLREF(tag);
                     retSideEffect->insertREF(tag);


                     // if the set is must defined than also place it into
                     // LDEF and DEF
                     if(tagSet->isMust()) {
                        retSideEffect->insertLDEF(tag);
                        retSideEffect->insertDEF(tag);
                     }
                 }
            }
        }

        // Iterate over procedure calls of a statement
        // to determine their effect
        OA_ptr<IRCallsiteIterator> callsiteItPtr = mIR->getCallsites(stmt);
        for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
            CallHandle call = callsiteItPtr->current();
            if (debug) {
                cout << "\tcall = " << mIR->toString(call) << endl;
            }

            OA_ptr<AliasTagIterator> tagsIter;

            // MOD
            tagsIter = inter->getMODIterator(call);
            for ( ; tagsIter->isValid(); ++(*tagsIter)) {
                retSideEffect->insertMOD(tagsIter->current());
                if (debug) {
                    cout << "\tinserting tag = ";
                    cout << tagsIter->current() << " ";
                    cout << "\tinto MOD" << endl;
                }
            }
            // DEF
            tagsIter = inter->getDEFIterator(call);
            for ( ; tagsIter->isValid(); ++(*tagsIter)) {
                retSideEffect->insertDEF(tagsIter->current());
            } 
            // USE
            tagsIter = inter->getUSEIterator(call);
            for ( ; tagsIter->isValid(); ++(*tagsIter)) {
                AliasTag tag = tagsIter->current();
                if (debug) {
                    cout << "SideEffectStandard USE retSideEffect  = ";
                    cout << tag << " " << endl;
                }
                retSideEffect->insertUSE(tag);  
            } 
            // REF
            tagsIter = inter->getREFIterator(call);
            for ( ; tagsIter->isValid(); ++(*tagsIter)) {
                retSideEffect->insertREF(tagsIter->current());
            } 
        }
    } // end of loop over statements

    return retSideEffect;
}

  } // end of namespace SideEffect
} // end of namespace OA
