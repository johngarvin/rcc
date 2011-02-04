//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "ParamBindings.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

  namespace DataFlow {

    void ParamBindings::output(IRHandlesIRInterface& ir) const
    {

         sOutBuild->objStart("ParamBindings");

        sOutBuild->mapStart("mCallToFormalToActual", "CallHandle", "std::list<std::pair<OA_ptr<OA::MemRefExpr>,OA_ptr<OA::ExprTree> > >");
         std::map<CallHandle, std::list<std::pair<OA_ptr<OA::MemRefExpr>,OA_ptr<OA::ExprTree> > > >::iterator reg_mCallToFormalToActual_iterator;

         for(reg_mCallToFormalToActual_iterator = mCallToFormalToActual.begin();
            reg_mCallToFormalToActual_iterator != mCallToFormalToActual.end();
            reg_mCallToFormalToActual_iterator++)
         {
            const CallHandle &first = reg_mCallToFormalToActual_iterator->first;
            std::list<std::pair<OA_ptr<OA::MemRefExpr>,OA_ptr<OA::ExprTree> > >  &second
                 = reg_mCallToFormalToActual_iterator->second;

            sOutBuild->mapEntryStart();
            sOutBuild->mapKeyStart();
            sOutBuild->outputIRHandle(first, ir);
            sOutBuild->mapKeyEnd();
            sOutBuild->mapValueStart();


            sOutBuild->mapStart("second", "MemRefExpr", "ExprTree");
            std::list<std::pair<OA_ptr<OA::MemRefExpr>,OA_ptr<OA::ExprTree> > >::iterator reg_second_iterator;
            for(reg_second_iterator = second.begin();
                reg_second_iterator != second.end();
                reg_second_iterator++)
            {
                    const OA_ptr<OA::MemRefExpr> &first = reg_second_iterator->first;
                    OA_ptr<OA::ExprTree> exprTree = reg_second_iterator->second;
                    sOutBuild->mapEntryStart();
                    sOutBuild->mapKeyStart();
                       first->output(ir);
                    sOutBuild->mapKeyEnd();
                       sOutBuild->mapValueStart();
                          exprTree->output(ir);
                       sOutBuild->mapValueEnd();
                    sOutBuild->mapEntryEnd();
            }

            sOutBuild->mapEnd("second");
            sOutBuild->mapValueEnd();
            sOutBuild->mapEntryEnd();
            
         }

         sOutBuild->objEnd("ParamBindings");
    }

  } // end of DataFlow namespace
} // end of OA namespace
~

