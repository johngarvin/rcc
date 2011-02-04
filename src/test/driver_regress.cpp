/*! \file
    
  \brief Driver for regression tests
    
  This program currently runs the ManagerFIAlias algorithm
  to perform alias analysis on the .oa file provided on the command
  line.  It then outputs the resulting EquivSets to stdout.
  The generated results can be checked against results generated in
  the past to perform regression tests.

  \authors Michelle Strout, Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "TestAliasIRInterface.hpp"
#include "loadNotation.hpp"
#include "CmdParams.h"
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
#include <iostream>
#include <fstream>
using namespace std;

/*! identifies what type of analysis we're going to perform in the
    regression test */
enum ANALYSIS {
    ANALYSIS_FIALIAS = 0,
    ANALYSIS_ALIASMAPBASIC,
    ANALYSIS_CFG,
    ANALYSIS_NONE
};

/*! holds information parsed from command line arguments */
struct CommandLineArgs {
    ANALYSIS analysis;
    string notationFile;

    CommandLineArgs() {
        analysis = ANALYSIS_NONE;
    }
};

// maps a numeric identifier to a command line argument string for some
// specific type of analysis
EnumStringPair analysisPairs[] =
    { {ANALYSIS_FIALIAS, "FIAlias"},
      {ANALYSIS_ALIASMAPBASIC, "AliasMapBasic"},
      {ANALYSIS_CFG, "CFG"}, 
      {ANALYSIS_NONE, "NONE"}
    };
int num_analysisPairs = 4;



/*! parse information from the command line arguments.  If there's
    an error in parsing, display usage information and set the analysis
    to perform as ANALYSIS_NONE */
CommandLineArgs parseCommandLine(int argc, char **argv)
{
    CommandLineArgs parsedArgs;

    // describe the expected parameters
    CmdParams *cmdParams = CmdParams_ctor(0);
    CmdParams_describeEnumParam(cmdParams, "analysis",
            'a', 0, "analysis to perform",
            analysisPairs, num_analysisPairs, ANALYSIS_NONE);
    CmdParams_describeStringParam(cmdParams, "notationFile",
            'f', 0, "notation file", "");

    // if no options are passed automatically display help
    if(argc == 1) {
        CmdParams_printHelp(cmdParams);
        return parsedArgs;
     }

    // parse the params
    CmdParams_parseParams(cmdParams, argc, argv);
    parsedArgs.analysis = (ANALYSIS)CmdParams_getValue(cmdParams, 'a');
    parsedArgs.notationFile = CmdParams_getString(cmdParams, 'f');

    return parsedArgs;
}

int main(int argc, char **argv)
{
    CommandLineArgs args = parseCommandLine(argc, argv);
    
    OA::OA_ptr<SubsidiaryIR> subIR;
    OA::OA_ptr<TestAliasIRInterface> aliasIR;

    subIR = loadNotation(args.notationFile);
    //subIR->output(aliasIR);
    aliasIR = new TestAliasIRInterface(subIR);
    
    // almost all of these need a procIter
    OA::OA_ptr<OA::IRProcIterator> procIter = aliasIR->getProcIterator();

    // declarations that can not be within the switch statement
    OA::OA_ptr<OA::Alias::ManagerFIAlias> fialiasman;
    OA::OA_ptr<OA::Alias::EquivSets> equivSets; 

    OA::OA_ptr<OA::Alias::ManagerAliasMapBasic> aliasmapman;
    OA::OA_ptr<OA::Alias::AliasMap> aliasmap;
    
    // do specified analysis
    switch (args.analysis)
    {
        case ANALYSIS_FIALIAS: 
            fialiasman= new OA::Alias::ManagerFIAlias(aliasIR);
            equivSets = fialiasman->performAnalysis(procIter);
            equivSets->output(aliasIR);
            
            break;

        case ANALYSIS_ALIASMAPBASIC:
            aliasmapman = new OA::Alias::ManagerAliasMapBasic(aliasIR);

            for (procIter->reset(); procIter->isValid(); (*procIter)++ ) {
                ProcHandle proc = procIter->current();
                aliasmap = aliasmapman->performAnalysis(proc);
                aliasmap->output(aliasIR);
            }
            break;

        case ANALYSIS_CFG:
            std::cerr << "CFG Analysis still needs to be implemented."
                      << endl;
            break;

        case ANALYSIS_NONE:
            return 1;
            break;

        default:
            std::cout << "Unknown analysis specified" << endl;
            break;
    } 

    return 0;
}

