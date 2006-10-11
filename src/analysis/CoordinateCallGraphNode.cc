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

  CallGraphAnnotationMap::CoordinateCallGraphNode::CoordinateCallGraphNode(const SEXP name, const LexicalScope * scope)
    : m_name(name), m_scope(scope)
  {}

  CallGraphAnnotationMap::CoordinateCallGraphNode::~CoordinateCallGraphNode()
  {}

  const SEXP CallGraphAnnotationMap::CoordinateCallGraphNode::get_name() const {
    return m_name;
  }

  const LexicalScope * CallGraphAnnotationMap::CoordinateCallGraphNode::get_scope() const {
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

    if (const InternalLexicalScope * scope = dynamic_cast<const InternalLexicalScope *>(m_scope)) {
      SEXP value = Rf_findFunUnboundOK(m_name, R_GlobalEnv, TRUE);
      assert(value != R_UnboundValue);
      cg->add_edge(this, cg->make_library_node(m_name, value));
      // but don't add library fun to worklist
    } else if (const FundefLexicalScope * scope = dynamic_cast<const FundefLexicalScope *>(m_scope)) {
      SymbolTable * st = getProperty(SymbolTable, scope->get_fundef());
      SymbolTable::const_iterator it = st->find(m_name);
      if (it == st->end()) {
	// Name not found in SymbolTable: unbound variable
	rcc_error("Unbound variable " + var_name(m_name));
      } else {  // symbol is defined at least once in this scope
	VarInfo * vi = it->second;
	VarInfo::const_iterator var;
	for (var = vi->beginDefs(); var != vi->endDefs(); ++var) {
	  DefVar * def = *var;
	  if (is_fundef(CAR(def->getRhs_c()))) {
	    // def is of the form _ <- function(...)
	    FundefCallGraphNode * node = cg->make_fundef_node(CAR(def->getRhs_c()));
	    cg->add_edge(this, node);
	  } else {
	    // RHS of def is a non-fundef; we don't know the function value
	    cg->add_edge(this, cg->make_unknown_value_node());
	    // TODO: if it's a variable, then we might know how to handle
	  }
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
    dump_string(os);
  }

  void CallGraphAnnotationMap::CoordinateCallGraphNode::
  dump_string(std::ostream & os) const {
    const std::string name = var_name(m_name);
    const std::string scope_name = get_scope()->get_name();
    beginObjDump(os, CoordinateCallGraphNode);
    dumpVar(os, get_id());
    dumpString(os, name);
    dumpString(os, scope_name);
    endObjDump(os, CoordinateCallGraphNode);
  }

} // end namespace RAnnot
