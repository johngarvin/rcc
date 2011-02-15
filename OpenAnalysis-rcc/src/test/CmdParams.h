/* CmdParams.h */
/*****************************************************************/
/*!
\file

 The CmdParams class holds information about all of the command
 line parameters.
 See sparseDriver_parseParams.c and .h to see example usage.

 Notes:
 -optional isn't actually implemented.
 -if a string is passed as a parameter for a num param, the value
  is just set to 0

 example usage:
 <pre>
typedef enum {
    test_a,
    test_b,
    test_c
} test_type;

EnumStringPair cPairs[] = { {test_a,"test_a"},{test_b,"test_b"},
                          {test_c,"test_c"} };
int num_cPairs = 3;


int main(int argc, char **argv) {
    char *aparam;
    int bparam;
    test_type cparam;
    char *cparamString;

    CmdParams *cmdparams = CmdParams_ctor(1);
    
    CmdParams_describeStringParam(cmdparams, "aparamName",
            'a', 1, "testing a string param", "default_a");
    CmdParams_describeNumParam(cmdparams, "bparamName",
            'b', 1, "testing a number param", 0, 4, 2);
    CmdParams_describeEnumParam(cmdparams, "cparamName",
            'c', 1, "testing an enum param",
            cPairs, num_cPairs, test_b);

    CmdParams_parseParams(cmdparams, argc, argv);

    aparam = CmdParams_getString(cmdparams, 'a');
    bparam = CmdParams_getValue(cmdparams, 'b');
    cparam = CmdParams_getValue(cmdparams, 'c');
    cparamString = CmdParams_getString(cmdparams, 'c');
 </pre>

*//*******************************************************************/
#ifndef _CMDPARAMS_H
#define _CMDPARAMS_H

#define MAXHELPSTRING 256
#define MAXPOSSVALSTRING 256
#define MAXPOSSVALS 20
#define MAXCMDPARAMS 40

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef enum {
    paramType_STRING,
    paramType_INT,
    paramType_ENUM
} paramType_type;

typedef struct enum_string_pair {
    int enumVal;
    char string[MAXPOSSVALSTRING];
} EnumStringPair;

struct cmd_param {
    char paramName[MAXPOSSVALSTRING];
    char flagChar;
    char helpstr[MAXHELPSTRING];
    int optional;
    paramType_type paramType;

    /* used for paramType_INT and paramType_ENUM */
    /* if ENUM then to find string look for actualVal
     * in EnumStringPairs */
    int actualVal; 

    /* used for paramType_ENUM */
    /* array of possible enum values and their
     * associated strings */
    int numPairs;
    EnumStringPair possibleStrings[MAXPOSSVALS];  

    /* used for paramType_INT */
    int startRange; int endRange; /* range for int params */

    /* used for paramType_STRING */
    char stringVal[MAXPOSSVALSTRING]; 
};         

typedef struct cmd_param_manager {
    int printVals;      /* whether to printVals while parsing */
    int numParams;    
    struct cmd_param cmd_params[MAXCMDPARAMS];
} CmdParams;

#define MAXNUMARGS 1+2*MAXCMDPARAMS 
/* allows for dummy arg[0], and MAXCMDPARAMS flag/value pairs */

CmdParams* CmdParams_ctor(int printVals);
void CmdParams_dtor(CmdParams** cmdparamPtr);

void CmdParams_describeNumParam(CmdParams *thee, char *paramName,
        char flag, int optional,
        char *helpstr, 
        int startRange, int endRange, int defaultVal);

void CmdParams_describeStringParam(CmdParams *thee, char *paramName,
        char flag, int optional,
        char *helpstr, char *defaultString);

void CmdParams_describeEnumParam(CmdParams *thee, char *paramName,
        char flag, int optional,
        char *helpstr, 
        EnumStringPair possibleStrings[], int numPairs,
        int defaultVal);

void CmdParams_printHelp(CmdParams *cmdparams);

void CmdParams_parseParams(CmdParams *cmdparams, int argc, char **argv);

void CmdParams_setValue(CmdParams* thee, char whichFlag, int val);
int CmdParams_getValue(CmdParams* thee, char whichFlag);

char* CmdParams_getString(CmdParams* thee, char whichFlag);

#endif

