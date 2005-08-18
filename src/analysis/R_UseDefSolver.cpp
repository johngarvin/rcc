#include "R_UseDefSolver.hpp"


static const bool debug = false;

//! meet for a single lattice element
static VarType var_meet(VarType x, VarType y);

//! meet function for a single lattice element
static VarType var_meet(VarType x, VarType y) {
  if (x == y) {
    return x;
  } else if (x == TOP) {
    return y;
  } else if (y == TOP) {
    return x;
  } else {
    return BOTTOM;
  }
}

R_UseDefSolver::R_UseDefSolver(OA::OA_ptr<R_IRInterface> _rir)
  : OA::DataFlow::CFGDFProblem( OA::DataFlow::Forward ), rir(_rir)
{
}

// return (smart) ptr to AnnotationSet?
OA::OA_ptr<RAnnot::AnnotationSet> R_UseDefSolver::
perform_analysis(OA::ProcHandle proc, OA::OA_ptr<OA::CFG::Interface> cfg) {
  m_cfg = cfg;
  m_proc = proc;

  // Note: ReachConsts uses:
  //  OA::DataFlow::CFGDFProblem::solve(cfg);
  // Why?
  solve(cfg);
  // fills in mNodeInSetMap and mNodeOutSetMap

  // For reference:
  //std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlowSet> > mNodeInSetMap;
  //std::map<OA_ptr<CFG::Interface::Node>,OA_ptr<DataFlowSet> > mNodeOutSetMap;

  OA::OA_ptr<RAnnot::AnnotationSet> an; an = new RAnnot::AnnotationSet();
  std::map<OA::OA_ptr<OA::CFG::Interface::Node>,OA::OA_ptr<OA::DataFlow::DataFlowSet> >::iterator map_iter;
  for(map_iter = mNodeOutSetMap.begin(); map_iter != mNodeOutSetMap.end(); ++map_iter) {
    OA::OA_ptr<OA::CFG::Interface::NodeStatementsIterator> stmt_iter_ptr;
    stmt_iter_ptr = map_iter->first->getNodeStatementsIterator();
    for( ; stmt_iter_ptr->isValid(); ++*stmt_iter_ptr) {          // should be only one
      //      OA::OA_ptr<R_UseSet> use_set = map_iter->second; // = ptr<R_UseSet>
      // for each element of use_set:
        // make VarInfo?
        // make Var
      //      an.insert(make_pair((SEXP)stmt_iter_ptr->current().hval(), map_iter->second));
    }
  }
  return an;
}

void R_UseDefSolver::dump_node_maps() {
  OA::OA_ptr<OA::DataFlow::DataFlowSet> df_in_set, df_out_set;
  OA::OA_ptr<R_UseSet> in_set, out_set;
  OA::OA_ptr<OA::CFG::Interface::NodesIterator> node_it = m_cfg->getNodesIterator();
  for ( ; node_it->isValid(); ++*node_it) {
    df_in_set = mNodeInSetMap[node_it->current()];
    df_out_set = mNodeOutSetMap[node_it->current()];
    in_set = df_in_set.convert<R_UseSet>();
    out_set = df_out_set.convert<R_UseSet>();
    //node_it->current()->dump(std::cout);
    std::cout << "CFG NODE #" << node_it->current()->getId() << ":\n";
    std::cout << "IN SET:\n";
    in_set->dump(std::cout, rir);
    std::cout << "OUT SET:\n";
    out_set->dump(std::cout, rir);
  }
}

//------------------------------------------------------------------
// Implementing the callbacks for CFGDFProblem
//------------------------------------------------------------------

//! Create a set of all variables set to TOP
OA::OA_ptr<OA::DataFlow::DataFlowSet> R_UseDefSolver::initializeTop() {
  if (all_top.ptrEqual(NULL)) {
    initialize_sets();
  }
  return all_top->clone();
}

//! Create a set of all variable set to BOTTOM
OA::OA_ptr<OA::DataFlow::DataFlowSet> R_UseDefSolver::initializeBottom() {
  if (all_bottom.ptrEqual(NULL)) {
    initialize_sets();
  }
  return all_bottom->clone();
}

void R_UseDefSolver::initialize_sets() {
  all_top = new R_UseSet;
  all_bottom = new R_UseSet;
  entry_values = new R_UseSet;
  OA::OA_ptr<OA::CFG::Interface::NodesIterator> node_it;
  node_it = m_cfg->getNodesIterator();
  OA::OA_ptr<OA::CFG::Interface::NodeStatementsIterator> stmt_it;
  for ( ; node_it->isValid(); ++*node_it) {
    // add vars found in each statement
    stmt_it = node_it->current()->getNodeStatementsIterator();
    for( ; stmt_it->isValid(); ++*stmt_it) {
      SEXP stmt_r = (SEXP)stmt_it->current().hval();

      if (debug) {
	PrintValue(stmt_r);
      }

      R_ExpUDLocInfo info(stmt_r);

      // all_top and all_bottom: all variables set to TOP/BOTTOM,
      // must be initialized for OA data flow analysis
      all_top->insert_varset(info.get_local_defs(), TOP);
      all_top->insert_varset(info.get_free_defs(), TOP);
      all_top->insert_varset(info.get_app_uses(), TOP);
      all_top->insert_varset(info.get_non_app_uses(), TOP);
      all_bottom->insert_varset(info.get_local_defs(), BOTTOM);
      all_bottom->insert_varset(info.get_free_defs(), BOTTOM);
      all_bottom->insert_varset(info.get_app_uses(), BOTTOM);
      all_bottom->insert_varset(info.get_non_app_uses(), BOTTOM);

      // entry_values: initial in set for entry node.  Formal
      // arguments are LOCAL; all other variables are FREE.  We add
      // all variables as FREE here, then reset the formals as LOCAL
      // after the per-node and per-statement stuff.
      entry_values->insert_varset(info.get_local_defs(), FREE);
      entry_values->insert_varset(info.get_free_defs(), FREE);
      entry_values->insert_varset(info.get_app_uses(), FREE);
      entry_values->insert_varset(info.get_non_app_uses(), FREE);

    } // statements
  } // CFG nodes
  
  // set all formals LOCAL instead of FREE on entry
  SEXP arglist = CAR(fundef_args_c((SEXP)m_proc.hval()));
  entry_values->insert_varset(refs_from_arglist(arglist), LOCAL);
}

//! Creates in and out R_UseSets and stores them in mNodeInSetMap and
//! mNodeOutSetMap.
void R_UseDefSolver::initializeNode(OA::OA_ptr<OA::CFG::Interface::Node> n)
{
  //  local_defs[n] = new VarSet();
  //  free_defs[n] = new VarSet();
  // now done in initialize_top_and_bottom

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
OA::OA_ptr<OA::DataFlow::DataFlowSet> R_UseDefSolver::
meet(OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, OA::OA_ptr<OA::DataFlow::DataFlowSet> set2)
{
  OA::OA_ptr<R_UseSet> retval;
  OA::OA_ptr<OA::DataFlow::DataFlowSet> set2clone = set2->clone();
  retval = meet_use_set(set1.convert<R_UseSet>(), set2clone.convert<R_UseSet>());
  return retval.convert<OA::DataFlow::DataFlowSet>();
}

//! Transfer function adds data given a statement. CFGDFProblem says:
//! OK to modify in set and return it again as result because solver
//! clones the BB in sets
OA::OA_ptr<OA::DataFlow::DataFlowSet> R_UseDefSolver::
transfer(OA::OA_ptr<OA::DataFlow::DataFlowSet> in_dfs, OA::StmtHandle stmt_handle) {
  OA::OA_ptr<R_UseSet> in; in = in_dfs.convert<R_UseSet>();
  SEXP stmt = (SEXP)stmt_handle.hval();
  R_ExpUDLocInfo info(stmt);
  in->insert_varset(info.get_local_defs(), LOCAL);
  //in->insert_varset(info.get_free_defs(), FREE);
  return in.convert<OA::DataFlow::DataFlowSet>();
}

//--------------------------------------------------------------------
// R_Use methods
//--------------------------------------------------------------------

//! copy an R_Use, not a deep copy, will refer to same Location
//! as other
R_Use& R_Use::operator=(const R_Use& other) {
  loc = other.getLoc();
  type = other.getVarType();
  return *this;
}

//! Equality operator for R_Use.  Just inspects location contents.
bool R_Use::operator== (const R_Use &other) const {
  return (loc==other.getLoc());
}

//! Just based on location, this way when insert a new R_Use it can
//! override the existing R_Use with same location
bool R_Use::operator< (const R_Use &other) const { 
  return (loc < other.getLoc());
}

//! Equality method for R_Use.
bool R_Use::equiv(const R_Use& other) {
  return (loc == other.getLoc() && type == other.getVarType());
}

std::string R_Use::toString(OA::OA_ptr<OA::IRHandlesIRInterface> pIR) {
  return toString();
}

//! Return a string that contains a character representation of
//! an R_Use.
std::string R_Use::toString() {
  std::ostringstream oss;
  oss << "<";
  oss << (*loc).toString();
  switch (type) {
  case TOP:
    oss << ",TOP>"; break;
  case BOTTOM: 
    oss << ",BOTTOM>"; break;
  case LOCAL:
    oss << ",LOCAL>"; break;
  case FREE:
    oss << ",FREE>"; break;
  }
  return oss.str();
}

//--------------------------------------------------------------------
// R_UseSet methods
//--------------------------------------------------------------------

//! Replace any R_Use in mSet with the same location as the given use
void R_UseSet::replace(OA::OA_ptr<R_Use> use) {
    replace(use->getLoc(), use->getVarType());
}

//! replace any R_Use in mSet with location loc 
//! with R_Use(loc,type)
void R_UseSet::replace(OA::OA_ptr<R_VarRef> loc, VarType type) {
  OA::OA_ptr<R_Use> use;
  use = new R_Use(loc);
  remove(use);
  use = new R_Use(loc,type);
  insert(use);
}


//! operator== for an R_UseSet cannot rely upon the == operator for
//! the underlying sets, because the == operator of an element of a
//! R_UseSet, namely a R_Use, only considers the contents of the
//! location pointer and not any of the other fields.  So, need to use
//! R_Use's equal() method here instead.
bool R_UseSet::operator==(OA::DataFlow::DataFlowSet &other) const
{

  // first dynamic cast to a R_UseSet, if that doesn't work then 
  // other is a different kind of DataFlowSet and *this is not equal
  R_UseSet& recastOther = dynamic_cast<R_UseSet&>(other);

  if (mSet->size() != recastOther.mSet->size()) {
    return false;
  }

  // same size:  for every element in mSet, find the element in other.mSet
  std::set<OA::OA_ptr<R_Use> >::iterator set1Iter;
  for (set1Iter = mSet->begin(); set1Iter!=mSet->end(); ++set1Iter) {
    OA::OA_ptr<R_Use> cd1 = *set1Iter;
    std::set<OA::OA_ptr<R_Use> >::iterator set2Iter;
    set2Iter = recastOther.mSet->find(cd1);

    if (set2Iter == recastOther.mSet->end()) {
      return (false); // cd1 not found
    } else {
      OA::OA_ptr<R_Use> cd2 = *set2Iter;
      if (!(cd1->equiv(*cd2))) { // use full equiv operator
        return (false); // cd1 not equiv to cd2
      }
    }
  } // end of set1Iter loog

  // hopefully, if we got here, all elements of set1 equiv set2
  return(true);

}

//! find the R_Use in this R_UseSet with the given location (should
//! be at most one) return a ptr to that R_Use
OA::OA_ptr<R_Use> R_UseSet::find(OA::OA_ptr<R_VarRef> loc) const {
  OA::OA_ptr<R_Use> retval; retval = NULL;
  
  OA::OA_ptr<R_Use> find_var; find_var = new R_Use(loc);
  // constructor starts as TOP

  std::set<OA::OA_ptr<R_Use> >::iterator iter = mSet->find(find_var);
  if (iter != mSet->end()) {
    retval = *iter;
  }
  return retval;
}

#if 0
//! Union in a set of variables associated with a given statement
void R_UseSet::insert_varset(OA::StmtHandle stmt,
			    OA::OA_ptr<VarSet> vars,
			    VarType type) {
  OA::OA_ptr<VarSetIterator> it; it = vars->get_iterator();
  OA::OA_ptr<R_Use> use;
  OA::OA_ptr<R_VarRef> loc;
  for ( ; it->isValid(); ++*it) {
    loc = new R_VarRef((SEXP)stmt.hval(), it->current());
    use = new R_Use(loc, type);
    mSet->insert(use);
  }
}
#endif

//! Union in a set of variables associated with a given statement
void R_UseSet::insert_varset(OA::OA_ptr<R_VarRefSet> vars,
			    VarType type) {
  OA::OA_ptr<R_VarRefSetIterator> it = vars->get_iterator();
  OA::OA_ptr<R_Use> use;
  for (; it->isValid(); ++*it) {
    replace(it->current(),type);
  }
}


//! Return a string representing the contents of an R_UseSet
std::string R_UseSet::toString(OA::OA_ptr<OA::IRHandlesIRInterface> pIR) {
  std::ostringstream oss;
  oss << "{";
  
  // iterate over R_Use's and have the IR print them out
  R_UseSetIterator iter(*this);
  //std::set<OA::OA_ptr<R_Use> >::iterator iter;
  OA::OA_ptr<R_Use> use;
  
  // first one
  if (iter.isValid()) {
    use = iter.current();
    oss << use->toString(pIR);
    ++iter;
  }
  
  // rest
  for (; iter.isValid(); ++iter) {
    use = iter.current();
    oss << ", " << use->toString(pIR); 
  }
  
  oss << "}";
  return oss.str();
}

//! Meet function for two R_UseSets, using the single-variable meet
//! operation when a use appears in both sets
OA::OA_ptr<R_UseSet> meet_use_set(OA::OA_ptr<R_UseSet> set1, OA::OA_ptr<R_UseSet> set2) {
  OA::OA_ptr<R_UseSet> retval;
  retval = set1->clone().convert<R_UseSet>();
  R_UseSetIterator it2(*set2);
  OA::OA_ptr<R_Use> use1;
  for( ; it2.isValid(); ++it2) {
    use1 = set1->find(it2.current()->getLoc());
    if (use1.ptrEqual(NULL)) {
      retval->insert(it2.current());
    } else {
      VarType new_type = var_meet(use1->getVarType(), it2.current()->getVarType());
      retval->replace(use1->getLoc(), new_type);
    }
  }
  return retval;
}
