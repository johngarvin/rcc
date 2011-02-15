//     This file has been modified from its original GenOutputTool version
//     specifically:
//          - AliasTag objects are now being passed a pointer to this results (this)

#include "AliasTagResults.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA { 
    namespace Alias { 

void AliasTagResults::output(OA::IRHandlesIRInterface &ir) const
{
    sOutBuild->objStart("AliasTagResults");

    //sOutBuild->mapStart("mMREToTags", "OA::OA_ptr<MemRefExpr> ", "OA::OA_ptr<std::set<AliasTag> > ");
    sOutBuild->mapStart("mMREToTags", "OA::OA_ptr<MemRefExpr> ", "OA::OA_ptr<AliasTagSet> ");
    //std::map<OA::OA_ptr<MemRefExpr> , OA::OA_ptr<std::set<AliasTag> > >::const_iterator reg_mMREToTags_iterator;
    std::map<OA::OA_ptr<MemRefExpr> , OA::OA_ptr<AliasTagSet> >::const_iterator reg_mMREToTags_iterator;
    for(reg_mMREToTags_iterator = mMREToTags.begin();
        reg_mMREToTags_iterator != mMREToTags.end();
        reg_mMREToTags_iterator++)
    {
        const OA::OA_ptr<MemRefExpr>  &first = reg_mMREToTags_iterator->first;
        //const OA::OA_ptr<std::set<AliasTag> >  &second = reg_mMREToTags_iterator->second;
        const OA::OA_ptr<AliasTagSet>  &second = reg_mMREToTags_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        if(first.ptrEqual(0)) {
            sOutBuild->field("first", "NULL");
        } else {
            first->output(ir);
        }
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        second->output(ir);
        //sOutBuild->listStart();
        //std::set<AliasTag>::const_iterator reg_second_iterator;
        //for(reg_second_iterator  = second->begin();
        //    reg_second_iterator != second->end();
        //    reg_second_iterator++)
       // {
        //    AliasTag item = *reg_second_iterator;
        //    sOutBuild->listItemStart();
        //    item.output(ir, *this);
        //    sOutBuild->listItemEnd();
       // }
        //sOutBuild->listEnd();
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMREToTags");

    sOutBuild->mapStart("mTagToMREs", "AliasTag", "OA::OA_ptr<std::set<OA::OA_ptr<MemRefExpr> > > ");
    std::map<AliasTag, OA::OA_ptr<std::set<OA::OA_ptr<MemRefExpr> > > >::const_iterator reg_mTagToMREs_iterator;
    for(reg_mTagToMREs_iterator = mTagToMREs.begin();
        reg_mTagToMREs_iterator != mTagToMREs.end();
        reg_mTagToMREs_iterator++)
    {
        AliasTag first = reg_mTagToMREs_iterator->first;
        OA::OA_ptr<std::set<OA::OA_ptr<MemRefExpr> > >  second = reg_mTagToMREs_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        first.output(ir, *this);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        sOutBuild->listStart();
        std::set<OA::OA_ptr<MemRefExpr> >::const_iterator reg_second_iterator;
        for(reg_second_iterator  = second->begin();
            reg_second_iterator != second->end();
            reg_second_iterator++)
        {
            OA::OA_ptr<MemRefExpr>  item = *reg_second_iterator;
            sOutBuild->listItemStart();
            if(item.ptrEqual(0)) {
                sOutBuild->field("item", "NULL");
            } else {
                item->output(ir);
            }
            sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mTagToMREs");

    //sOutBuild->mapStart("mMemRefToTags", "MemRefHandle", "OA::OA_ptr<std::set<AliasTag> > ");
    sOutBuild->mapStart("mMemRefToTags", "MemRefHandle", "OA::OA_ptr<AliasTagSet> ");
    //std::map<MemRefHandle, OA::OA_ptr<std::set<AliasTag> > >::const_iterator reg_mMemRefToTags_iterator;
    std::map<MemRefHandle, OA::OA_ptr<AliasTagSet> >::const_iterator reg_mMemRefToTags_iterator;
    for(reg_mMemRefToTags_iterator = mMemRefToTags.begin();
        reg_mMemRefToTags_iterator != mMemRefToTags.end();
        reg_mMemRefToTags_iterator++)
    {
        const MemRefHandle &first = reg_mMemRefToTags_iterator->first;
        const OA::OA_ptr<AliasTagSet>  &second = reg_mMemRefToTags_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        second->output(ir);
        //sOutBuild->listStart();
        //std::set<AliasTag>::const_iterator reg_second_iterator;
        //for(reg_second_iterator  = second->begin();
        //    reg_second_iterator != second->end();
        //    reg_second_iterator++)
       // {
        //    AliasTag item = *reg_second_iterator;
        //    sOutBuild->listItemStart();
        //    item.output(ir, *this);
        //    sOutBuild->listItemEnd();
       // }
        //sOutBuild->listEnd();
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMemRefToTags");

    sOutBuild->mapStart("mMREToMustFlag", "OA::OA_ptr<MemRefExpr> ", "bool");
    std::map<OA::OA_ptr<MemRefExpr> , bool>::const_iterator reg_mMREToMustFlag_iterator;
    for(reg_mMREToMustFlag_iterator = mMREToMustFlag.begin();
        reg_mMREToMustFlag_iterator != mMREToMustFlag.end();
        reg_mMREToMustFlag_iterator++)
    {
        const OA::OA_ptr<MemRefExpr>  &first = reg_mMREToMustFlag_iterator->first;
        bool second = reg_mMREToMustFlag_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        if(first.ptrEqual(0)) {
            sOutBuild->field("first", "NULL");
        } else {
            first->output(ir);
        }
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValue(bool2string(second));
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMREToMustFlag");

    sOutBuild->mapStart("mMemRefToMustFlag", "MemRefHandle", "bool");
    std::map<MemRefHandle, bool>::const_iterator reg_mMemRefToMustFlag_iterator;
    for(reg_mMemRefToMustFlag_iterator = mMemRefToMustFlag.begin();
        reg_mMemRefToMustFlag_iterator != mMemRefToMustFlag.end();
        reg_mMemRefToMustFlag_iterator++)
    {
        MemRefHandle first = reg_mMemRefToMustFlag_iterator->first;
        bool second = reg_mMemRefToMustFlag_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValue(bool2string(second));
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMemRefToMustFlag");

    sOutBuild->objEnd("AliasTagResults");
}


    } // end of Alias namespace
} // end of OA namespace
