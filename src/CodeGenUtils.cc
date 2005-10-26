#include <cstdarg>
#include <support/StringUtils.h>
#include <CodeGenUtils.h>
#include <CheckProtect.h>


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

string emit_assign(const string lhs, const string rhs, Protection resultProtected) 
{
  string assign = lhs + " = " + rhs;
  if (resultProtected == Protected) 
    assign = emit_call1("PROTECT", assign) + eos;
  else assign += eos;
  return assign; 
}

string emit_prot_assign(const string lhs, const string rhs)
{
  return emit_assign(lhs, rhs, Protected); 
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

string emit_call3(string fname, const string arg1, const string arg2, const string arg3)
{
  return emit_call_internal(fname, &arg1, &arg2, &arg3, NULL);
} 

string emit_call4(string fname, const string arg1, const string arg2, const string arg3, const string arg4)
{
  return emit_call_internal(fname, &arg1, &arg2, &arg3, &arg4, NULL);
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

#ifdef CHECK_PROTECT
  static int protcnt = 0; 
#endif

string emit_in_braces(string code, bool balanced) {
  string result;
  result = "{\n";
#ifdef CHECK_PROTECT
  if (balanced) {
    result += indent("int prot" + i_to_s(protcnt) + " = R_PPStackTop" + eos);
  }
#endif
  result += indent(code);
#ifdef CHECK_PROTECT
  if (balanced) {
    result += indent("assert(prot" + i_to_s(protcnt++) + " == R_PPStackTop)"+ eos);
  }
#endif
  result += "}\n";
  return result;
}

string emit_unprotect(string code) {
  return emit_call1("UNPROTECT_PTR", code) + eos;
}

string emit_decl(string var) {
  return "SEXP " + var + eos;
}

string emit_static_decl(string var) {
  return "static " + emit_decl(var);
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
