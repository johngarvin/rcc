/*! \file
    
  \brief This program reads in a .OA file into a SubsidiaryIR. It
         then outputs this SubsidiaryIR into another .OA file,
         effectively copying the original.

  \authors Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "loadNotation.hpp"
#include "NotationGenerator.hpp"
#include "testSubIR.hpp"
#include "TestAliasIRInterface.hpp"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    string sInFile, sOutFile;
    OA::OA_ptr<SubsidiaryIR> subIR;
    OA::OA_ptr<TestAliasIRInterface> aliasIR;

    if(argc > 2) {
        sInFile = argv[1];
        sOutFile = argv[2];
    }
    else {
        cout << "usage: oacopy infile.oa outfile.oa" << endl;
        return -1;
    }

    std::ofstream outFile(sOutFile.c_str());

    subIR = loadNotation(sInFile);
    aliasIR = new TestAliasIRInterface(subIR);
    NotationGenerator codeGen(aliasIR, outFile);
    OA::OA_ptr<IRProcIterator> i = aliasIR->getProcIterator();
    while(i->isValid()) {
        codeGen.generate(i->current());
        (*i)++;
    }
}

