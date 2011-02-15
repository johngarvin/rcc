//     THIS IS AN AUTOMATICALLY GENERATED FILE
//     CREATED BY GenOutputTool.

#include "UDDUChainsStandard.hpp"
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {

    namespace UDDUChains {

void UDDUChainsStandard::output(OA::IRHandlesIRInterface &ir)
{
    sOutBuild->objStart("UDDUChainsStandard");

    sOutBuild->mapStart("mUDChainForStmt", "StmtHandle", "OA::OA_ptr<StmtSet> ");
    std::map<StmtHandle, OA::OA_ptr<StmtSet> >::iterator reg_mUDChainForStmt_iterator;
    for(reg_mUDChainForStmt_iterator = mUDChainForStmt->begin();
        reg_mUDChainForStmt_iterator != mUDChainForStmt->end();
        reg_mUDChainForStmt_iterator++)
    {
        const StmtHandle &first = reg_mUDChainForStmt_iterator->first;
        OA::OA_ptr<StmtSet>  &second = reg_mUDChainForStmt_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();

        //sOutBuild->objStart("ActiveStmtSet"); {
        //  sOutBuild->listStart(); {
        //    OA_ptr<StmtHandleIterator> stmtIterPtr;
        //    stmtIterPtr=getUDChainStmtIterator(first);

        //    for ( ; stmtIterPtr->isValid(); (*stmtIterPtr)++ ) {
        //      sOutBuild->listItemStart(); {
        //        sOutBuild->outputIRHandle(stmtIterPtr->current(),ir);
        //      }sOutBuild->listItemEnd();
        //    }
        //  }sOutBuild->listEnd();
        //}sOutBuild->objEnd("ActiveStmtSet");


        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mUDChainForStmt");

    sOutBuild->mapStart("mDUChainForStmt", "StmtHandle", "OA::OA_ptr<StmtSet> ");
    std::map<StmtHandle, OA::OA_ptr<StmtSet> >::iterator reg_mDUChainForStmt_iterator;
    for(reg_mDUChainForStmt_iterator = mDUChainForStmt->begin();
        reg_mDUChainForStmt_iterator != mDUChainForStmt->end();
        reg_mDUChainForStmt_iterator++)
    {
        const StmtHandle &first = reg_mDUChainForStmt_iterator->first;
        OA::OA_ptr<StmtSet>  &second = reg_mDUChainForStmt_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mDUChainForStmt");

    sOutBuild->mapStart("mUDChainForMemRef", "MemRefHandle", "OA::OA_ptr<StmtSet> ");
    std::map<MemRefHandle, OA::OA_ptr<StmtSet> >::iterator reg_mUDChainForMemRef_iterator;
    for(reg_mUDChainForMemRef_iterator = mUDChainForMemRef->begin();
        reg_mUDChainForMemRef_iterator != mUDChainForMemRef->end();
        reg_mUDChainForMemRef_iterator++)
    {
        const MemRefHandle &first = reg_mUDChainForMemRef_iterator->first;
        OA::OA_ptr<StmtSet>  &second = reg_mUDChainForMemRef_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mUDChainForMemRef");

    sOutBuild->mapStart("mDUChainForMemRef", "MemRefHandle", "OA::OA_ptr<StmtSet> ");
    std::map<MemRefHandle, OA::OA_ptr<StmtSet> >::iterator reg_mDUChainForMemRef_iterator;
    for(reg_mDUChainForMemRef_iterator = mDUChainForMemRef->begin();
        reg_mDUChainForMemRef_iterator != mDUChainForMemRef->end();
        reg_mDUChainForMemRef_iterator++)
    {
        const MemRefHandle &first = reg_mDUChainForMemRef_iterator->first;
        OA::OA_ptr<StmtSet>  &second = reg_mDUChainForMemRef_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        //second->output(ir);
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mDUChainForMemRef");

    sOutBuild->mapStart("mMemRefToStmtMap", "MemRefHandle", "StmtHandle");
    std::map<MemRefHandle, StmtHandle>::iterator reg_mMemRefToStmtMap_iterator;
    for(reg_mMemRefToStmtMap_iterator = mMemRefToStmtMap.begin();
        reg_mMemRefToStmtMap_iterator != mMemRefToStmtMap.end();
        reg_mMemRefToStmtMap_iterator++)
    {
        const MemRefHandle &first = reg_mMemRefToStmtMap_iterator->first;
        StmtHandle &second = reg_mMemRefToStmtMap_iterator->second;
        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(first, ir);

        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
        sOutBuild->outputIRHandle(second, ir);

        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mMemRefToStmtMap");

    sOutBuild->objEnd("UDDUChainsStandard");
}

} // end of namespace UDDUChains

} // end of namespace OA
