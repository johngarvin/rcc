#ifndef USE_DEF_SOLVER_H
#define USE_DEF_SOLVER_H

#include <OpenAnalysis/Utils/OA_ptr.hpp>
#include <OpenAnalysis/DataFlow/CFGDFProblem.hpp>
#include <OpenAnalysis/DataFlow/IRHandleDataFlowSet.hpp>

#include <analysis/LocalityType.h>
// TODO: change to forward declaration when UseSet, etc. move to separate file

class OA::CFG::Interface;
class RAnnot::AnnotationSet;

class R_UseSet;
class R_VarRef;
class R_VarRefSet;
class R_IRInterface;

//! Implements the OpenAnalysis CFG data flow problem interface to
//! determine whether variable references refer to local or free
//! variables.
class R_UseDefSolver : private OA::DataFlow::CFGDFProblem {
public:
  R_UseDefSolver(OA::OA_ptr<R_IRInterface> _rir);
  ~R_UseDefSolver() {}
  void perform_analysis(OA::ProcHandle proc, OA::OA_ptr<OA::CFG::Interface> cfg);
  void dump_node_maps();
  void dump_node_maps(ostream &os);
  //------------------------------------------------------------------
  // Implementing the callbacks for CFGDFProblem
  //------------------------------------------------------------------
private:
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeTop();
  OA::OA_ptr<OA::DataFlow::DataFlowSet> initializeBottom();

  void initializeNode(OA::OA_ptr<OA::CFG::Interface::Node> n);

  //! CFGDFProblem says: OK to modify set1 and return it as result, because solver
  //! only passes a tempSet in as set1
  OA::OA_ptr<OA::DataFlow::DataFlowSet>
  meet (OA::OA_ptr<OA::DataFlow::DataFlowSet> set1, OA::OA_ptr<OA::DataFlow::DataFlowSet> set2);

  //! CFGDFProblem says: OK to modify in set and return it again as result because
  //! solver clones the BB in sets
  OA::OA_ptr<OA::DataFlow::DataFlowSet> 
  transfer(OA::OA_ptr<OA::DataFlow::DataFlowSet> in, OA::StmtHandle stmt); 

private:
  OA::OA_ptr<R_IRInterface> rir;

  void initialize_sets();

  OA::OA_ptr<R_UseSet> m_all_top;
  OA::OA_ptr<R_UseSet> m_all_bottom;
  OA::OA_ptr<R_UseSet> m_entry_values;
  OA::OA_ptr<OA::CFG::Interface> m_cfg;
  OA::ProcHandle m_proc;
};


//! Associates an R_VarRef with a LocalityType. This is the unit that
//! R_UseSet (which implements DataFlowSet) contains.
//! Modeled after ConstDef in ReachConstsStandard.hpp.
class R_Use {
public:
  // constructors
  R_Use(OA::OA_ptr<R_VarRef> _loc, LocalityType _type);
  R_Use(RAnnot::Var *);
  R_Use(const R_Use& other);

  // access
  OA::OA_ptr<R_VarRef> get_loc() const { return m_loc; }
  LocalityType get_locality_type() const { return m_type; }

  // relationships
  R_Use& operator= (const R_Use& other);

  //! not doing a deep copy
  OA::OA_ptr<R_Use> clone();
    
  //! operator== just compares content of loc
  bool operator== (const R_Use &other) const;
  //! method equiv compares all parts of R_Use as appropriate
  bool equiv(const R_Use& other);
  
  bool operator!= (const R_Use &other) const;
  bool operator< (const R_Use &other) const;
  bool sameLoc (const R_Use &other) const;

  std::string R_Use::toString(OA::OA_ptr<OA::IRHandlesIRInterface> ir);
  std::string R_Use::toString();

private:
  OA::OA_ptr<R_VarRef> m_loc;
  LocalityType m_type;
};

//! Iterator over R_Use's in an R_UseSet
class R_UseSetIterator {
public:
  R_UseSetIterator (OA::OA_ptr<std::set<OA::OA_ptr<R_Use> > > _set) : mSet(_set)
      { assert(!mSet.ptrEqual(NULL)); mIter = mSet->begin(); }
  ~R_UseSetIterator () {}
  
  void operator++();
  bool isValid();
  OA::OA_ptr<R_Use> current();
  void reset();

private:
  OA::OA_ptr<std::set<OA::OA_ptr<R_Use> > > mSet;
  std::set<OA::OA_ptr<R_Use> >::iterator mIter;
};

//! Set of R_Use objects. Inherits from DataFlowSet for use in
//! CFGDFProblem.
// Removed "virtual": need clone() to be able to return an R_UseSet.
//class R_UseSet : public virtual OA::DataFlow::DataFlowSet {
class R_UseSet : public OA::DataFlow::DataFlowSet {
public:

  // methods inherited from DataFlowSet
  // construction
  R_UseSet() { mSet = new std::set<OA::OA_ptr<R_Use> >; }
  R_UseSet(const R_UseSet& other) : mSet(other.mSet) {}
  ~R_UseSet() { }
  
  // After the assignment operation, the lhs R_UseSet will point to
  // the same instances of R_Use's that the rhs points to.  Use clone
  // if you want separate instances of the R_Use's
  R_UseSet& operator= (const R_UseSet& other);
  OA::OA_ptr<OA::DataFlow::DataFlowSet> clone();
  
  void insert(OA::OA_ptr<R_Use> h);
  void remove(OA::OA_ptr<R_Use> h);
  int insert_and_tell(OA::OA_ptr<R_Use> h);
  int remove_and_tell(OA::OA_ptr<R_Use> h);

  //! replace any R_Use in mSet with location locPtr 
  //! with R_Use(locPtr,cdType)
  //! must use this instead of insert because std::set::insert will just see
  //! that the element with the same locptr is already in the set and then not
  //! insert the new element
  void replace(OA::OA_ptr<R_VarRef> loc, LocalityType locality_type);
  void replace(OA::OA_ptr<R_Use> ru);

  // relationship
  // param for these can't be const because will have to 
  // dynamic cast to specific subclass
  bool operator ==(OA::DataFlow::DataFlowSet &other) const;
  bool operator !=(OA::DataFlow::DataFlowSet &other) const
  { return (!(*this==other)); }

  //! need this one for stl containers
  bool operator==(const R_UseSet& other) const 
  { return R_UseSet::operator==(const_cast<R_UseSet&>(other)); }

  bool empty() const { return mSet->empty(); }
  
  //! Return pointer to a copy of a R_Use in this set with matching loc
  //! NULL is returned if no R_Use in this set matches loc
  OA::OA_ptr<R_Use> find(OA::OA_ptr<R_VarRef> locPtr) const;

  void insert_varset(OA::OA_ptr<R_VarRefSet> vars, LocalityType type);

  // debugging
  std::string toString(OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  std::string toString();
  void dump(std::ostream &os, OA::OA_ptr<OA::IRHandlesIRInterface> pIR);
  void dump(std::ostream &os);

  // new methods not inherited
  OA::OA_ptr<R_UseSetIterator> get_iterator() const;
  
protected:
  OA::OA_ptr<std::set<OA::OA_ptr<R_Use> > > mSet;

  friend class R_UseSetIterator;  
};

OA::OA_ptr<R_UseSet> meet_use_set(OA::OA_ptr<R_UseSet> set1, OA::OA_ptr<R_UseSet>);

#endif  // R_UseDefSolver_h
