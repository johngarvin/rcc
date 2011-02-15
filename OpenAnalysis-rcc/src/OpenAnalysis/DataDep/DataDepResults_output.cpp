//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "DataDepResults.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace DataDep { 

void DataDepResults::output(OA::IRHandlesIRInterface &ir)
{
    sOutBuild->objStart("DataDepResults");

    sOutBuild->field("temp", OA::int2string(temp));
    sOutBuild->objEnd("DataDepResults");
}


    } // end of DataDep namespace
} // end of OA namespace
