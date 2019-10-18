//===- Andersens.h - Andersens AA ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This is the interface for alias analysis for Andersens points-to.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_ANDERSENS_H
#define LLVM_ANALYSIS_INTEL_ANDERSENS_H

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SparseBitVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Pass.h"

#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>

namespace llvm {

using AndersGetTLITy = std::function<const TargetLibraryInfo &(Function &)>;

// Forward declaration for use within AndersensAAResult class
class IntelModRefImpl;
class IntelModRef;

/// An alias analysis result set for Andersens points-to.
///
/// This focuses on handling aliasing properties using Andersens points-to.
class AndersensAAResult : public AAResultBase<AndersensAAResult>, 
                          private InstVisitor<AndersensAAResult> {
  friend AAResultBase<AndersensAAResult>;
  friend IntelModRefImpl;

  struct BitmapKeyInfo {
    static inline SparseBitVector<> *getEmptyKey() {
      return reinterpret_cast<SparseBitVector<> *>(-1);
    }
    static inline SparseBitVector<> *getTombstoneKey() {
      return reinterpret_cast<SparseBitVector<> *>(-2);
    }
    static unsigned getHashValue(const SparseBitVector<> *bitmap) {
      uint64_t HashVal = 0;

      for (auto bi = bitmap->begin(), E = bitmap->end();
         bi != E; ++bi) {
        HashVal ^= *bi;
      }
      return HashVal;
    }
    static bool isEqual(const SparseBitVector<> *LHS,
                        const SparseBitVector<> *RHS) {
      if (LHS == RHS)
        return true;
      else if (LHS == getEmptyKey() || RHS == getEmptyKey()
               || LHS == getTombstoneKey() || RHS == getTombstoneKey())
        return false;

      return *LHS == *RHS;
    }

    static bool isPod() { return true; }
  };

  struct ConstraintKeyInfo;

  // Constraint - Objects of this structure are used to represent the various
  // constraints identified by the algorithm.  The constraints are 'copy',
  // for statements like "A = B", 'load' for statements like "A = *B",
  // 'store' for statements like "*A = B", and AddressOf for statements like
  // A = alloca;  The Offset is applied as *(A + K) = B for stores,
  // A = *(B + K) for loads, and A = B + K for copies.  It is
  // illegal on addressof constraints (because it is statically
  // resolvable to A = &C where C = B + K)
  //
  struct Constraint {
    enum ConstraintType { Copy, Load, Store, AddressOf } Type;
    unsigned Dest;
    unsigned Src;
    unsigned Offset;
  
    Constraint(ConstraintType Ty, unsigned D, unsigned S, unsigned O = 0)
      : Type(Ty), Dest(D), Src(S), Offset(O) {
      assert((Offset == 0 || Ty != AddressOf) &&
             "Offset is illegal on addressof constraints");
    }
  
    bool operator==(const Constraint &RHS) const {
      return RHS.Type == Type
        && RHS.Dest == Dest
        && RHS.Src == Src
        && RHS.Offset == Offset;
    }
  
    bool operator!=(const Constraint &RHS) const {
      return !(*this == RHS);
    }
  
    bool operator<(const Constraint &RHS) const {
      if (RHS.Type != Type)
        return RHS.Type < Type;
      else if (RHS.Dest != Dest)
        return RHS.Dest < Dest;
      else if (RHS.Src != Src)
        return RHS.Src < Src;
      return RHS.Offset < Offset;
    }
  };

  // Node class - This class is used to represent a node in the constraint
  // graph.  Due to various optimizations, it is not always the case that
  // there is a mapping from a Node to a Value.  In particular, we add
  // artificial Node's that represent the set of pointed-to variables shared
  // for each location equivalent Node.
  struct Node {
  public:
    Value *Val;
    SparseBitVector<> *Edges;
    SparseBitVector<> *PointsTo;
    SparseBitVector<> *OldPointsTo;
    std::list<Constraint> Constraints;

    // The incoming edges and outgoing edges are used by the escape analysis.
    SparseBitVector<> *InEdges;
    SparseBitVector<> *OutEdges;
    // The reversed points to set
    SparseBitVector<> *RevPointsTo;
    unsigned int EscFlag;

    // Pointer and location equivalence labels
    unsigned PointerEquivLabel;
    unsigned LocationEquivLabel;
    // Predecessor edges, both real and implicit
    SparseBitVector<> *PredEdges;
    SparseBitVector<> *ImplicitPredEdges;
    // Set of nodes that point to us, only use for location equivalence.
    SparseBitVector<> *PointedToBy;
    // Number of incoming edges, used during variable substitution to early
    // free the points-to sets
    unsigned NumInEdges;
    // True if our points-to set is in the Set2PEClass map
    bool StoredInHash;
    // True if our node has no indirect constraints (complex or otherwise)
    bool Direct;
    // True if the node is address taken, *or* it is part of a group of nodes
    // that must be kept together.  This is set to true for functions and
    // their arg nodes, which must be kept at the same position relative to
    // their base function node.
    bool AddressTaken;

    // True is the Value object that originally tracked this node has been
    // removed from the IR, causing this node to no longer be valid.
    bool Invalidated;

    // Nodes in cycles (or in equivalence classes) are united together using a
    // standard union-find representation with path compression.  NodeRep
    // gives the index into GraphNodes for the representative Node.
    unsigned NodeRep;

    // Modification timestamp.  Assigned from Counter.
    // Used for work list prioritization.
    unsigned Timestamp;

    explicit Node(bool direct = true)
        : Val(0), Edges(0), PointsTo(0), OldPointsTo(0), InEdges(nullptr),
          OutEdges(nullptr), RevPointsTo(nullptr), EscFlag(0),
          PointerEquivLabel(0), LocationEquivLabel(0), PredEdges(0),
          ImplicitPredEdges(0), PointedToBy(0), NumInEdges(0),
          StoredInHash(false), Direct(direct), AddressTaken(false),
          Invalidated(false), NodeRep((unsigned)-1), Timestamp(0) {}

    Node *setValue(Value *V) {
      assert(V == nullptr ||
          (Val == nullptr && "Value already set for this node!"));
      Val = V;
      return this;
    }

    /// getValue - Return the LLVM value corresponding to this node.
    ///
    Value *getValue() const { return Val; }

    /// setInvalidated - Mark the node as no longer tracking a valid object.
    void setInvalidated() {
      Invalidated = true;
    }

    /// getInvalidated - Check whether the node trackes a valid object.
    bool getInvalidated() const {
      return Invalidated;
    }

    /// addPointerTo - Add a pointer to the list of pointees of this node,
    /// returning true if this caused a new pointer to be added, or false if
    /// we already knew about the points-to relation.
    bool addPointerTo(unsigned Node) {
      return PointsTo->test_and_set(Node);
    }

    bool addRevPointerto(unsigned Node) {
      if (!RevPointsTo)
        RevPointsTo = new SparseBitVector<>;
      return RevPointsTo->test_and_set(Node);
    }

    /// intersects - Return true if the points-to set of this node intersects
    /// with the points-to set of the specified node.
    bool intersects(Node *N) const;

    /// intersectsIgnoring - Return true if the points-to set of this node
    /// intersects with the points-to set of the specified node on any nodes
    /// except for the specified node to ignore.
    bool intersectsIgnoring(Node *N, unsigned) const;

    // Timestamp a node (used for work list prioritization)
    void Stamp() {
      // Initialize Timestamp Counter (static).
      static std::atomic<unsigned> Counter (0);

      Timestamp = ++Counter;
      --Timestamp;
    }

    bool isRep() const {
      return( (int) NodeRep < 0 );
    }
  };

  typedef std::set<Node *> NodeSetTy;

  struct PairKeyInfo;

  struct WorkListElement {
    Node* node;
    unsigned Timestamp;
    WorkListElement(Node* n, unsigned t);
    bool operator<(const WorkListElement& that) const;
  };

  class WorkList {
      std::priority_queue<WorkListElement> Q;

    public:
      void insert(Node* n);
      Node* pop();
      bool empty();
  };

  const DataLayout &DL;
  AndersGetTLITy GetTLI;

  // This flag indicates whether wholeprogram safe is true or not.
  // This is computed from WholeProgramAnalysis. This is currently
  // used by alias queries that are invoked after Andersens analysis
  // is done.
  bool WholeProgramSafeDetected = false;

  std::set<unsigned> PossibleSourceOfPointsToInfo;

  std::vector<CallSite> IndirectCallList;
  std::vector<CallSite> DirectCallList;

  // GraphNodes - This vector is populated as part of the object
  // identification stage of the analysis, which populates this vector with a
  // node for each memory object and fills in the ValueNodes map.
  std::vector<Node> GraphNodes;

  // ValueNodes - This map indicates the Node that a particular Value* is
  // represented by.  This contains entries for all pointers.
  DenseMap<Value*, unsigned> ValueNodes;

  // ObjectNodes - This map contains entries for each memory object in the
  // program: globals, alloca's and mallocs.
  DenseMap<Value*, unsigned> ObjectNodes;

  // ReturnNodes - This map contains an entry for each function in the
  // program that returns a value.
  DenseMap<Function*, unsigned> ReturnNodes;

  // VarargNodes - This map contains the entry used to represent all pointers
  // passed through the varargs portion of a function call for a particular
  // function.  An entry is not present in this map for functions that do not
  // take variable arguments.
  DenseMap<Function*, unsigned> VarargNodes;

  // Constraints - This vector contains a list of all of the constraints
  // identified by the program.
  std::vector<Constraint> Constraints;

  std::list<unsigned> NodeWorkList;

  // Map from graph node to maximum K value that is allowed (for functions,
  // this is equivalent to the number of arguments + CallFirstArgPos)
  std::map<unsigned, unsigned> MaxK;

  // This enum defines the GraphNodes indices that correspond to important
  // fixed sets.
  enum {
    UniversalSet = 0,
    NullPtr      = 1,
    NullObject   = 2,
    NumberSpecialNodes
  };

  // Stack for Tarjan's
  std::stack<unsigned> SCCStack;

  // Map from Graph Node to DFS number
  std::vector<unsigned> Node2DFS;

  // Map from Graph Node to Deleted from graph.
  std::vector<bool> Node2Deleted;

  // Same as Node Maps, but implemented as std::map because it is faster to
  // clear
  std::map<unsigned, unsigned> Tarjan2DFS;
  std::map<unsigned, bool> Tarjan2Deleted;

  // Current DFS number
  unsigned DFSNumber = 0;

  // Work lists.
  WorkList w1, w2;

  // "current" and "next" work lists
  WorkList *CurrWL = nullptr;
  WorkList *NextWL = nullptr;

  // Temporary rep storage, used because we can't collapse SCC's in the
  // predecessor graph by uniting the variables permanently, we can only do so
  // for the successor graph.
  std::vector<unsigned> VSSCCRep;

  // Mapping from node to whether we have visited it during SCC finding yet.
  std::vector<bool> Node2Visited;

  // During variable substitution, we create unknowns to represent the unknown
  // value that is a dereference of a variable.  These nodes are known as
  // "ref" nodes (since they represent the value of dereferences).
  unsigned FirstRefNode = 0;

  // During HVN, we create represent address taken nodes as if they were
  // unknown (since HVN, unlike HU, does not evaluate unions).
  unsigned FirstAdrNode = 0;

  // Current pointer equivalence class number
  unsigned PEClass = 0;

  // Mapping from points-to sets to equivalence classes
  typedef DenseMap<SparseBitVector<> *, unsigned, BitmapKeyInfo> BitVectorMap;
  BitVectorMap Set2PEClass;

  // Mapping from pointer equivalences to the representative node.  -1 if we
  // have no representative node for this pointer equivalence class yet.
  std::vector<int> PEClass2Node;

  // Mapping from pointer equivalences to representative node.  This includes
  // pointer equivalent but not location equivalent variables. -1 if we have
  // no representative node for this pointer equivalence class yet.
  std::vector<int> PENLEClass2Node;

  // Union/Find for HCD
  std::vector<unsigned> HCDSCCRep;

  // HCD's offline-detected cycles; "Statically DeTected"
  // -1 if not part of such a cycle, otherwise a representative node.
  std::vector<int> SDT;

  // Whether to use SDT (UniteNodes can use it during solving, but not before)
  bool SDTActive = false;

  // Skip doing Andersens Analysis if it finds unexpected Insts.
  bool SkipAndersensAnalysis = false;

  // The data structure to record the static global variable
  // which are not escaped from the current routine.
  SmallPtrSet<const Value *, 16> NonEscapeStaticVars;
  SmallPtrSet<const Instruction *, 16> NonPointerAssignments;

  /// Handle to clear this analysis on deletion of values.
  struct AndersensDeletionCallbackHandle final : CallbackVH {
    AndersensAAResult &AAR;

    AndersensDeletionCallbackHandle(AndersensAAResult &AAR, Value *V)
        : CallbackVH(V), AAR(AAR) {}

    void deleted() override {
      Value *Val = getValPtr();

      AAR.ProcessIRValueDestructed(Val);

      // Clear the value handle, so that the object can be destroyed.
      setValPtr(nullptr);
      //AAR.AndersensHandles.erase(Val)

    }
  };

  // Helper class for Intel style mod/ref sets. This is the external module
  // interface used by the AndersensAAResult.
  class IntelModRef {
  public:
    IntelModRef(AndersensAAResult *AnderAA, AndersGetTLITy GetTLI);
    ~IntelModRef();

    void runAnalysis(Module &M);
    ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc,
                             AAQueryInfo &AAQI);

    // When the AndersenAAResult is moved to a new object, the handle stored
    // within this class needs to be updated so that calls can be made to query
    // alias information.
    void resetAndersenAAResult(AndersensAAResult *AnderAA);
  private:
    // Pointer to implementation idiom
    IntelModRefImpl *Impl;
  };

  std::unique_ptr<IntelModRef> IMR;
  // Interface for IntelModRef collection.
  enum {
    PointsToValue = 0x1,
    PointsToNonLocalLoc = 0x2,
    PointsToBottom = 0x4
  };

  unsigned getPointsToSet(const Value *V,
    std::vector<llvm::Value*>& PtVec);

  // List of callbacks for Values being tracked by this analysis.
  std::set<AndersensDeletionCallbackHandle> AndersensHandles;

  explicit AndersensAAResult(const DataLayout &DL, AndersGetTLITy GetTLI,
                             WholeProgramInfo *WPInfo);

public:
  AndersensAAResult(AndersensAAResult &&Arg);

  enum AndersenSetResult {
    Complete,                // All targets have the same type
    PartiallyComplete,       // All targets have the same or similar types
    Incomplete,              // At least one target is unsafe or invalid
  };

  static AndersensAAResult analyzeModule(Module &M, AndersGetTLITy GetTLI,
              CallGraph &CG, WholeProgramInfo *WPInfo);

  // Interface routine to get possible targets of function pointers
  AndersenSetResult GetFuncPointerPossibleTargets(Value *FP, 
              std::vector<llvm::Value*>& Targets, CallSite CS, bool Trace);

  //------------------------------------------------
  // Implement the AliasAnalysis API
  //
  AliasResult alias(const MemoryLocation &LocA, const MemoryLocation &LocB,
                    AAQueryInfo &AAQI);

  ModRefInfo getModRefInfo(const CallBase *Call, const MemoryLocation &Loc,
                           AAQueryInfo &AAQI);
  ModRefInfo getModRefInfo(const CallBase *Call1, const CallBase *Call2,
                           AAQueryInfo &AAQI);

  // Return 'true' if the memory location may escape.
  bool mayEscape(const MemoryLocation &LocB);

  // Chases pointers until we find a (constant global) or not.
  bool pointsToConstantMemory(const MemoryLocation &Loc, AAQueryInfo &AAQI,
                              bool OrLocal);
  // Returns true if the given value V does not escape from
  // the current routine.
  bool escapes(const Value *V);

private:
  bool findNameInTable(StringRef rname, const char** name_table);

  bool isTrackableType(Type *Ty) const;

  bool isAggregateOrVecType(Type *Ty) const;

  bool isPointsToType(Type *Ty) const;

  unsigned getNode(Value *V);

  unsigned getObject(Value *V) const;

  unsigned getReturnNode(Function *F) const;

  unsigned getVarargNode(Function *F) const;

  unsigned getNodeValue(Value &V);

  unsigned UniteNodes(unsigned First, unsigned Second,
                      bool UnionByRank = true);
  unsigned FindNode(unsigned Node);
  unsigned FindNode(unsigned Node) const;

  void RunAndersensAnalysis(Module &M);
  void IdentifyObjects(Module &M);
  void CollectConstraints(Module &M);
  void CreateConstraintGraph();
  void OptimizeConstraints();
  void CollectPossibleIndirectNodes();
  unsigned FindEquivalentNode(unsigned, unsigned);
  void ClumpAddressTaken();
  void RewriteConstraints();
  void HU();
  void HVN();
  void HCD();
  void Search(unsigned Node);
  void UnitePointerEquivalences();
  void SolveConstraints();
  bool QueryNode(unsigned Node);
  void Condense(unsigned Node);
  void HUValNum(unsigned Node);
  void HVNValNum(unsigned Node);
  unsigned getNodeForConstantPointer(Constant *C);
  unsigned getNodeForConstantPointerTarget(Constant *C);
  void AddGlobalInitializerConstraints(unsigned, Constant *C);
  void CreateConstraint(Constraint::ConstraintType Ty, unsigned D, 
                        unsigned S, unsigned O);

  void AddConstraintsForNonInternalLinkage(Function *F);
  void AddConstraintsForCall(CallSite CS, Function *F);
  bool AddConstraintsForExternalCall(CallSite CS, Function *F);
  void AddConstraintsForDirectCall(CallSite CS, Function *F);
  void AddConstraintsForInitActualsToUniversalSet(CallSite CS);
  void IndirectCallActualsToFormals(CallSite CS, Function *F);
  void InitIndirectCallActualsToUniversalSet(CallSite CS);
  void AddEdgeInGraph(unsigned N1, unsigned N2);

  // Return true if the type of a function pointer (FPType) matches with
  // the type of the target (TargetType).
  bool isSimilarType(Type *FPType, Type *TargetType,
      DenseSet<std::pair<Type *, Type *>> &TypesUsed);

  bool IsLibFunction(const Function *F);
  void CreateInOutEdgesforNodes();
  void CreateRevPointsToGraph();
  void CallSitesAnalysis();
  void AddToWorkList(unsigned int NodeIdx);
  void NewHoldingNode(unsigned int NodeIdx, unsigned int Flags);
  void ProcessHoldingNode(unsigned int NodeIdx);
  void NewPropNode(unsigned int NodeIdx, unsigned int Flags);
  void ProcessPropNode(unsigned int NodeIdx);
  void NewOpaqueNode(unsigned int NodeIdx, unsigned int Flags);
  void ProcessOpaqueNode(unsigned int NodeIdx);
  void InitEscAnal(Module &M);
  void InitEscAnalForGlobals(Module &M);
  unsigned int FindFlags(unsigned NodeIdx);
  void AddFlags(unsigned NodeIdx, unsigned int F);
  void PerformEscAnal(Module &M);
  void MarkEscaped();
  void ProcessCall(CallSite &CS);

  // Update the information stored for the points-to graph to remove
  // tracking of the value because the object in the IR that represents is
  // is being destroyed.
  void ProcessIRValueDestructed(Value *V);

  // Return true if one of the point-to targets escapes
  bool pointsToSetEscapes(Node *N);
  // Return true if the value represented by the graph node N escapes
  bool graphNodeEscapes(Node *N);
  // Analyze whether the given global escapes or not
  bool analyzeGlobalEscape(const Value *V,
               SmallPtrSet<const PHINode *, 16> PhiUsers,
               const Function **SinlgeAcessingFunction);
  void PrintNonEscapes() const;

  void ProcessIndirectCall(CallSite CS);
  void ProcessIndirectCalls(void);
  void PrintNode(const Node *N) const;
  void PrintConstraints() const ;
  void PrintConstraint(const Constraint &) const;
  void PrintLabels() const;
  void PrintPointsToGraph() const;
  void printValueNode(const Value *V);

  // Instruction visitation methods for adding constraints
  friend class InstVisitor<AndersensAAResult>;
  void visitReturnInst(ReturnInst &RI);
  void visitInvokeInst(InvokeInst &II) { visitCallSite(CallSite(&II)); }
  void visitAddressInst(AddressInst &AI);
  void visitCallInst(CallInst &CI) { visitCallSite(CallSite(&CI)); }
  void visitCallSite(CallSite CS);
  void visitAllocaInst(AllocaInst &AI);
  void visitLoadInst(LoadInst &LI);
  void visitStoreInst(StoreInst &SI);
  void visitGetElementPtrInst(GetElementPtrInst &GEP);
  void visitPHINode(PHINode &PN);
  void visitCastInst(CastInst &CI);
  void visitICmpInst(ICmpInst &ICI) {}
  void visitFCmpInst(FCmpInst &ICI) {} 
  void visitSelectInst(SelectInst &SI);
  void visitVAArg(VAArgInst &I);
  void visitInstruction(Instruction &I);
  void visitInsertValueInst(InsertValueInst &AI);
  void visitExtractValueInst(ExtractValueInst &AI);
  void visitAtomicRMWInst(AtomicRMWInst &AI);
  void visitPtrToIntInst(PtrToIntInst &AI);
  void visitIntToPtrInst(IntToPtrInst &AI);
  void visitBinaryOperator(BinaryOperator &AI);
  void visitExtractElementInst(ExtractElementInst &AI);
  void visitInsertElementInst(InsertElementInst &AI);
  void visitShuffleVectorInst(ShuffleVectorInst &AI);
  void visitLandingPadInst(LandingPadInst &AI);
  void visitAtomicCmpXchgInst(AtomicCmpXchgInst &AI);
  void visitCleanupPadInst(CleanupPadInst &AI);
  void visitCatchPadInst(CatchPadInst &AI);
  void visitCleanupReturnInst(CleanupReturnInst &AI);
  void visitCatchReturnInst(CatchReturnInst &AI);

  void processWinEhOperands(Instruction &AI);
 
};

/// Analysis pass providing a never-invalidated alias analysis result.
class AndersensAA : public AnalysisInfoMixin<AndersensAA> {
  friend AnalysisInfoMixin<AndersensAA>;
  static AnalysisKey Key;

public:
  typedef AndersensAAResult Result;

  AndersensAAResult run(Module &M, ModuleAnalysisManager &AM);
};

/// Legacy wrapper pass to provide the AndersensAAResult object.
class AndersensAAWrapperPass : public ModulePass {
  std::unique_ptr<AndersensAAResult> Result;

public:
  static char ID;

  AndersensAAWrapperPass();

  AndersensAAResult &getResult() { return *Result; }
  const AndersensAAResult &getResult() const { return *Result; }

  bool runOnModule(Module &M) override;
  bool doFinalization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

//===--------------------------------------------------------------------===//
//
// createAndersensAAWrapperPass - This pass provides alias info
// using Andersens points-to analysis.
//
ModulePass *createAndersensAAWrapperPass();
}

#endif
