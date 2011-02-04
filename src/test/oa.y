/* \file

  \brief Context Free Grammers used by bison to parse .oa notation files.
  
  \authors Andy Stone

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/


%{
    extern int yylex ();
    #include "SubsidiaryIR.hpp"
    #include "StrToHandle.hpp"
    #include <iostream>
    using namespace std;
    using namespace OA;
    extern OA_ptr<SubsidiaryIR> gSubIR;
    extern OA_ptr<StrToHandle> gStrToHandle;
    #define YYERROR_VERBOSE
    #define YYDEBUG 1
    void yyerror(char*);
    extern char *oa_text;
    extern int oa_lineno;
%}

%union {
    char *sval;
    int  ival;
    bool bval ;
    OA::ProcHandle   *prochandle;
    OA::SymHandle    *symhandle;
    OA::StmtHandle   *stmthandle;
    OA::CallHandle   *callhandle;
    OA::MemRefHandle *memrefhandle;
    OA::MemRefExpr   *memrefexpr;
    OA::MemRefExpr::MemRefType defuse;
    std::pair<OA::OA_ptr<OA::MemRefExpr>,
              OA::OA_ptr<OA::MemRefExpr> > *ptrassignpair;
    std::list<std::pair<OA::CallHandle,
                        OA::OA_ptr<OA::MemRefExpr> > > *callsitelist;
    std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>,
                        OA::OA_ptr<OA::MemRefExpr> > > *ptrassignlist;
    std::list<std::pair<int,
                        OA::OA_ptr<OA::MemRefExpr> > > *paramptrlist;
    std::list<std::pair<OA::MemRefHandle,
                        OA::OA_ptr<OA::MemRefExpr> > > *memrefexprslist;
    std::list<std::pair<int, OA::SymHandle> > *formallist;
};

%token <sval> T_STR_LITERAL
%token <ival> T_INT_LITERAL
%token T_PROCHANDLE T_SYMHANDLE T_STMTHANDLE T_CALLHANDLE T_MEMREFHANDLE
%token T_EXPRHANDLE T_OPHANDLE T_CONSTSYMHANDLE T_CONSTVALHANDLE
%token T_NAMEDREF T_UNKNOWNREF T_UNNAMEDREF T_DEREF
%token T_PROCEDURE T_LOCATION T_MEMREFHANDLE T_PTRASSIGNPAIRS T_MEMREFEXPRS
%token T_PARAMBINDPTRASSIGNPAIRS T_FORMALS T_CALLSITES T_CALLHANDLE
%token T_DEF "DEF" T_USE "USE" T_USEDEF "USEDEF" T_DEFUSE "DEFUSE"
%token T_ARROW "=>" T_LOCAL T_NOT T_ADDRESSOF T_ACCURACY
%token T_FULL T_PART T_T T_F

%type <prochandle>      prochandle
%type <symhandle>       symhandle
%type <stmthandle>      stmthandle
%type <callhandle>      callhandle
%type <memrefhandle>    memrefhandle
%type <memrefexpr>      memrefexpr namedref unknownref unnamedref deref
%type <defuse>          memreftype
%type <ptrassignpair>   ptrassignpair
%type <callsitelist>    callsiteslist;
%type <ptrassignlist>   ptrassignlist
%type <paramptrlist>    parambindptrassignpairslist;
%type <memrefexprslist> memreflist
%type <formallist>      formalslist;
%type <bval>            locality local notlocal;
%type <bval>            addressof
%type <bval>            accuracy

%start start

%%

 /* start grammer */
start:
    start T_PROCEDURE '=' '{' procedure '}'
    | start T_LOCATION '=' '{' location '}'
    | start T_MEMREFEXPRS '=' '{' memrefexprs '}'
    | start T_PTRASSIGNPAIRS '=' '{' ptrassignpairs '}'
    | start T_PARAMBINDPTRASSIGNPAIRS '=' '{' parambindptrassignpairs '}'
    | start T_FORMALS '=' '{' formals '}'
    | start T_CALLSITES '=' '{' callsites '}'
    | /* empty */
;




 /* procedure */
procedure:
    '<' prochandle ',' symhandle '>'
    {
        gSubIR->startProc(*$2, *$4);
    }
;




 /* location */
location:
    '<' symhandle ',' locality '>'
    {   
        OA::OA_ptr<OA::Location> loc;
        loc = new OA::NamedLoc(*$2, $4);
        gSubIR->addLocation(loc);
    }
;




 /* memrefexprs */
memrefexprs:
    stmthandle T_ARROW '[' memreflist ']'
    {  
        std::list<std::pair<OA::MemRefHandle,
          OA::OA_ptr<OA::MemRefExpr> > >::iterator mrefIter;
        for(mrefIter = $4->begin(); mrefIter != $4->end(); mrefIter++) {
            gSubIR->addMemRefExpr(*$1, mrefIter->first, mrefIter->second);
        }
    }
    ;

memreflist:
    memreflist memrefhandle T_ARROW memrefexpr
    { 
        OA::OA_ptr<OA::MemRefExpr> mre;
        mre = $4;
        std::pair<OA::MemRefHandle,
          OA::OA_ptr<OA::MemRefExpr> > p(*$2, mre);
        $1->push_back(p);
        $$ = $1;
    }
    | /* empty */
    { 
        $$ = new std::list<std::pair<OA::MemRefHandle,
                 OA::OA_ptr<OA::MemRefExpr> > >;
    }
;





 /* ptrassignpairs */
ptrassignpairs:
    stmthandle T_ARROW '[' ptrassignlist ']'
    {
        std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>,
          OA::OA_ptr<OA::MemRefExpr> > >::iterator ptrIter;
        for (ptrIter = $4->begin(); ptrIter != $4->end(); ptrIter++) {
            gSubIR->addPtrAssignPair(*$1, ptrIter->first, ptrIter->second);
        }
    }
    ;

ptrassignlist:
    ptrassignlist ptrassignpair
    {
	$1->push_back(*$2);
        $$ = $1;
        }
    | /* empty */
    {
        $$ = new std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>,
                                     OA::OA_ptr<OA::MemRefExpr> > >;
    }
;

ptrassignpair:
    '<' memrefexpr ',' memrefexpr '>'
    {
        OA::OA_ptr<OA::MemRefExpr> p1;
        OA::OA_ptr<OA::MemRefExpr> p2;
        p1 = $2;
        p2 = $4;
        std::pair<OA::OA_ptr<OA::MemRefExpr>,
                  OA::OA_ptr<OA::MemRefExpr> > *p =
            new std::pair<OA::OA_ptr<OA::MemRefExpr>,
                          OA::OA_ptr<OA::MemRefExpr> >(p1, p2);
        $$ = p;
    }




 /* callsites */
callsites:
    stmthandle T_ARROW '[' callsiteslist ']'
    {
        std::list<std::pair<OA::CallHandle,
            OA::OA_ptr<MemRefExpr> > >::iterator callsiteIter;
        for(callsiteIter = $4->begin(); callsiteIter != $4->end();
            callsiteIter++)
        {
            gSubIR->addCallSite(*$1, callsiteIter->first, 
                callsiteIter->second);
        }
    }
;

callsiteslist:
    callsiteslist callhandle T_ARROW namedref
    {
        OA::OA_ptr<MemRefExpr> ptr;
        ptr = (MemRefExpr *)$4;
        std::pair<OA::CallHandle, OA::OA_ptr<MemRefExpr> > p(*$2, ptr);
        $1->push_back(p);
        $$ = $1;
    }
    | /* empty */
    {
        $$ = new std::list<std::pair<OA::CallHandle,
            OA::OA_ptr<MemRefExpr> > >;
    }
;




 /* parambindptrassignpairs */
parambindptrassignpairs:
    callhandle T_ARROW '[' parambindptrassignpairslist ']'
    {
        std::list<std::pair<int,
            OA::OA_ptr<OA::MemRefExpr> > >::iterator ptrIter;

        for (ptrIter = $4->begin(); ptrIter != $4->end(); ptrIter++) {
            gSubIR->addParamBindPtrAssignPair(
                *$1, ptrIter->first, ptrIter->second);
        }
    }
;

parambindptrassignpairslist:
    parambindptrassignpairslist '<' T_INT_LITERAL ',' namedref '>'
    {
        OA::OA_ptr<OA::MemRefExpr> ref;
        ref = $5;
        std::pair<int, OA::OA_ptr<OA::MemRefExpr> > p($3, ref);
        $1->push_back(p);
        $$ = $1;
    }
    | /* empty */
    {

        $$ = new std::list<std::pair<int, OA::OA_ptr<OA::MemRefExpr> > >;
    }
    ;




 /* formals */
formals:
    '[' formalslist ']'
    {
        std::list<std::pair<int, OA::SymHandle> >::iterator frmIter;
        for (frmIter = $2->begin(); frmIter != $2->end(); frmIter++) {
           gSubIR->addFormal(frmIter->first, frmIter->second);
        }
    }
;

formalslist:
    formalslist '<' T_INT_LITERAL ',' symhandle '>'
    {
        std::pair<int, OA::SymHandle> p($3, *$5);
        $1->push_back(p);
        $$ = $1;
    }
    | /* empty */
    {
        $$ = new std::list<std::pair<int, OA::SymHandle> >;
    }
;






 /* memrefexpr */
memrefexpr:
    namedref
    {
        $$ = $1;
    }
    | unknownref
    {
        $$ = $1;
    }
    | unnamedref
    {
        $$ = $1;
    }
    | deref
    {
        $$ = $1;
    }
;

namedref:
    T_NAMEDREF '(' memreftype ',' symhandle ')'
    {
        $$ = new OA::NamedRef($3, *$5);
    }
    | T_NAMEDREF '(' memreftype ',' symhandle ',' addressof
      ',' accuracy ')'
    {
        $$ = new OA::NamedRef($7, $9, $3, *$5);
    }
;

unknownref:
    T_UNKNOWNREF '(' memreftype ')'
    {
        $$ = new OA::UnknownRef($3);
    }
    | T_UNKNOWNREF '(' memreftype ',' addressof ',' accuracy ')'
    {
        $$ = new OA::UnknownRef($5, $7, $3);
    }
;

unnamedref:
    T_UNNAMEDREF '(' memreftype ',' stmthandle ')'
    {
        $$ = new OA::UnnamedRef($3, *$5);
    }
    | T_UNNAMEDREF '(' memreftype ',' stmthandle ',' addressof 
      ',' accuracy ')'
    {
        $$ = new OA::UnnamedRef($7, $9, $3, *$5);
    }
;

deref:
    T_DEREF '(' memreftype ',' memrefexpr ')'
    {
        OA::OA_ptr<OA::MemRefExpr> mre;
        mre = $5;
        $$ = new OA::Deref($3, mre);
    }
    | T_DEREF '(' memreftype ',' memrefexpr ',' T_INT_LITERAL
      ',' addressof ',' accuracy ')'
    {
        OA::OA_ptr<OA::MemRefExpr> mre;
        mre = $5;
        $$ = new OA::Deref($9, $11, $3, mre, $7);
    }
;

memreftype:
    T_DEF
    {
        $$ = OA::MemRefExpr::DEF;
    }
    | T_USE
    {
        $$ = OA::MemRefExpr::USE;
    }
    | T_DEFUSE
    {
        $$ = OA::MemRefExpr::DEFUSE;
    }
    | T_USEDEF
    {
        $$ = OA::MemRefExpr::USEDEF;
    }
    ;




 /* handles */
callhandle:
    T_CALLHANDLE '(' T_STR_LITERAL ')'
    {
        $$ = new OA::CallHandle(gStrToHandle->getCallHandle($3));
        gSubIR->addCallString(*$$, $3);
    }
    ;

memrefhandle:
    T_MEMREFHANDLE '(' T_STR_LITERAL ')'
    {
        $$ = new OA::MemRefHandle(gStrToHandle->getMemRefHandle($3));
        gSubIR->addMemRefString(*$$, $3);
    }
    ;

prochandle:  
    T_PROCHANDLE '(' T_STR_LITERAL ')'
    {
        $$ = new OA::ProcHandle(gStrToHandle->getProcHandle($3));
        gSubIR->addProcString(*$$,$3);
    }
;

symhandle:
    T_SYMHANDLE '(' T_STR_LITERAL ')'
    {
        $$ = new OA::SymHandle(gStrToHandle->getSymHandle($3));
        gSubIR->addSymString(*$$,$3);
    }
    ;

stmthandle:
    T_STMTHANDLE '(' T_STR_LITERAL ')'
    {
        $$ = new OA::StmtHandle(gStrToHandle->getStmtHandle($3));
        gSubIR->addStmtString(*$$, $3);
        gSubIR->addStmt(*$$);
    }
    ;



locality:
    local | notlocal
    {
        $$ = $1;
    };

local:
    T_LOCAL
    {
        $$ = true;
    };

notlocal:
    T_NOT T_LOCAL
    {
        $$ = false;
    }
    ;

addressof:
      T_T                 { $$ = true; }
    | T_ADDRESSOF '=' T_T { $$ = true; }
    | T_F                 { $$ = false; }
    | T_ADDRESSOF '=' T_F { $$ = false; }
    ;

accuracy:
      T_FULL                { $$ = true; }
    | T_ACCURACY '=' T_FULL { $$ = true; }
    | T_PART                { $$ = false; }
    | T_ACCURACY '=' T_PART { $$ = false; }
    ;

%%
#include <stdio.h>
void yyerror (char *s)  /* Called by yyparse on error */
{
    fprintf(stderr, "%s at '%s', line %d\n", s, oa_text, oa_lineno);
    exit(-1);
}

