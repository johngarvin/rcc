//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "Loop.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 

void Loop::output(OA::IRHandlesIRInterface &ir)
{
    sOutBuild->objStart("Loop");

    sOutBuild->listStart();
    std::set<StmtHandle>::iterator reg_mStatements_iterator;
    for(reg_mStatements_iterator  = mStatements.begin();
        reg_mStatements_iterator != mStatements.end();
        reg_mStatements_iterator++)
    {
        StmtHandle item = *reg_mStatements_iterator;
        sOutBuild->listItemStart();
        sOutBuild->outputIRHandle(item, ir);

        sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
    mLoopIndex->output(ir);
    sOutBuild->objEnd("Loop");
}


} // end of OA namespace
