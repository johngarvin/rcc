/*! \file
    
  \brief Functions to load notation files

  \authors Andy Stone
 
  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#include "loadNotation.hpp"

extern int oa_parse();

OA::OA_ptr<SubsidiaryIR> gSubIR;
OA::OA_ptr<StrToHandle> gStrToHandle;
char * input_buffer;
char * input_ptr;
char * input_end;

OA::OA_ptr<SubsidiaryIR> loadNotation(string file)
{
    FILE * pFile;
    long lSize;

    pFile = fopen(file.c_str(), "rb");
    if (pFile==NULL) exit (1);

    // obtain file size
    fseek (pFile , 0 , SEEK_END);
    lSize = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file
    input_buffer = (char*) malloc (lSize);
    if (input_buffer == NULL) exit (2);
    input_end = input_buffer + lSize;

    // copy the whole file into the buffer
    fread (input_buffer,1,lSize,pFile);
    input_ptr = input_buffer;
    
    // initialize globals
    gSubIR = new SubsidiaryIR();
    gStrToHandle = new StrToHandle();

    // parse all possible IRInterface information out of the file
    oa_parse();
  
    return gSubIR;
}

int my_yyinput_getchar()
{
    int retchar;

    if (input_ptr != input_end) {
        retchar = input_ptr[0];
        input_ptr ++;
    } else {
        retchar = EOF;
    }

    return retchar;
}

