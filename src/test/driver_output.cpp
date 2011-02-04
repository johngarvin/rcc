/*! \file
    
  \brief This program reads in a .OA file into a SubsidiaryIR then
         outputs the contents of that SubsidiaryIR.

  \authors Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "loadNotation.hpp"
#include "NotationGenerator.hpp"
#include "TestAliasIRInterface.hpp"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char **argv)
{
    string sInFile;
    OA::OA_ptr<SubsidiaryIR> subIR;
    OA::OA_ptr<TestAliasIRInterface> aliasIR;

    if(argc > 1) {
        sInFile = argv[1];
    }
    else {
        cout << "usage: output file.oa" << endl;
        return -1;
    }
    
    subIR = loadNotation(sInFile);
    aliasIR = new TestAliasIRInterface(subIR);
    subIR->output(aliasIR);
}

