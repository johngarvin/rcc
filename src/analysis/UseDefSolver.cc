#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/CFG/Interface.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/HandleInterface.h>
#include <analysis/IRInterface.h>
#include <analysis/Annotation.h>
#include <analysis/AnnotationSet.h>
#include <analysis/AnalysisResults.h>
#include <analysis/LocalVariableAnalysis.h>

#include <analysis/UseDefSolver.h>

using namespace OA;
using namespace RAnnot;

static R_VarRef * make_var_ref_from_annotation(Var * annot);

static const bool debug = false;

//! meet for a single lattice element
static LocalityType var_meet(LocalityType x, LocalityType y);

//! meet function for a single lattice element
static LocalityType var_meet(LocalityType x, LocalityType y) {
  if (x == y) {
    return x;
  } else if (x == Locality_TOP) {
    return y;
  } else if (y == Locality_TOP) {
    return x;
  } else {
    return Locality_BOTTOM;
  }
}

//--------------------------------------------------------------------
// Meet function
//--------------------------------------------------------------------

//! Meet function for two R_UseSets, using the single-variable meet
//! operation when a use appears in both sets
OA_ptr<R_UseSet> meet_use_set(OA_ptr<R_UseSet> set1, OA_ptr<R_UseSet> set2) {
  OA_ptr<R_UseSet> retval;
  retval = set1->clone().convert<R_UseSet>();
  OA_ptr<R_UseSetIterator> it2 = set2->get_iterator();
  OA_ptr<R_Use> use1;
  for( ; it2->isValid(); ++*it2) {
    use1 = set1->find(it2->current()->get_loc());
    if (use1.ptrEqual(NULL)) {
      retval->insert(it2->current());
    } else {
      LocalityType new_type = var_meet(use1->get_locality_type(), it2->current()->get_locality_type());
      retval->replace(use1->get_loc(), new_type);
    }
  }
  return retval;
}


//--------------------------------------------------------------------
// UseDefSolver methods
//--------------------------------------------------------------------

R_UseDefSolver::R_UseDefSolver(OA_ptr<R_IRInterface> _rir)
  : DataFlow::CFGDFProblem( DataFlow::Forward ), rir(_rir)
{
}

//! Perform data flow analysis. Creates and returns a pointer to an
//! AnnotationSet that maps each variable reference to a Var
//! annotation with use and def information. The caller is responsible
//! for releasing the memory when finished.
RAnnot::AnnotationSet* R_UseDefSolver::
perform_analysis(ProcHandle proc, OA_ptr<CFG::Interface> cfg) {
  m_cfg = cfg;
  m_proc = proc;

  // Note: ReachConsts explicitly uses base class name:
  //  DataFlow::CFGDFProblem::solve(cfg);
  // Why?
  solve(cfg);
  // solve populates mNodeInSetMap and mNodeOutSetMap

  // For reference:
  //std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlowSet> > mNodeInSetMap;
  //std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlowSet> > mNodeOutSetMap;

  RAnnot::AnnotationSet* an = new RAnnot::AnnotationSet();

  // iterate through each node's mNodeOutSetMap
  std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlow::DataFlowSet> >::iterator map_iter;
  for(map_iter = mNodeOutSetMap.begin(); map_iter != mNodeOutSetMap.end(); ++map_iter) {

    // map contains ptrs to DataFlowSet; convert to ptr to derived class R_UseSet
    OA_ptr<R_UseSet> use_set = map_iter->second.convert<R_UseSet>();

    // iterate through map
    OA_ptr<R_UseSetIterator> use_iter = use_set->get_iterator();
    for( ; use_iter->isValid(); ++*use_iter) {
      RAnnot::Var *var = new RAnnot::Var();
      var->setType(RAnnot::Var::Var_USE);
      // FIXME vi->setMayMustType(???);
      OA_ptr<R_VarRef> location; location = use_iter->current()->get_loc();
      an->insert(std::make_pair(HandleInterface::make_mem_ref_h(location->get_sexp()), var));
    }
  }
  return an;
}

void R_UseDefSolver::dump_node_maps() {
  dump_node_maps(std::cout);
}

//! Print out a representation of the in and out sets for each CFG node.
void R_UseDefSolver::dump_node_maps(ostream &os) {
  OA_ptr<DataFlow::DataFlowSet> df_in_set, df_out_set;
  OA_ptr<R_UseSet> in_set, out_set;
  OA_ptr<CFG::Interface::NodesIterator> node_it = m_cfg->getNodesIterator();
  for ( ; node_it->isValid(); ++*node_it) {
    df_in_set = mNodeInSetMap[node_it->current()];
    df_out_set = mNodeOutSetMap[node_it->current()];
    in_set = df_in_set.convert<R_UseSet>();
    out_set = df_out_set.convert<R_UseSet>();
    os << "CFG NODE #" << node_it->current()->getId() << ":\n";
    os << "IN SET:\n";
    in_set->dump(os, rir);
    os << "OUT SET:\n";
    out_set->dump(os, rir);
  }
}

//------------------------------------------------------------------
// Implementing the callbacks for CFGDFProblem
//------------------------------------------------------------------

//! Create a set of all variables set to TOP
OA_ptr<DataFlow::DataFlowSet> R_UseDefSolver::initializeTop() {
  if (all_top.ptrEqual(NULL)) {
    initialize_sets();
  }
  return all_top->clone();
}

//! Create a set of all variable set to BOTTOM
OA_ptr<DataFlow::DataFlowSet> R_UseDefSolver::initializeBottom() {
  if (all_bottom.ptrEqual(NULL)) {
    initialize_sets();
  }
  return all_bottom->clone();
}

void R_UseDefSolver::initialize_sets() {
  all_top = new R_UseSet;
  all_bottom = new R_UseSet;
  entry_values = new R_UseSet;
  OA_ptr<CFG::Interface::NodesIterator> node_it;
  node_it = m_cfg->getNodesIterator();
  OA_ptr<CFG::Interface::NodeStatementsIterator> stmt_it;
  for ( ; node_it->isValid(); ++*node_it) {
    // add vars found in each statement
    stmt_it = node_it->current()->getNodeStatementsIterator();
    for( ; stmt_it->isValid(); ++*stmt_it) {
      SEXP stmt_r = (SEXP)stmt_it->current().hval();

      if (debug) {
	Rf_PrintValue(stmt_r);
      }

      ExpressionInfo * stmt_annot = getProperty(ExpressionInfo, stmt_r);
      if (stmt_annot == 0) {
	LocalVariableAnalysis lva(stmt_r);
	lva.perform_analysis();
      }

      Var::iterator var_iter;
      for(var_iter = stmt_annot->begin(); var_iter != stmt_annot->end(); ++var_iter) {
	OA_ptr<R_VarRef> ref; ref = new R_BodyVarRef((*var_iter)->getMention());

	// all_top and all_bottom: all variables set to TOP/BOTTOM,
	// must be initialized for OA data flow analysis
	OA_ptr<R_Use> top; top = new R_Use(ref, Locality_TOP);
	all_top->insert(top);
	OA_ptr<R_Use> bottom; bottom = new R_Use(ref, Locality_BOTTOM);
	all_bottom->insert(bottom);

	// entry_values: initial in set for entry node.  Formal
	// arguments are LOCAL; all other variables are FREE.  We add
	// all variables as FREE here, then reset the formals as LOCAL
	// after the per-node and per-statement stuff.
	OA_ptr<R_Use> free; free = new R_Use(ref, Locality_FREE);
	entry_values->insert(free);
      }
    } // statements
  } // CFG nodes
  
  // set all formals LOCAL instead of FREE on entry
  SEXP arglist = CAR(fundef_args_c((SEXP)m_proc.hval()));
  entry_values->insert_varset(R_VarRefSet::refs_from_arglist(arglist), Locality_LOCAL);
}

//! Creates in and out R_UseSets and stores them in mNodeInSetMap and
//! mNodeOutSetMap.
void R_UseDefSolver::initializeNode(OA_ptr<CFG::Interface::Node> n) {
  if (n.ptrEqual(m_cfg->getEntry())) {
    mNodeInSetMap[n] = entry_values->clone();
  } else {
    mNodeInSetMap[n] = all_top->clone();
  }
  mNodeOutSetMap[n] = all_top->clone();
}

//! Meet function merges info from predecessors. CFGDFProblem says: OK
//! to modify set1 and return it as result, because solver only passes
//! a tempSet in as set1
OA_ptr<DataFlow::DataFlowSet> R_UseDefSolver::
meet(OA_ptr<DataFlow::DataFlowSet> set1, OA_ptr<DataFlow::DataFlowSet> set2) {
  OA_ptr<R_UseSet> retval;
  OA_ptr<DataFlow::DataFlowSet> set2clone = set2->clone();
  retval = meet_use_set(set1.convert<R_UseSet>(), set2clone.convert<R_UseSet>());
  return retval.convert<DataFlow::DataFlowSet>();
}

//! Transfer function adds data given a statement. CFGDFProblem says:
//! OK to modify in set and return it again as result because solver
//! clones the BB in sets
OA_ptr<DataFlow::DataFlowSet> R_UseDefSolver::
transfer(OA_ptr<DataFlow::DataFlowSet> in_dfs, StmtHandle stmt_handle) {
  OA_ptr<R_UseSet> in; in = in_dfs.convert<R_UseSet>();
  SEXP stmt = (SEXP)stmt_handle.hval();
  ExpressionInfo * annot = getProperty(ExpressionInfo, stmt);
  ExpressionInfo::iterator var_iter;
  for(var_iter = annot->begin(); var_iter != annot->end(); ++var_iter) {
    if ((*var_iter)->getLocalityType() == Locality_LOCAL) {
      OA_ptr<R_Use> use; use = new R_Use(*var_iter);
      in->insert(use);
    }
  }
}

//--------------------------------------------------------------------
// R_Use methods
//--------------------------------------------------------------------

R_Use::R_Use(OA_ptr<R_VarRef> _loc, LocalityType _type)
  : m_loc(_loc), m_type(_type)
  {}

R_Use::R_Use(RAnnot::Var * annot)
  : m_type(annot->getLocalityType())
{
  OA_ptr<R_VarRef> ref; ref = make_var_ref_from_annotation(annot);
  m_loc = ref;
}

R_Use::R_Use(const R_Use& other)
  : m_loc(other.m_loc), m_type(other.m_type)
  {}

//! not doing a deep copy
OA_ptr<R_Use> R_Use::clone() { 
  OA_ptr<R_Use> retval;
  retval = new R_Use(*this);
  return retval;
}
  
//! copy an R_Use, not a deep copy, will refer to same Location
//! as other
R_Use& R_Use::operator=(const R_Use& other) {
  m_loc = other.get_loc();
  m_type = other.get_locality_type();
  return *this;
}

//! Equality operator for R_Use.  Just inspects location contents.
bool R_Use::operator== (const R_Use &other) const {
  return (m_loc==other.get_loc());
}

//! Inequality operator.
bool R_Use::operator!= (const R_Use &other) const {
  return !(*this==other);
}


//! Just based on location, this way when insert a new R_Use it can
//! override the existing R_Use with same location
bool R_Use::operator< (const R_Use &other) const { 
  return (m_loc < other.get_loc());
}

//! Equality method for R_Use.
bool R_Use::equiv(const R_Use& other) {
  return (m_loc == other.get_loc() && m_type == other.get_locality_type());
}

bool R_Use::sameLoc (const R_Use &other) const {
  return (m_loc == other.get_loc());
}

std::string R_Use::toString(OA_ptr<IRHandlesIRInterface> pIR) {
  return toString();
}

//! Return a string that contains a character representation of
//! an R_Use.
std::string R_Use::toString() {
  std::ostringstream oss;
  oss << "<";
  oss << m_loc->toString();
  switch (m_type) {
  case Locality_TOP:
    oss << ",TOP>"; break;
  case Locality_BOTTOM: 
    oss << ",BOTTOM>"; break;
  case Locality_LOCAL:
    oss << ",LOCAL>"; break;
  case Locality_FREE:
    oss << ",FREE>"; break;
  }
  return oss.str();
}

//--------------------------------------------------------------------
// R_UseSetIterator methods
//--------------------------------------------------------------------

void R_UseSetIterator::operator++() {
  if (isValid()) mIter++; 
}

//! is the iterator at the end
bool R_UseSetIterator::isValid() {
  return (mIter != mSet->end());
}

//! return copy of current node in iterator
OA_ptr<R_Use> R_UseSetIterator::current() {
  assert(isValid());
  return (*mIter);
}

//! reset iterator to beginning of set
void R_UseSetIterator::reset() {
  mIter = mSet->begin();
}

//--------------------------------------------------------------------
// R_UseSet methods
//--------------------------------------------------------------------

// After the assignment operation, the lhs R_UseSet will point to
// the same instances of R_Use's that the rhs points to.  Use clone
// if you want separate instances of the R_Use's
R_UseSet& R_UseSet::operator= (const R_UseSet& other) {
  mSet = other.mSet; 
  return *this;
}

OA_ptr<DataFlow::DataFlowSet> R_UseSet::clone() {
  OA_ptr<R_UseSet> retval;
  retval = new R_UseSet(); 
  std::set<OA_ptr<R_Use> >::iterator defIter;
  for (defIter=mSet->begin(); defIter!=mSet->end(); defIter++) {
    OA_ptr<R_Use> def = *defIter;
    retval->insert(def->clone());
  }
  return retval;
}
  
void R_UseSet::insert(OA_ptr<R_Use> h) {
  mSet->insert(h); 
}
  
void R_UseSet::remove(OA_ptr<R_Use> h) {
  remove_and_tell(h); 
}

int R_UseSet::insert_and_tell(OA_ptr<R_Use> h) {
  return (int)((mSet->insert(h)).second); 
}

int R_UseSet::remove_and_tell(OA_ptr<R_Use> h) {
  return (mSet->erase(h)); 
}

//! Replace any R_Use in mSet with the same location as the given use
void R_UseSet::replace(OA_ptr<R_Use> use) {
    replace(use->get_loc(), use->get_locality_type());
}

//! replace any R_Use in mSet with location loc 
//! with R_Use(loc,type)
void R_UseSet::replace(OA_ptr<R_VarRef> loc, LocalityType type) {
  OA_ptr<R_Use> use;
  use = new R_Use(loc, Locality_TOP);
  remove(use);
  use = new R_Use(loc,type);
  insert(use);
}

//! operator== for an R_UseSet cannot rely upon the == operator for
//! the underlying sets, because the == operator of an element of a
//! R_UseSet, namely an R_Use, only considers the contents of the
//! location pointer and not any of the other fields.  So, need to use
//! R_Use's equal() method here instead.
bool R_UseSet::operator==(DataFlow::DataFlowSet &other) const {
  // first dynamic cast to an R_UseSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  R_UseSet& recastOther = dynamic_cast<R_UseSet&>(other);

  if (mSet->size() != recastOther.mSet->size()) {
    return false;
  }

  // same size:  for every element in mSet, find the element in other.mSet
  std::set<OA_ptr<R_Use> >::iterator set1Iter;
  for (set1Iter = mSet->begin(); set1Iter!=mSet->end(); ++set1Iter) {
    OA_ptr<R_Use> cd1 = *set1Iter;
    std::set<OA_ptr<R_Use> >::iterator set2Iter;
    set2Iter = recastOther.mSet->find(cd1);

    if (set2Iter == recastOther.mSet->end()) {
      return (false); // cd1 not found
    } else {
      OA_ptr<R_Use> cd2 = *set2Iter;
      if (!(cd1->equiv(*cd2))) { // use full equiv operator
        return (false); // cd1 not equiv to cd2
      }
    }
  } // end of set1Iter loop

  // hopefully, if we got here, all elements of set1 equiv set2
  return(true);
}

//! find the R_Use in this R_UseSet with the given location (should
//! be at most one) return a ptr to that R_Use
OA_ptr<R_Use> R_UseSet::find(OA_ptr<R_VarRef> loc) const {
  OA_ptr<R_Use> retval; retval = NULL;
  
  OA_ptr<R_Use> find_var; find_var = new R_Use(loc, Locality_TOP);

  std::set<OA_ptr<R_Use> >::iterator iter = mSet->find(find_var);
  if (iter != mSet->end()) {
    retval = *iter;
  }
  return retval;
}

//! Union in a set of variables associated with a given statement
void R_UseSet::insert_varset(OA_ptr<R_VarRefSet> vars,
			    LocalityType type)
{
  OA_ptr<R_VarRefSetIterator> it = vars->get_iterator();
  OA_ptr<R_Use> use;
  for (; it->isValid(); ++*it) {
    replace(it->current(),type);
  }
}


//! Return a string representing the contents of an R_UseSet
std::string R_UseSet::toString(OA_ptr<IRHandlesIRInterface> pIR) {
  std::ostringstream oss;
  oss << "{";
  
  // iterate over R_Use's and have the IR print them out
  OA_ptr<R_UseSetIterator> iter = get_iterator();
  OA_ptr<R_Use> use;
  
  // first one
  if (iter->isValid()) {
    use = iter->current();
    oss << use->toString(pIR);
    ++*iter;
  }
  
  // rest
  for (; iter->isValid(); ++*iter) {
    use = iter->current();
    oss << ", " << use->toString(pIR); 
  }
  
  oss << "}";
  return oss.str();
}

void R_UseSet::dump(std::ostream &os, OA_ptr<IRHandlesIRInterface> pIR) {
  os << toString(pIR) << std::endl;
}

void R_UseSet::dump(std::ostream &os) {
  std::cout << "call dump(os,interface) instead";
}

OA_ptr<R_UseSetIterator> R_UseSet::get_iterator() const {
  OA_ptr<R_UseSetIterator> it;
  it = new R_UseSetIterator(mSet);
  return it;
}

static R_VarRef * make_var_ref_from_annotation(RAnnot::Var * annot) {
  DefVar * def_annot;
  if (dynamic_cast<UseVar *>(annot)) {
    return new R_BodyVarRef(annot->getMention());
  } else if (def_annot = dynamic_cast<DefVar *>(annot)) {
    DefVar::SourceT source = def_annot->getSourceType();
    R_VarRef * retval;
    switch(source) {
    case DefVar::DefVar_ASSIGN:
      return new R_BodyVarRef(annot->getMention());
      break;
    case DefVar::DefVar_FORMAL:
      return new R_ArgVarRef(annot->getMention());
      break;
    default:
      err("make_var_ref_from_annotation: unrecognized DefVar::SourceT");
      return 0;
    }
  } else {
    err(0 && "make_var_ref_from_annotation: unknown annotation type");
  }
}
