#include <list>

#include <analysis/AnalysisException.h>
#include <analysis/AnalysisResults.h>
#include <analysis/Analyst.h>
#include <analysis/CallSiteCallGraphEdge.h>
#include <analysis/CallGraphEdge.h>
#include <analysis/CallGraphNode.h>
#include <analysis/CoordinateCallGraphEdge.h>
#include <analysis/FundefCallGraphNode.h>
#include <analysis/HandleInterface.h>
#include <analysis/SymbolTable.h>
#include <analysis/VarBinding.h>

#include <support/DumpMacros.h>

#include "CallGraphAnnotationMap.h"

namespace RAnnot {

// ----- type definitions for readability -----
  
typedef CallGraphAnnotationMap::MyKeyT MyKeyT;
typedef CallGraphAnnotationMap::MyMappedT MyMappedT;
typedef CallGraphAnnotationMap::iterator iterator;
typedef CallGraphAnnotationMap::const_iterator const_iterator;
  
// ----- constructor, destructor -----

CallGraphAnnotationMap::CallGraphAnnotationMap()
  : m_computed(false), m_node_map(), m_edge_set()
{}
  
CallGraphAnnotationMap::~CallGraphAnnotationMap() {
  CallGraphAnnotationMap::const_iterator i;
  
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
  
  return m_node_map[k];
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
  std::map<MyKeyT, MyMappedT>::const_iterator annot = m_node_map.find(k);
  if (annot == m_node_map.end()) {
    rcc_error("Possible invalid key not found in CallGraph map");
  }

  return annot->second;
}
  
bool CallGraphAnnotationMap::is_computed() {
  return m_computed;
}

//  ----- iterators ----- 

iterator CallGraphAnnotationMap::begin() { return m_node_map.begin(); }
iterator CallGraphAnnotationMap::end() { return m_node_map.end(); }
const_iterator CallGraphAnnotationMap::begin() const { return m_node_map.begin(); }
const_iterator CallGraphAnnotationMap::end() const { return m_node_map.end(); }

// ----- creation -----

FundefCallGraphNode * CallGraphAnnotationMap::make_fundef_node(SEXP e) {
  assert(is_fundef(e));
  FundefCallGraphNode * node;
  const_iterator it = m_node_map.find(HandleInterface::make_proc_h(e));
  if (it == m_node_map.end()) {  // not yet in map
    node = new FundefCallGraphNode(e);
    m_node_map[HandleInterface::make_proc_h(e)] = new CallGraphInfo();
    m_fundef_map[e] = node;
  } else {
    node = m_fundef_map[e];
  }
  return node;
}

LibraryCallGraphNode * CallGraphAnnotationMap::make_library_node(SEXP e) {
  assert(TYPEOF(e) == CLOSXP || TYPEOF(e) == BUILTINSXP || TYPEOF(e) == SPECIALSXP);
  LibraryCallGraphNode * node;
  OA::IRHandle handle(reinterpret_cast<OA::irhandle_t>(e));
  const_iterator it = m_node_map.find(handle);
  if (it == m_node_map.end()) {  // not yet in map
    node = new LibraryCallGraphNode(e);
    m_node_map[handle] = new CallGraphInfo();
    m_library_map[e] = node;
  } else {
    node = m_library_map[e];
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
    m_node_map[node->get_handle()] = new CallGraphInfo();
    m_coord_map[std::make_pair(name, scope)] = node;
  } else {
    node = it->second;
  }
  return node;
}

void CallGraphAnnotationMap::add_edge(CallGraphNode * source_node,
				      SEXP source_call_site, CallGraphNode * sink)
{
  assert(is_call(source_call_site));
  CallSiteCallGraphEdge * edge = new CallSiteCallGraphEdge(source_call_site, sink);
  m_edge_set.insert(edge);
  CallGraphInfo * source_info = dynamic_cast<CallGraphInfo *>(m_node_map[source_node->get_handle()]);
  CallGraphInfo * sink_info = dynamic_cast<CallGraphInfo *>(m_node_map[sink->get_handle()]);
  assert(source_info != 0);
  assert(sink_info != 0);
  source_info->insert_call_out(edge);
  sink_info->insert_call_in(edge);
}

void CallGraphAnnotationMap::add_edge(CoordinateCallGraphNode * source, CallGraphNode * sink) {
  CoordinateCallGraphEdge * edge = new CoordinateCallGraphEdge(source, sink);
  m_edge_set.insert(edge);
  CallGraphInfo * source_info = dynamic_cast<CallGraphInfo *>(m_node_map[source->get_handle()]);
  CallGraphInfo * sink_info = dynamic_cast<CallGraphInfo *>(m_node_map[sink->get_handle()]);
  assert(source_info != 0);
  assert(sink_info != 0);
  source_info->insert_call_out(edge);
  sink_info->insert_call_in(edge);
}

// ----- computation -----  

void CallGraphAnnotationMap::compute() {
  std::list<CallGraphNode *> worklist;
  std::set<CallGraphNode *> processed;

  // start with worklist containing one entry, the whole program
  SEXP program = CAR(assign_rhs_c(R_Analyst::get_instance()->get_program()));
  FundefCallGraphNode * root_node = make_fundef_node(program);
  worklist.push_back(root_node);

  while(!worklist.empty()) {
    CallGraphNode * c = worklist.front();
    worklist.pop_front();

    processed.insert(c);
    
    FundefCallGraphNode * fnode;
    CoordinateCallGraphNode * cnode;

    if ((fnode = dynamic_cast<FundefCallGraphNode *>(c)) != 0) {
      // if node is a fundef
      FuncInfo * fi = getProperty(FuncInfo, fnode->get_sexp());
      FuncInfo::const_call_site_iterator csi;
      for(csi = fi->beginCallSites(); csi != fi->endCallSites(); ++csi) {
	SEXP cs = *csi;
	if (is_var(call_lhs(cs))) {
	  // if left side of call is a symbol
	  VarBinding * binding = getProperty(VarBinding, cs);  // cell containing symbol
	  VarBinding::const_iterator si;
	  for(si = binding->begin(); si != binding->end(); ++si) {
	    CoordinateCallGraphNode * node;
	    node = make_coordinate_node(call_lhs(cs), (*si)->getDefn());
	    add_edge(fnode, cs, node);
	    if (processed.find(node) == processed.end()) {
	      worklist.push_back(node);
	    }
	  }
	} else {
	  // LHS of call is a non-symbol expression
	  // TODO: handle this case
	  throw AnalysisException();
	}
      }
    } else if ((cnode = dynamic_cast<CoordinateCallGraphNode *>(c)) != 0) {
      // if node is a Coordinate
      std::cout << "A" << std::endl;
      SymbolTable * st = getProperty(SymbolTable, cnode->get_scope());
      std::cout << "B" << std::endl;
      st->dump(std::cout);
      Rf_PrintValue(cnode->get_name());
      SymbolTable::const_iterator it = st->find(cnode->get_name());
      std::cout << "C" << std::endl;
      if (it == st->end()) {
	// Name not found in SymbolTable. This means either an unbound
	// variable or a predefined (library or built-in) function.
	SEXP value = Rf_findFun(cnode->get_name(), R_GlobalEnv);
	if (value == R_UnboundValue) {
	  rcc_error("Unbound variable " + var_name(cnode->get_name()));
	} else {
	  add_edge(cnode, make_library_node(value));
	  // but don't add library fun to worklist
	}
      } else {  // symbol is defined at least once in this scope
	VarInfo * vi = it->second;
	VarInfo::const_iterator var;
	for(var = vi->beginDefs(); var != vi->endDefs(); ++var) {
	  std::cout << "D" << std::endl;
	  DefVar * def = dynamic_cast<DefVar *>(*var);
	  assert(def != 0);
	  std::cout << "E" << std::endl;
	  if (is_fundef(def->getRhs())) {
	    std::cout << "F" << std::endl;
	    // def is of the form _ <- function(...)
	    FundefCallGraphNode * node = make_fundef_node(def->getRhs());
	    std::cout << "G" << std::endl;
	    add_edge(cnode, node);
	    std::cout << "H" << std::endl;
	    if (processed.find(node) != processed.end()) {
	      std::cout << "I" << std::endl;
	      worklist.push_back(node);
	      std::cout << "J" << std::endl;
	    }
	    std::cout << "K" << std::endl;
	  } else {
	    // RHS of def is a non-fundef
	    // TODO: handle this case
	    throw AnalysisException();
	  }
	  std::cout << "L" << std::endl;	
	}
	std::cout << "M" << std::endl;
      }
    } else {
      rcc_error("Unrecognized CallGraphNode type");
    }
  }

  //  for each procedure entry point (just the big proc if one executable):
  //    add to worklist as new call graph node
  //  for each proc in worklist:
  //    if it's a Fundef:
  //      for each call site:
  //        if LHS is not a simple name:
  //          add edge (call site, any-coordinate)
  //        else LHS is a simple name:
  //          for each scope in VarBinding:
  //            add name, scope as new coordinate node if not already present
  //            add edge (call site, coordinate)
  //            if coordinate node hasn't been processed, add to worklist
  //    else if worklist item is a Coordinate:
  //      look up name, scope in SymbolTable, get list of defs
  //      for each def:
  //        if RHS is a fundef:
  //          add fundef as fundef node if not already present
  //          add edge (Coordinate, fundef)
  //          if fundef hasn't been processed, add to worklist
  //        else:
  //          add edge (Coordinate, any-fundef) NOTE: look up DSystem's name
}

void CallGraphAnnotationMap::dump(std::ostream & stream) {
  const_iterator node;
  CallGraphInfo::const_iterator edge;
  for(node = begin(); node != end(); ++node) {
    stream << "Node: ";
    dumpVar(stream, node->first.hval());  // dump node
    CallGraphInfo * info = dynamic_cast<CallGraphInfo *>(node->second);
    assert(info != 0);
    stream << "In: ";
    for(edge = info->begin_calls_in();
	edge != info->end_calls_in();
	++edge)
      {
	dumpPtr(stream, *edge);
	stream << " ";
      }
    stream << std::endl;
    stream << "Out: ";
    for(edge = info->begin_calls_out();
	edge != info->end_calls_out();
	++edge)
      {
	dumpPtr(stream, *edge);
	stream << " ";
      }
    stream << std::endl;
  }
}

} // end namespace RAnnot
