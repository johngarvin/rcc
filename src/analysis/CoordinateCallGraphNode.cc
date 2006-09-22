#include <support/DumpMacros.h>
#include <support/RccError.h>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/DefVar.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/LibraryCallGraphNode.h>
#include <analysis/UnknownValueCallGraphNode.h>
#include <analysis/SymbolTable.h>
#include <analysis/Utils.h>
#include <analysis/VarInfo.h>

#include "CoordinateCallGraphNode.h"

namespace RAnnot {

  CallGraphAnnotationMap::CoordinateCallGraphNode::CoordinateCallGraphNode(SEXP name, SEXP scope)
    : m_name(name), m_scope(scope)
  {}

  CallGraphAnnotationMap::CoordinateCallGraphNode::~CoordinateCallGraphNode()
  {}

  const SEXP CallGraphAnnotationMap::CoordinateCallGraphNode::get_name() const {
    return m_name;
  }

  const SEXP CallGraphAnnotationMap::CoordinateCallGraphNode::get_scope() const {
    return m_scope;
  }

  const OA::IRHandle CallGraphAnnotationMap::CoordinateCallGraphNode::get_handle() const {
    return reinterpret_cast<OA::irhandle_t>(this);
  }

  void CallGraphAnnotationMap::CoordinateCallGraphNode::
  compute(CallGraphAnnotationMap::NodeListT & worklist,
	  CallGraphAnnotationMap::NodeSetT & visited) const
  {
    CallGraphAnnotationMap * cg = CallGraphAnnotationMap::get_instance();
    SymbolTable * st = getProperty(SymbolTable, m_scope);

    SymbolTable::const_iterator it = st->find(m_name);
    if (it == st->end()) {
      // Name not found in SymbolTable. This means either an unbound
      // variable or a predefined (library or built-in) function.
      SEXP value = Rf_findFun(m_name, R_GlobalEnv);
      if (value == R_UnboundValue) {
	rcc_error("Unbound variable " + var_name(m_name));
      } else {
	cg->add_edge(this, cg->make_library_node(m_name, value));
	// but don't add library fun to worklist
      }
    } else {  // symbol is defined at least once in this scope
      VarInfo * vi = it->second;
      VarInfo::const_iterator var;
      for (var = vi->beginDefs(); var != vi->endDefs(); ++var) {
	DefVar * def = *var;
	if (is_fundef(CAR(def->getRhs_c()))) {
	  // def is of the form _ <- function(...)
	  FundefCallGraphNode * node = cg->make_fundef_node(CAR(def->getRhs_c()));
	  cg->add_edge(this, node);
	  if (visited.find(node) == visited.end()) {
	    worklist.push_back(node);
	  }
	} else {
	  // RHS of def is a non-fundef; we don't know the function value
	  cg->add_edge(this, cg->make_unknown_value_node());
	  // TODO: if it's a variable, then we might know how to handle
	}
      }
    }
  }

  void CallGraphAnnotationMap::CoordinateCallGraphNode::
  get_call_bindings(CallGraphAnnotationMap::NodeListT & worklist,
		    CallGraphAnnotationMap::NodeSetT & visited,
		    CallGraphAnnotation * ann) const
  {
    CallGraphAnnotationMap * cg = CallGraphAnnotationMap::get_instance();
    // TODO: remove cast when maps are parametrized
    CallGraphInfo * info = cg->get_edges(this);
    for (CallGraphInfo::const_iterator edge = info->begin_calls_out();
	 edge != info->end_calls_out();
	 ++edge) {
      worklist.push_back((*edge)->get_sink());
    }
  }

  void CallGraphAnnotationMap::CoordinateCallGraphNode::
  dump(std::ostream & os) const {
    FuncInfo * finfo = getProperty(FuncInfo, m_scope);
    SEXP scope_first_name = finfo->get_first_name();

    beginObjDump(os, CoordinateCallGraphNode);
    dumpPtr(os, this);
    dumpSEXP(os, m_name);
    dumpSEXP(os, scope_first_name);
    endObjDump(os, CoordinateCallGraphNode);
  }

  void CallGraphAnnotationMap::CoordinateCallGraphNode::
  dump_string(std::ostream & os) const {
    FuncInfo * finfo = getProperty(FuncInfo, m_scope);
    std::string first_name = CHAR(PRINTNAME(m_name));
    std::string scope_first_name = CHAR(PRINTNAME(finfo->get_first_name()));
    beginObjDump(os, CoordinateCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, first_name);
    dumpString(os, scope_first_name);
    endObjDump(os, CoordinateCallGraphNode);
  }

} // end namespace RAnnot
