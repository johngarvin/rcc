/*! \file
  
  \brief OA utilities

  \authors Nathan Tallent, Andy Stone, Eric Eastman
  \version $Id: Util.hpp,v 1.1 2004/12/13 20:04:49 ntallent Exp $

  Copyright (c) 2002-2005, Rice University <br>
  Copyright (c) 2004-2005, University of Chicago <br>
  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef OA_Util_H
#define OA_Util_H

#include <cstdlib>
#include <string>

// Could use templates within OA namespace

#ifndef OA_MIN
# define OA_MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef OA_MAX
# define OA_MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/* 
Debugging macro

OA_DEBUG_CTRL_MACRO(ModuleNameListDEBUG, DeBugDEBUG)

This .hpp file sets up the macro OA_DEBUG_CTRL_MACRO.  This macro 
at runtime retrieves the ':' colon-separated  list from the environment 
variable OA_DEBUG and compares this list to the list of ':' separate 
names passed to it in ModuleNameListDEBUG.  If it finds an exact name 
match for any of the elements it sets the bool variable true, otherwise 
it set it false.  This variable can then be used to control printing 
of debug information.  

OA_DEBUG_CTRL_MACRO was written as a macro so it would always be 
inlined.  The only library call the macro uses is getenv(). All 
the string compares are done inline without any library calls, 
so the code is a bit ugly, but should be safe. 

String case does matter, so I recommend using upper-case 
charters for both the call and in the environment OA_DEBUG variable!

No memory is allocated other then a couple pointers taken from the stack. 
Everything is wrapped within a { } pair to keep all the variables in a 
local scope.

To use, within the program code (must be executed), declare a bool variable
and call the macro:

	bool debug;
     	OA_DEBUG_CTRL ( "THISMODNAME:GROUPMODNAME:GLOBALMODNAME:ALL", debug) ;

Where the text between the "" is one or more names, separated with ':' 
that will be compared to the environment variable returned with 
getenv("OA_DEBUG"). The parameter "debug" is the name of the bool variable that 
will be set to true of false depending on if the environment variable is 
matched. 

	WARNING: This is a macro, so do not do:

     	OA_DEBUG_CTRL ( "MYMODULE:ALL", bool debug) ;

	As "bool debug" will get expanded more then once 
	in the macro and your code will not compile

The reason ModuleNameListDEBUG takes a list is so you can place 
a specific entry for this one module, and then add in zero or 
more group names, allowing you to turn on debugging for a group 
of modules by only setting a single environment variable

When you want to output debugging information, use something like this 
within your code:

 if (debug) {
          std::cout << "This is a debug message << std::endl ;
 }

To turn debugging on, under the bash shell use the command:

    export OA_DEBUG=FREDSMODULE:THISMODULENAME:SOMEOTHERMODUAL

and then run your code.

To turn debugging off, issued the command:

    export OA_DEBUG=

E. Eastman
Feb 2006

*/
#define OA_GLOBAL_DEBUG_ON

#if defined OA_GLOBAL_DEBUG_ON

#define OA_DEBUG_CTRL_MACRO(ModuleNameListDEBUG, DeBugDEBUG)\
    {\
        DeBugDEBUG = false;\
        const char *ept = getenv("OA_DEBUG");\
        if( ept != NULL ) {\
            while (*ept == ':') {++ept;}\
            const char *ept1 = ept;\
            const char *mpt1;\
            const char *mpt;\
            mpt1 = ModuleNameListDEBUG;\
            while (*mpt1 == ':') {++mpt1;}\
            mpt = mpt1;\
            while(*mpt1 != '\0') {\
                while(*ept != '\0'){\
                    if( *ept == *mpt ){\
                        *ept++;\
                        *mpt++;\
                        if ((*ept == '\0' || *ept == ':' ) && \
                          (*mpt == '\0' || *mpt == ':')) {\
                            DeBugDEBUG = true;\
                            break;\
                        }\
                    }\
                    else {\
                        mpt = mpt1;\
                        while ((*ept != ':') && (*ept != '\0')){++ept;}\
                        while (*ept == ':') {++ept;}\
                    }\
                }\
                if (DeBugDEBUG) {break;}\
                while( (*mpt1 != ':') && (*mpt1 != '\0') ){++mpt1;}\
                while (*mpt1 == ':') {++mpt1;}\
                mpt = mpt1; ept = ept1;\
            }\
        }\
    }\

#else
/* 
*    If OA_GLOBAL_DEBUG_ON is not set, we still need to define DeBugDEBUG  
*/ 
#define OA_DEBUG_CTRL_MACRO(ModuleNameListDEBUG, DeBugDEBUG)\
    DeBugDEBUG = false;\

#endif


#endif
