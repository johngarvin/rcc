//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "ReachConstsStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace ReachConsts { 

void ConstDef::output(OA::IRHandlesIRInterface &ir) const
{
    sOutBuild->objStart("ConstDef");

    sOutBuild->fieldStart("Location");
    mLocPtr->output(ir);
    sOutBuild->fieldEnd("Location");

  // mCDType.output(ir);

    sOutBuild->fieldStart("mCDType");
    std::ostringstream oss;
    switch (mCDType) {
      case TOP: 
         oss << ",TOP>"; break;
      case BOTTOM: 
         oss << ",BOTTOM>"; break;
      case VALUE:
         oss << ",VALUE=" << (*mConstPtr).toString() << ">"; break;
     }
    sOutBuild->fieldEnd("mCDType");
     // return oss.str();
    
     sOutBuild->outputString( oss.str());

     sOutBuild->objEnd("ConstDef");
}

void ConstDefSet::output(OA::IRHandlesIRInterface &ir) const
{
    sOutBuild->objStart("ConstDefSet");

    sOutBuild->listStart();
    std::set<OA::OA_ptr<ConstDef> >::iterator reg_mSet_iterator;
    for(reg_mSet_iterator  = mSet->begin();
        reg_mSet_iterator != mSet->end();
        reg_mSet_iterator++)
    {
        OA::OA_ptr<ConstDef>  item = *reg_mSet_iterator;
        sOutBuild->listItemStart();
        sOutBuild->fieldStart("ConstDefSet");
        item->output(ir);
        sOutBuild->fieldEnd("ConstDefSet");
        sOutBuild->listItemEnd();
    }
    sOutBuild->listEnd();
    sOutBuild->objEnd("ConstDefSet");
}

void ReachConstsStandard::output(OA::IRHandlesIRInterface &ir) const
{
    sOutBuild->objStart("ReachConstsStandard");

    sOutBuild->mapStart("mReachConsts", "StmtHandle", "OA::OA_ptr<ConstDefSet> ");
    std::map<StmtHandle, OA::OA_ptr<ConstDefSet> >::iterator reg_mReachConsts_iterator;
    for(reg_mReachConsts_iterator = mReachConsts.begin();
        reg_mReachConsts_iterator != mReachConsts.end();
        reg_mReachConsts_iterator++)
    {
        const StmtHandle &first = reg_mReachConsts_iterator->first;
        OA::OA_ptr<ConstDefSet>  &second = reg_mReachConsts_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->fieldStart("Statement");
        sOutBuild->outputIRHandle(first, ir);
        sOutBuild->fieldEnd("Statement");
 
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mReachConsts");

    sOutBuild->objEnd("ReachConstsStandard");
}


    } // end of ReachConsts namespace
} // end of OA namespace
