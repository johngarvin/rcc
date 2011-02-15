/*
 CmdParams.c */
#include "CmdParams.h"

CmdParams* CmdParams_ctor(int printVals)
/*------------------------------------------------------------*//*!
  Allocate space for a CmdParams object.

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    CmdParams* thee = (CmdParams*)malloc(sizeof(CmdParams));
    thee->numParams = 0;
    thee->printVals = printVals;

    return thee;
}

void CmdParams_dtor(CmdParams **cmdparamPtr)
/*------------------------------------------------------------*//*!
  Deallocate space for a CmdParams object.

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    if (*cmdparamPtr != NULL) {
        free(*cmdparamPtr);
        *cmdparamPtr = NULL;
    }
}

void CmdParams_describeEnumParam(CmdParams *thee, char *paramName,
        char flag, int optional,
        char *helpstr, 
        EnumStringPair possibleStrings[], int numPairs,
        int defaultVal)
/*------------------------------------------------------------*//*!
  Describe a number command line parameter.  Will be of the
  form "-f string".
  Enum parameters have an enumerated type which is associated
  with strings.  

  \param paramName  name of parameter if want to print it out
  \param flag   The flag character
  \param optional   Is this command line parameter optional
  \param helpstr    String describing the command line parameter
  \param possibleStrings    Array of {enumval,string} pairs.
  \param numPairs   number of pairs in possibleStrings array
  \param defaultVal Value of enumerated type which is default

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    int i;
    struct cmd_param * cps;

    assert(thee->numParams < MAXCMDPARAMS);

    /* get array of command parameter descriptions */
    cps = thee->cmd_params;

    /* Put this command line param next in the array */
    strcpy(cps[thee->numParams].paramName, paramName);
    cps[thee->numParams].paramType = paramType_ENUM;
    cps[thee->numParams].flagChar = flag;
    cps[thee->numParams].optional = optional;
    cps[thee->numParams].actualVal = defaultVal;
    cps[thee->numParams].numPairs = numPairs;
    strcpy(cps[thee->numParams].helpstr, helpstr);
    for (i=0; i<numPairs; i++) {
        strcpy(cps[thee->numParams].possibleStrings[i].string, 
               possibleStrings[i].string);
        cps[thee->numParams].possibleStrings[i].enumVal 
               = possibleStrings[i].enumVal;
    }

    thee->numParams++;
}


void CmdParams_describeStringParam(CmdParams *thee, char *paramName,
        char flag, int optional,
        char *helpstr, char *defaultString)
/*------------------------------------------------------------*//*!
  Describe a number command line parameter.  Will be of the
  form "-f string".

  \param paramName  name of parameter if want to print it out
  \param flag   The flag character
  \param optional   Is this command line parameter optional
  \param helpstr    String describing the command line parameter
  \param defaultString The default string for the parameter

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    struct cmd_param * cps;

    assert(thee->numParams < MAXCMDPARAMS);

    /* get array of command parameter descriptions */
    cps = thee->cmd_params;

    /* Put this command line param next in the array */
    strcpy(cps[thee->numParams].paramName, paramName);
    cps[thee->numParams].paramType = paramType_STRING;
    cps[thee->numParams].flagChar = flag;
    cps[thee->numParams].optional = optional;
    strcpy(cps[thee->numParams].helpstr, helpstr);
    strcpy(cps[thee->numParams].stringVal, defaultString);

    thee->numParams++;
}

void CmdParams_describeNumParam(CmdParams *thee, char *paramName,
        char flag, int optional,
        char *helpstr, 
        int startRange, int endRange, int defaultVal)
/*------------------------------------------------------------*//*!
  Describe a number command line parameter.  Will be of the
  form "-f num".

  \param paramName  name of parameter if want to print it out
  \param flag   The flag character
  \param optional   Is this command line parameter optional
  \param helpstr    String describing the command line parameter
  \param startRange The value of the param can be in the range
                    [startRange to endRange]
  \param endRange
  \param defaultVal The default value for the parameter

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    struct cmd_param * cps;

    assert(thee->numParams < MAXCMDPARAMS);

    /* get array of command parameter descriptions */
    cps = thee->cmd_params;

    /* Put this command line param next in the array */
    strcpy(cps[thee->numParams].paramName, paramName);
    cps[thee->numParams].paramType = paramType_INT;
    cps[thee->numParams].flagChar = flag;
    cps[thee->numParams].optional = optional;
    strcpy(cps[thee->numParams].helpstr, helpstr);
    cps[thee->numParams].startRange = startRange;
    cps[thee->numParams].endRange = endRange;
    cps[thee->numParams].actualVal = defaultVal;

    thee->numParams++;
}

void CmdParams_setValue(CmdParams* thee, char whichFlag, int val)
/*------------------------------------------------------------*//*!
  Set the value for the given flag.

  \author Michelle Strout 10/1/02
*//*--------------------------------------------------------------*/
{
    int i, j;
    struct cmd_param *cps;

    /* get pointer to the array of cmd_params */
    cps = thee->cmd_params;

    /* create a string with all of the command line flags */
    for (i=0; i<thee->numParams; i++) {
        if (cps[i].flagChar == whichFlag) {
            cps[i].actualVal = val;
            break;
        }
    }
}


int CmdParams_getValue(CmdParams* thee, char whichFlag)
/*------------------------------------------------------------*//*!
  Return the value for the given flag.

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    int i, j, retval=0;
    struct cmd_param *cps;

    /* get pointer to the array of cmd_params */
    cps = thee->cmd_params;

    /* create a string with all of the command line flags */
    for (i=0; i<thee->numParams; i++) {
        if (cps[i].flagChar == whichFlag) {
            retval = cps[i].actualVal;
            break;
        }
    }

    return retval;
}

char* CmdParams_getString(CmdParams* thee, char whichFlag)
/*------------------------------------------------------------*//*!
  Return the string value for the given flag.

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    int i, j;
    char * retval = NULL;
    struct cmd_param *cps;

    /* get pointer to the array of cmd_params */
    cps = thee->cmd_params;

    /* create a string with all of the command line flags */
    for (i=0; i<thee->numParams; i++) {
        if (cps[i].flagChar == whichFlag) {
            switch (cps[i].paramType) {
                case paramType_STRING:
                    retval = cps[i].stringVal;
                    break;
                case paramType_ENUM:
                    for (j=0; j<cps[i].numPairs; j++) {
                        if (cps[i].possibleStrings[j].enumVal 
                            == cps[i].actualVal)
                        {
                            retval = cps[i].possibleStrings[j].string;
                        }
                    }
                    break;
            }
            break;
        }
    }
    assert(retval!=NULL);
    return retval;
}

void CmdParams_printHelp(CmdParams *thee)
/*------------------------------------------------------------*//*!
  Print a help message for all command line parameters.

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    int i, j, defaultEnumIndex;
    struct cmd_param *cps;

    /* get pointer to the array of cmd_params */
    cps = thee->cmd_params;

    printf("\nCommand line parameter help\n");

    /* create a string with all of the command line flags */
    for (i=0; i<thee->numParams; i++) {
        printf("\n\t-%c\t%s\n",
               cps[i].flagChar, cps[i].helpstr );
        switch(cps[i].paramType) {
            case paramType_INT:
                printf("\t\tDefault: %d, Range: [%d to %d]\n",
                       cps[i].actualVal, cps[i].startRange, cps[i].endRange);
                break;
            case paramType_STRING:
                printf("\t\tDefault: %s\n", cps[i].stringVal);
                break;
            case paramType_ENUM:
                defaultEnumIndex = -1;
                /* find string for enumval and print out possibles */
                printf("\t\tPossible values: ");
                for (j=0; j<cps[i].numPairs; j++) {
                    if (cps[i].possibleStrings[j].enumVal 
                        == cps[i].actualVal)
                    {
                        defaultEnumIndex = j;
                    }
                    printf("%s ", cps[i].possibleStrings[j].string);
                }
                assert(defaultEnumIndex >= 0);
                printf("\n\t\tDefault: %s\n", 
                       cps[i].possibleStrings[defaultEnumIndex].string);
                break;
        } /* end switch */
    }
}

void CmdParams_parseParams(CmdParams *thee, int argc, char **argv)
/*------------------------------------------------------------*//*!
  Parse argc and argv for command line parameters.
  Expects -f val for each possible command line parameter.
  The one exception is -h or --help, if either of those are at
  the beginning then help will be printed and the program 
  will be exited.
  For other parameters, if thee->printVals is true then
  the value of the parameter will be printed to stdout as it is parsed.

  \param  argc  number of command line params
  \param  argv  array of string tokens from command line

  \author Michelle Strout 9/28/02
*//*--------------------------------------------------------------*/
{
    int i,c,j;
    char flagstr[MAXCMDPARAMS*2+1];
    char tempstr[2+1];
    struct cmd_param *cps;
    int flagFound, foundEnum;

    /* first check if this is a request for help */
    if (argc > 1) {
        if (strcmp(argv[1],"--help")==0 ||
            strcmp(argv[1],"-h")==0 ) 
        {
            CmdParams_printHelp(thee);
            exit(0);
        }
    }
     
    /* empty this string */
    flagstr[0] = '\0';

    /* get pointer to the array of cmd_params */
    cps = thee->cmd_params;

    /*
     * optind is an external variable that getopt uses to remember
     * it's place from previous call.  resetting to 1 allows a new
     * pp_argc and pp_argv to be parsed of each call to this routine
     */
    optind=1;
    /*oldoptind = 0;*/  /* needed for blue */

    /* create a string with all of the command line flags */
    for (i=0; i<thee->numParams; i++) {
        sprintf(tempstr, "%c:", cps[i].flagChar );
        strcat(flagstr, tempstr);
    }

    /* parse command line parameters, returns EOF=-1 when done */
    while ( (c=getopt(argc,argv, flagstr)) != -1) 
    {
        #if (defined(MYDEBUG) || defined(MYDEBUG_VERBOSE) )
            printf("CmdParams_parseParams: c=%c, optarg=%s\n", c, optarg);
            printf("       and: pp_argc= %d\n", argc);
            printf("       and: optind=%d\n",optind);
        #endif

        /*if (optind == oldoptind) { break; } */ /* needed for blue */
        /*oldoptind = optind; */ /* needed for blue */

        flagFound = 0;
        for (i=0; i<thee->numParams; i++) {
            if (c == cps[i].flagChar ) {
                flagFound = 1;

                /* process the param based on its type */
                switch(cps[i].paramType) {
                    case paramType_STRING: 
                        strcpy(cps[i].stringVal, optarg); 
                        if (thee->printVals) {
                            printf("CmdParams_parseParams: %s = %s\n",
                                   cps[i].paramName, cps[i].stringVal);
                        }
                        break;

                    case paramType_INT: 
                        cps[i].actualVal = atoi(optarg); 
                        if (thee->printVals) {
                            printf("CmdParams_parseParams: %s = %d\n",
                                   cps[i].paramName, cps[i].actualVal);
                        }
                        if ((cps[i].actualVal < cps[i].startRange) || 
                            (cps[i].actualVal > cps[i].endRange) )
                        {
                            fprintf(stderr,"Error: -%c %d is out of range\n",
                                    c, cps[i].actualVal );
                            exit(0);
                        }
                        break;

                    case paramType_ENUM: 
                        foundEnum = 0;
                        for (j = 0; j<cps[i].numPairs; j++) {
                            if (strcmp(optarg,
                                       cps[i].possibleStrings[j].string)==0)
                            {
                                foundEnum = 1;
                                cps[i].actualVal 
                                    = cps[i].possibleStrings[j].enumVal;
                            }
                        }
                        if (thee->printVals) {
                            printf("CmdParams_parseParams: %s = %s\n",
                                   cps[i].paramName, optarg);
                        }
                        if (!foundEnum) {
                            fprintf(stderr, "Error: "
                                "-%c %s is not in list of possible strings\n",
                                c, optarg );
                            exit(0);
                        }
                        break;
                } /* switch */
            }  /* if found flag */
        } /* loop over cmd_params to find flag */
        if (!flagFound) {
            fprintf(stderr, "Error: -%c is not a known flag\n", c);
            exit(0);
        }
    }
}

