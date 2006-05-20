#include <list>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/CallGraphNode.h>
#include <analysis/CallGraphInfo.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/LibraryCallGraphNode.h>
#include <analysis/CoordinateCallGraphNode.h>
#include <analysis/CallSiteCallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/SymbolTable.h>
#include <analysis/VarBinding.h>
#include <analysis/VarInfo.h>

#include <support/DumpMacros.h>
#include <support/RccError.h>

#include "CallGraphAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef CallGraphAnnotationMap::MyKeyT MyKeyT;
typedef CallGraphAnnotationMap::MyMappedT MyMappedT;
typedef CallGraphAnnotationMap::iterator iterator;
typedef CallGraphAnnotationMap::const_iterator const_iterator;
  
// ----- constructor, destructor -----

CallGraphAnnotationMap::CallGraphAnnotationMap()
  : m_computed(false), m_node_map(), m_edge_set(), m_traversed_map(),
    m_fundef_map(), m_library_map(), m_coord_map(), m_call_site_map()
{}
  
CallGraphAnnotationMap::~CallGraphAnnotationMap() {
  std::map<const CallGraphNode *, CallGraphInfo *>::const_iterator i;
  
  // delete CallGraphInfo objects
  for(i = m_node_map.begin(); i != m_node_map.end(); ++i) {
    delete(i->second);
  }
  
  // delete edges
  std::set<const CallGraphEdge *>::const_iterator j;
  for(j = m_edge_set.begin(); j != m_edge_set.end(); ++j) {
    delete(*j);
  }
}  
  
// ----- singleton pattern -----

CallGraphAnnotationMap * CallGraphAnnotationMap::get_instance() {
  if (m_instance == 0) {
    create();
  }
  return m_instance;
}
  
PropertyHndlT CallGraphAnnotationMap::handle() {
  if (m_instance == 0) {
    create();
  }
  return m_handle;
}
  
void CallGraphAnnotationMap::create() {
  m_instance = new CallGraphAnnotationMap();
  analysisResults.add(m_handle, m_instance);
}
  
CallGraphAnnotationMap * CallGraphAnnotationMap::m_instance = 0;
PropertyHndlT CallGraphAnnotationMap::m_handle = "CallGraph";
  
//  ----- demand-driven analysis ----- 

// Subscripting is here temporarily to allow PutProperty -->
// PropertySet::insert to work right.
// FIXME: delete this when fully refactored to disallow insertion from outside.
MyMappedT & CallGraphAnnotationMap::operator[](const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  return m_traversed_map[k];
}

// Perform the computation if necessary and returns the requested
// data.
MyMappedT CallGraphAnnotationMap::get(const MyKeyT & k) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  // after computing, an annotation ought to exist for every valid
  // key. If not, it's an error
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_traversed_map.find(k);
  if (annot == m_traversed_map.end()) {
    rcc_error("Possible invalid key not found in CallGraph map");
  }

  return annot->second;
}
  
bool CallGraphAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator CallGraphAnnotationMap::begin() { return m_traversed_map.begin(); }
iterator CallGraphAnnotationMap::end() { return m_traversed_map.end(); }
const_iterator CallGraphAnnotationMap::begin() const { return m_traversed_map.begin(); }
const_iterator CallGraphAnnotationMap::end() const { return m_traversed_map.end(); }

// ----- creation -----

FundefCallGraphNode * CallGraphAnnotationMap::make_fundef_node(SEXP e) {
  assert(is_fundef(e));
  FundefCallGraphNode * node;
  std::map<SEXP, FundefCallGraphNode *>::const_iterator it = m_fundef_map.find(e);
  if (it == m_fundef_map.end()) {  // not yet in map
    node = new FundefCallGraphNode(e);
    m_node_map[node] = new CallGraphInfo();
    m_fundef_map[e] = node;
  } else {
    node = it->second;
  }
  return node;
}

LibraryCallGraphNode * CallGraphAnnotationMap::make_library_node(SEXP name, SEXP value) {
  assert(is_var(name));
  assert(TYPEOF(value) == CLOSXP ||
	 TYPEOF(value) == BUILTINSXP ||
	 TYPEOF(value) == SPECIALSXP);
  LibraryCallGraphNode * node;
  std::map<SEXP, LibraryCallGraphNode *>::const_iterator it = m_library_map.find(name);
  if (it == m_library_map.end()) {  // not yet in map
    node = new LibraryCallGraphNode(name, value);
    m_node_map[node] = new CallGraphInfo();
    m_library_map[name] = node;
  } else {
    node = it->second;
  }
  return node;
}

CoordinateCallGraphNode * CallGraphAnnotationMap::make_coordinate_node(SEXP name, SEXP scope) {
  assert(is_var(name));
  assert(is_fundef(scope));
  CoordinateCallGraphNode * node;
  std::map<std::pair<SEXP, SEXP>, CoordinateCallGraphNode *>::const_iterator it;
  it = m_coord_map.find(std::make_pair(name, scope));
  if (it == m_coord_map.end()) {
    node = new CoordinateCallGraphNode(name, scope);
    m_node_map[node] = new CallGraphInfo();
    m_coord_map[std::make_pair(name, scope)] = node;
  } else {
    node = it->second;
  }
  return node;
}

CallSiteCallGraphNode * CallGraphAnnotationMap::make_call_site_node(SEXP e) {
  assert(is_cons(e));
  CallSiteCallGraphNode * node;
  std::map<SEXP, CallSiteCallGraphNode *>::const_iterator it = m_call_site_map.find(e);
  if (it == m_call_site_map.end()) {  // not yet in map
    node = new CallSiteCallGraphNode(e);
    m_node_map[node] = new CallGraphInfo();
    m_call_site_map[e] = node;
  } else {
    node = it->second;
  }
  return node; 
}

void CallGraphAnnotationMap::add_edge(const CallGraphNode * const source, const CallGraphNode * const sink) {
  CallGraphEdge * edge = new CallGraphEdge(source, sink);
  m_edge_set.insert(edge);
  CallGraphInfo * source_info = m_node_map[source];
  CallGraphInfo * sink_info = m_node_map[sink];
  assert(source_info != 0);
  assert(sink_info != 0);
  source_info->insert_call_out(edge);
  sink_info->insert_call_in(edge);
}

// ----- computation -----  

void CallGraphAnnotationMap::compute() {
  std::list<CallGraphNode *> worklist;
  std::set<const CallGraphNode *> visited;

  // start with worklist containing one entry, the whole program
  SEXP program = CAR(assign_rhs_c(R_Analyst::get_instance()->get_program()));
  FundefCallGraphNode * root_node = make_fundef_node(program);
  worklist.push_back(root_node);

  while(!worklist.empty()) {
    const CallGraphNode * c = worklist.front();
    worklist.pop_front();

    visited.insert(c);
    
    const FundefCallGraphNode * fnode = dynamic_cast<const FundefCallGraphNode *>(c);
    if (fnode != 0) {
      // if node is a fundef
      visited.insert(fnode);
      FuncInfo * fi = getProperty(FuncInfo, fnode->get_sexp());
      FuncInfo::const_call_site_iterator csi;
      for(csi = fi->beginCallSites(); csi != fi->endCallSites(); ++csi) {
	SEXP cs = *csi;
	CallSiteCallGraphNode * csnode = make_call_site_node(cs);
	add_edge(fnode, csnode);
	if (visited.find(csnode) == visited.end()) {
	  worklist.push_back(csnode);
	}
      }
    } else {
      const CallSiteCallGraphNode * csnode = dynamic_cast<const CallSiteCallGraphNode *>(c);
      if (csnode != 0) {    // if node is a call site
	visited.insert(csnode);
	SEXP cs = csnode->get_sexp();
	if (is_var(call_lhs(cs))) {
	  // if left side of call is a symbol
	  VarBinding * binding = getProperty(VarBinding, cs);  // cell containing symbol
	  VarBinding::const_iterator si;
	  for(si = binding->begin(); si != binding->end(); ++si) {
	    CoordinateCallGraphNode * node;
	    node = make_coordinate_node(call_lhs(cs), (*si)->getDefn());
	    add_edge(csnode, node);
	    if (visited.find(node) == visited.end()) {
	      worklist.push_back(node);
	    }
	  }
	} else {
	  // LHS of call is a non-symbol expression
	  // TODO: handle this case
	  throw AnalysisException();
	}
      } else {
	const CoordinateCallGraphNode * cnode = dynamic_cast<const CoordinateCallGraphNode *>(c);
	if (cnode != 0) {      // if node is a Coordinate
	  visited.insert(cnode);
	  SymbolTable * st = getProperty(SymbolTable, cnode->get_scope());
	  SymbolTable::const_iterator it = st->find(cnode->get_name());
	  if (it == st->end()) {
	    // Name not found in SymbolTable. This means either an unbound
	    // variable or a predefined (library or built-in) function.
	    SEXP value = Rf_findFun(cnode->get_name(), R_GlobalEnv);
	    if (value == R_UnboundValue) {
	      rcc_error("Unbound variable " + var_name(cnode->get_name()));
	    } else {
	      add_edge(cnode, make_library_node(cnode->get_name(), value));
	      // but don't add library fun to worklist
	    }
	  } else {  // symbol is defined at least once in this scope
	    VarInfo * vi = it->second;
	    VarInfo::const_iterator var;
	    for(var = vi->beginDefs(); var != vi->endDefs(); ++var) {
	      DefVar * def = *var;
	      if (is_fundef(CAR(def->getRhs_c()))) {
		// def is of the form _ <- function(...)
		FundefCallGraphNode * node = make_fundef_node(CAR(def->getRhs_c()));
		add_edge(cnode, node);
		if (visited.find(node) == visited.end()) {
		  worklist.push_back(node);
		}
	      } else {
		// if it's a variable, then we know how to handle
		// RHS of def is a non-fundef
		// TODO: handle this case
		throw AnalysisException();
	      }
	    }
	  }
	} else {
	  rcc_error("Unrecognized CallGraphNode type");
	}
      }
    }
  }

  //  for each procedure entry point (just the big proc if one executable):
  //    add to worklist as new call graph node
  //  for each node in worklist:
  //    if it's a Fundef:
  //      for each call site:
  //        add edge (fundef, call site)
  //        if call site node hasn't been visited, add to worklist
  //    else if worklist item is a CallSite:
  //      if LHS is a simple name:
  //        for each scope in VarBinding:
  //          add name, scope as new coordinate node if not already present
  //          add edge (call site, coordinate)
  //          if coordinate node hasn't been visited, add to worklist
  //      else [LHS is not a simple name]:
  //        add edge (call site, any-coordinate)
  //    else if worklist item is a Coordinate:
  //      look up name, scope in SymbolTable, get list of defs
  //      for each def:
  //        if RHS is a fundef:
  //          add fundef as fundef node if not already present
  //          add edge (Coordinate, fundef)
  //          if fundef hasn't been visited, add to worklist
  //        else:
  //          add edge (Coordinate, any-fundef) NOTE: look up DSystem's name
}

void CallGraphAnnotationMap::dump(std::ostream & os) {
  if (!is_computed()) {
    compute();
    m_computed = true;
  }
  
  beginObjDump(os, CallGraph);

  std::map<const CallGraphNode *, CallGraphInfo *>::const_iterator node_it;
  CallGraphInfo::const_iterator edge_it;
  const CallGraphEdge * edge;
  for(node_it = m_node_map.begin(); node_it != m_node_map.end(); ++node_it) {
    const CallGraphNode * node = node_it->first;
    assert(node != 0);
    dumpObj(os, node);
    CallGraphInfo * info = node_it->second;
    assert(info != 0);
    info->dump(os);
    os << std::endl;
  }

  endObjDump(os, CallGraph);
}

} // end namespace RAnnot
