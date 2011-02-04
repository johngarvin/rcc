//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "SideEffectStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace SideEffect { 

void SideEffectStandard::output(OA::IRHandlesIRInterface &ir,
                                Alias::Interface& alias) const
{
    sOutBuild->objStart("SideEffectStandard");

    sOutBuild->fieldStart("mLMODSet");
    if(mLMODSet.ptrEqual(0)) {
        sOutBuild->field("mLMODSet", "NULL");
    } else {
        mLMODSet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mLMODSet");

    sOutBuild->fieldStart("mMODSet");
    if(mMODSet.ptrEqual(0)) {
        sOutBuild->field("mMODSet", "NULL");
    } else {
        mMODSet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mMODSet");

    sOutBuild->fieldStart("mLDEFSet");
    if(mLDEFSet.ptrEqual(0)) {
        sOutBuild->field("mLDEFSet", "NULL");
    } else {
        mLDEFSet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mLDEFSet");

    sOutBuild->fieldStart("mDEFSet");
    if(mDEFSet.ptrEqual(0)) {
        sOutBuild->field("mDEFSet", "NULL");
    } else {
        mDEFSet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mDEFSet");



    sOutBuild->fieldStart("mLUSESet");
    if(mLUSESet.ptrEqual(0)) {
        sOutBuild->field("mLUSESet", "NULL");
    } else {
        mLUSESet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mLUSESet");

    sOutBuild->fieldStart("mUSESet");
    if(mUSESet.ptrEqual(0)) {
        sOutBuild->field("mUSESet", "NULL");
    } else {
        mUSESet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mUSESet");

    sOutBuild->fieldStart("mLREFSet");
    if(mLREFSet.ptrEqual(0)) {
        sOutBuild->field("mLREFSet", "NULL");
    } else {
        mLREFSet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mLREFSet");

    sOutBuild->fieldStart("mREFSet");
    if(mREFSet.ptrEqual(0)) {
        sOutBuild->field("mREFSet", "NULL");
    } else {
        mREFSet->output(ir,alias);
    }
    sOutBuild->fieldEnd("mREFSet");

    sOutBuild->objEnd("SideEffectStandard");
}


    } // end of SideEffect namespace
} // end of OA namespace
