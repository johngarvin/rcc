//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "AffineExprAbstraction.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace AffineExpr { 

void AffineExprAbstraction::output(OA::IRHandlesIRInterface &ir)
{
    sOutBuild->objStart("AffineExprAbstraction");

    sOutBuild->mapStart("mCoefficients", "OA::OA_ptr<NamedLoc> ", "int");
    std::map<OA::OA_ptr<NamedLoc> , int>::iterator reg_mCoefficients_iterator;
    for(reg_mCoefficients_iterator = mCoefficients.begin();
        reg_mCoefficients_iterator != mCoefficients.end();
        reg_mCoefficients_iterator++)
    {
        const OA::OA_ptr<NamedLoc>  &first = reg_mCoefficients_iterator->first;
        int &second = reg_mCoefficients_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        first->output(ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValue(OA::int2string(second));
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mCoefficients");

    sOutBuild->field("mOffset", OA::int2string(mOffset));
    sOutBuild->objEnd("AffineExprAbstraction");
}


    } // end of AffineExpr namespace
} // end of OA namespace
