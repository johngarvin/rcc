#include <string>
#include <util.h>

//------------------------------------------------------------------------------
// emit conditional: if(expn) 
//------------------------------------------------------------------------------
std::string emit_logical_if(const std::string expn); 


//------------------------------------------------------------------------------
// emit conditional: if(expn) stmt;
//------------------------------------------------------------------------------
std::string emit_logical_if_stmt(const std::string expn, 
				 const std::string stmt); 


//------------------------------------------------------------------------------
// emit assignment: lhs = rhs;
//------------------------------------------------------------------------------
std::string emit_assign(const std::string lhs, const std::string rhs); 


//------------------------------------------------------------------------------
// emit call: fname(...)
//    note: sigh. arguments cannot be specified by varargs list for strings
//------------------------------------------------------------------------------
std::string emit_call0(const std::string fname);

std::string emit_call1(const std::string fname, const std::string arg1);

std::string emit_call2(std::string fname, const std::string arg1, 
		       const std::string arg2);


//------------------------------------------------------------------------------
// emit call statement: fname(arg1, arg2, ...);
//    note: arguments are specified as varargs list
//          all will be interpreted as strings 
//    note: statement terminated 
//------------------------------------------------------------------------------
// std::string emit_call_stmt(const std::string fname, ...);

std::string emit_in_braces(std::string code);
