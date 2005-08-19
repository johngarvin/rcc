#include <cstdarg>
#include "codegen.h"

using namespace std;



//******************************************************************************
// local declarations 
//******************************************************************************


// end of statement string
const string eos = ";\n";



//******************************************************************************
// forward declarations 
//******************************************************************************


static string emit_call_internal(string fname, ...);
static string emit_call_internal_va(string fname, va_list args); 



//******************************************************************************
// interface functions 
//******************************************************************************

string emit_logical_if(const string expn) 
{
  string ifexpn;
  ifexpn = "if (" + expn + ") ";
  return ifexpn; 
}
	
string emit_logical_if_stmt(const string expn, const string stmt) 
{
  string ifstmt;
  if (expn.length() > 0) ifstmt += emit_logical_if(expn);
  ifstmt += stmt + eos;
  return ifstmt;
}

string emit_assign(string lhs, string rhs) 
{
  string assign = lhs + " = " + rhs + eos;
  return assign; 
}

string emit_call0(const string fname)
{
  return emit_call_internal(fname, NULL);
} 

string emit_call1(const string fname, const string arg1)
{
  return emit_call_internal(fname, &arg1, NULL);
} 

string emit_call2(string fname, const string arg1, const string arg2)
{
  return emit_call_internal(fname, &arg1, &arg2, NULL);
} 

#if 0
string emit_call_stmt(const string fname, ...)
{
  va_list args;
  va_start (args, fname);
  string cstmt = emit_call_internal_va(fname, args) + eos;
  va_end(args);
  return cstmt;
}
#endif

string emit_in_braces(string code) {
  return "{\n" + indent(code) + "}\n";
}


//******************************************************************************
// private functions
//******************************************************************************
static string emit_call_internal(string fname, ...) 
{
  va_list args;
  va_start (args, fname);
  string cstmt = emit_call_internal_va(fname, args); 
  va_end(args);
  return cstmt;
}

static string emit_call_internal_va(string fname, va_list args) 
{
    string call;
    string *arg;

    // open the call
    call = fname + "(";

    // first argument
    arg = va_arg(args, string *);
    if (arg) call += *arg;

    // subsequent arguments preceded by a comma
    arg = va_arg(args, string *);
    while (arg != NULL) {
      call += ", " + *arg;
      arg = va_arg(args, string *);
    }

    // finish the call
    call += ")";
    return call;
}


#ifdef __codegen_test_

#include <iostream>
int main()
{
	string foo = "foo";
	string out1 = emit_logical_if(foo);
	cout << out1 << endl;
	string out = emit_call2(foo,"bar","2");
	cout << out << endl;
	return 0;
}

#endif
