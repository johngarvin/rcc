//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "LoopAbstraction.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace Loop { 

void LoopAbstraction::output(OA::IRHandlesIRInterface &ir)
{
    sOutBuild->objStart("LoopAbstraction");

    sOutBuild->outputIRHandle(mLoopStatement, ir);

    if(mLoopIndex.ptrEqual(0)) {
        sOutBuild->field("mLoopIndex", "NULL");
    } else {
        mLoopIndex->output(ir);
    }
    if(mParent.ptrEqual(0)) {
        sOutBuild->field("mParent", "NULL");
    } else {
        mParent->output(ir);
    }
    sOutBuild->objEnd("LoopAbstraction");
}


    } // end of Loop namespace
} // end of OA namespace
