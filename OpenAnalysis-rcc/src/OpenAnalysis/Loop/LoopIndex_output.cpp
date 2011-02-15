//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "LoopIndex.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
  namespace Loop {

void LoopIndex::output(OA::IRHandlesIRInterface &ir)
{
    sOutBuild->objStart("LoopIndex");

    mVariable->output(ir);
    sOutBuild->field("mInitialBound", OA::int2string(mInitialBound));
    sOutBuild->field("mTerminalBound", OA::int2string(mTerminalBound));
    sOutBuild->field("mStep", OA::int2string(mStep));
    sOutBuild->objEnd("LoopIndex");
}

} // end of Loop namespace
} // end of OA namespace
