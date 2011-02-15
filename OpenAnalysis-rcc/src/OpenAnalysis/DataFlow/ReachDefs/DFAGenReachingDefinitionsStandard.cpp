
// DFAGenReachingDefinitionsStandard.cpp


#include <OpenAnalysis/DataFlow/ReachDefs/DFAGenReachingDefinitionsStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilder.hpp>

namespace OA {
  namespace DFAGenReachingDefinitions {

OA_ptr<LocIterator> DFAGenReachingDefinitionsStandard::getDFAGenReachingDefinitionsIterator(StmtHandle s) 
{ 
    OA_ptr<LocIterator> retval;

    if (mDFAGenReachingDefinitions[s].ptrEqual(0)) {
        OA_ptr<LocSet> emptySet; emptySet = new LocSet;
        retval = new LocSetIterator(emptySet);
    } else {
    retval = new LocSetIterator(mDFAGenReachingDefinitions[s]);
    }
    return retval;
}

OA_ptr<LocIterator> DFAGenReachingDefinitionsStandard::getExitDFAGenReachingDefinitionsIterator() 
{ 
    OA_ptr<LocIterator> retval;
     if (mExitDFAGenReachingDefinitions.ptrEqual(0)) {
        OA_ptr<LocSet> emptySet; emptySet = new LocSet;
        retval = new LocSetIterator(emptySet);
    }
    else {
    retval = new LocSetIterator(mExitDFAGenReachingDefinitions);
    }
    return retval;
}

void DFAGenReachingDefinitionsStandard::output(IRHandlesIRInterface& ir)
{    
    sOutBuild->objStart("DFAGenReachingDefinitionsStandard");

    sOutBuild->mapStart("mDFAGenReachingDefinitions", "StmtHandle", "OA_ptr<LocSet>");
    std::map<StmtHandle,OA_ptr<LocSet> >::iterator mapIter;
    for (mapIter = mDFAGenReachingDefinitions.begin(); mapIter != mDFAGenReachingDefinitions.end(); mapIter++) {
        StmtHandle s = mapIter->first;
        OA_ptr<LocSet> rdset = mapIter->second;
        if ( rdset.ptrEqual(0) ) continue;

        sOutBuild->mapEntryStart();
        sOutBuild->mapKeyStart();
        sOutBuild->outputIRHandle(s, ir);
        sOutBuild->mapKeyEnd();
        sOutBuild->mapValueStart();
       
       
        sOutBuild->listStart();
        OA_ptr<LocIterator> setIter;
        setIter = getDFAGenReachingDefinitionsIterator(s);
        for ( ; setIter->isValid(); (*setIter)++ ) {
             sOutBuild->listItemStart(); {
             setIter->current()->output(ir);  
            } sOutBuild->listItemEnd();
        }
        sOutBuild->listEnd();  
        sOutBuild->mapValueEnd();
        sOutBuild->mapEntryEnd();
    }
    sOutBuild->mapEnd("mDFAGenReachingDefinitions");
    sOutBuild->objEnd("DFAGenReachingDefinitionsStandard");
}



void DFAGenReachingDefinitionsStandard::dump(std::ostream& os, OA_ptr<IRHandlesIRInterface> ir)
{
    std::cout<< "======DFAGenReachingDefinitions===="<< std::endl;
    OA_ptr<LocIterator> locIterPtr;
    std::map<StmtHandle,OA_ptr<LocSet> > ::iterator mapIter;
    for (mapIter = mDFAGenReachingDefinitions.begin(); mapIter != mDFAGenReachingDefinitions.end(); mapIter++) {
        std::cout<< "\tstmt = " << ir->toString(mapIter->first) << std::endl; 
         locIterPtr = getDFAGenReachingDefinitionsIterator(mapIter->first);
        for ( ; locIterPtr->isValid(); (*locIterPtr)++ ) {
            std::cout << "\t\t";
            locIterPtr->current()->dump(std::cout,ir);
            std::cout << std::endl;
        }

    }
}



  } // end of DFAGenReachingDefinitions namespace
} // end of OA namespace
