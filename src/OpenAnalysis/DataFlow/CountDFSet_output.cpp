//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include <OpenAnalysis/DataFlow/CountDFSet.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace DataFlow { 

void CountDFSet::output(OA::IRHandlesIRInterface &ir) const
{
    sOutBuild->objStart("CountDFSet");

    sOutBuild->field("mCount", OA::int2string(mCount));
    sOutBuild->objEnd("CountDFSet");
}


    } // end of DataFlow namespace
} // end of OA namespace
