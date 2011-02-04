/*! \file

  \brief FLEX file describing the tokens that are parsed in .oa notation
         files.

  \authors Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

%{
    #include <math.h>
    #include <string.h>
    #include <stdlib.h>
    #include <iostream>
    #include "SubsidiaryIR.hpp"
    #include "oa-tab.hpp"
    using namespace std;

    #define YY_NO_UNPUT
    #undef YY_INPUT
    extern int my_yyinput_getchar();
    #define YY_INPUT(buf,result,max_size) \
    { \
        int c = my_yyinput_getchar(); \
        result = (c == EOF) ? YY_NULL : (buf[0] = c, 1); \
    }

    int oa_lineno = 0;
%}

DIGIT   [0-9]
ID      [a-zA-Z][a-zA-Z0-9]*

%%

 /* newlines, misc. characters, and stringed quotes */
\n                    { oa_lineno++; }
[\(,\)\[\]\<\>\=\{\}] { return yytext[0]; }
"\""[^\"]*"\""        { char *retstr = (char*)malloc(strlen(yytext)+1);
                        // don't want quotes in string
                        strcpy(retstr,&(yytext[1]));
                        retstr[strlen(yytext)-2] = 0;
                        oa_lval.sval = retstr; 
                        return T_STR_LITERAL; 
                      }
{DIGIT}+              { oa_lval.ival = atoi(yytext);
                        return T_INT_LITERAL;;
                      }

PROCEDURE               { return T_PROCEDURE; }
LOCATION                { return T_LOCATION; }
MEMREFEXPRS             { return T_MEMREFEXPRS; }
PTRASSIGNPAIRS          { return T_PTRASSIGNPAIRS; }
CALLSITES               { return T_CALLSITES; }
FORMALS                 { return T_FORMALS; }
PARAMBINDPTRASSIGNPAIRS { return T_PARAMBINDPTRASSIGNPAIRS; }

CallHandle    { return T_CALLHANDLE; }
MemRefHandle  { return T_MEMREFHANDLE; }
ProcHandle    { return T_PROCHANDLE; }
SymHandle     { return T_SYMHANDLE; }
StmtHandle    { return T_STMTHANDLE; }

NamedRef      { return T_NAMEDREF; }
UnknownRef    { return T_UNKNOWNREF; }
UnnamedRef    { return T_UNNAMEDREF; }
Deref         { return T_DEREF; }

local         { return T_LOCAL; }
not           { return T_NOT; }
"\=\>"        { return T_ARROW; }



DEF           { return T_DEF; }
USE           { return T_USE; }
USEDEF        { return T_USEDEF; }
DEFUSE        { return T_DEFUSE; }

T           { return T_T; }
F           { return T_F; }
addressOf   { return T_ADDRESSOF; }
accuracy    { return T_ACCURACY; }
full        { return T_FULL; }
partial |
part        { return T_PART; }

 /* Comments and ignored spaces */

"\/\/"[^\n]*"\n" { oa_lineno++; }
[ \t]+

%%

int yywrap() { return 1; }

