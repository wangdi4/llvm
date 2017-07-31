//===- OptVLS.cpp - Optimization of Vector Loads/Stores ----------*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
///
/// OptVLS performs two optimizations:
/// 1. Replaces a set of complex loads/stores(indexed, strided) by a set of
///    simple loads/stores(contiguous) followed by shuffle/permute
/// 2. Replaces a set of overlapping accesses by a set of fewer loads/stores
///    followed by shuffle/permute.
///
/// This file defines an implementation of optimization of vector loads/stores.
/// The core functionality of Opt_VLS is provided by three functions:
/// getGroups(),
/// getCost(), getSequence(). This implementation is IR and machine agnostic.
/// For any required information to compute groups, cost and sequence OptVLS
/// communicates to its clients.
///
//===---------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptVLS.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#define DEBUG_TYPE "ovls"
#include <deque>
#include <set>
using namespace llvm;

// MemrefDistanceMap contains a set of distances where each distance gets
// mapped to a memref. Basically, it contains a set of adjacent memrefs.
// Keeping them in a map helps eliminate any duplicate distances and keeps them
// sorted.
typedef OVLSMap<int, OVLSMemref *> MemrefDistanceMap;
typedef MemrefDistanceMap::iterator MemrefDistanceMapIt;

// MemrefDistanceMapVector is a vector of groups where each group contains a set
// of adjacent memrefs.
typedef OVLSVector<MemrefDistanceMap *> MemrefDistanceMapVector;
typedef MemrefDistanceMapVector::iterator MemrefDistanceMapVectorIt;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void OVLSAccessType::dump() const {
  print(OVLSdbgs());
  OVLSdbgs() << '\n';
}
#endif

void OVLSAccessType::print(OVLSostream &OS) const {
  switch (AccType) {
  case SLoad:
    OS << "SLoad";
    break;
  case SStore:
    OS << "SStore";
    break;
  case ILoad:
    OS << "ILoad";
    break;
  case IStore:
    OS << "IStore";
    break;
  default:
    OS << "Unknown";
    break;
  }
}

OVLSMemref::OVLSMemref(OVLSMemrefKind K, OVLSType T,
                       const OVLSAccessType &AType)
    : Kind(K), AccType(AType) {
  DType = T;
  static unsigned MemrefId = 1;
  Id = MemrefId++;

  if (AccType.isUnknown()) {
    OVLSDebug(OVLSdbgs() << "#" << Id << " ");
    assert("Invalid AccType!!");
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void OVLSMemref::dump() const {
  print(OVLSdbgs(), 0);
  OVLSdbgs() << '\n';
}
#endif

void OVLSMemref::print(OVLSostream &OS, unsigned NumSpaces) const {
  unsigned Counter = 0;
  while (Counter++ != NumSpaces)
    OS << " ";

  // print Id
  OS << "#" << getId();

  // print memref type
  OS << " ";
  OS << getType();

  // print accessType
  OS << " ";
  getAccessType().print(OS);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void OVLSGroup::dump() const {
  print(OVLSdbgs(), 0);
  OVLSdbgs() << '\n';
}
#endif

uint64_t OVLSCostModel::getShuffleCost(SmallVectorImpl<int> &Mask,
                                       Type *Tp) const {
  int index = 0;
  unsigned NumSubVecElems = 0;
  unsigned NumVecElems = Tp->getVectorNumElements();
  assert(NumVecElems == Mask.size() && "Mismatched vector elements!!");

  if (isExtractSubvectorMask(Mask)) {
    // TODO: Support other sized subvectors.
    index = Mask[0] == 0 ? 0 : 1;
    return TTI.getShuffleCost(
        TargetTransformInfo::SK_ExtractSubvector, Tp, index,
        VectorType::get(Tp->getScalarType(), NumVecElems / 2));
  } else if (isInsertSubvectorMask(Mask, index, NumSubVecElems))
    return TTI.getShuffleCost(
        TargetTransformInfo::SK_InsertSubvector, Tp, index,
        VectorType::get(Tp->getScalarType(), NumSubVecElems));
  else if (TTI.isTargetSpecificShuffleMask(Mask))
    return TTI.getShuffleCost(TargetTransformInfo::SK_TargetSpecific, Tp, 0,
                              nullptr);
  else if (isReverseVectorMask(Mask))
    return TTI.getShuffleCost(TargetTransformInfo::SK_Reverse, Tp, 0, nullptr);
  else if (isAlternateVectorMask(Mask))
    return TTI.getShuffleCost(TargetTransformInfo::SK_Alternate, Tp, 0,
                              nullptr);

  // TODO: Support SK_Insert
  uint32_t TotalElems = Mask.size();
  for (int MaskElem : Mask)
    if (MaskElem < 0)
      TotalElems--;

  return 2 * TotalElems;
}

namespace OptVLS {
class GraphNode;
typedef std::list<GraphNode *> GraphNodeList;
typedef OVLSMap<GraphNode *, OVLSMemref *> GraphNodeToOVLSMemrefMap;

/// Represents a range of bits using a bit-location of the leftmost bit and
/// a number of consecutive bits immediately to the right that are included
/// in the range. {0, 0} means undefined bit-range.
class BitRange {
  uint32_t BIndex;
  uint32_t NumBits;

public:
  BitRange(uint32_t BI, uint32_t NBits) {
    assert(BI % BYTE == 0 &&
           "BitIndex in a BitRange needs to be divisible by a byte size!!!");
    assert(NBits % BYTE == 0 &&
           "NumBits in a BitRange needs to be divisible by a byte size!!!");
    BIndex = BI;
    NumBits = NBits;
  }

  // Increment BitIndex by NumBits.
  BitRange &operator++() {
    BIndex += NumBits;
    return *this;
  }
  uint32_t getNumBits() const { return NumBits; }
  uint32_t getBitIndex() const { return BIndex; }
  void updateBitIndex(uint32_t NewBIndex) { BIndex = NewBIndex; }
};

/// print BitRange as "Start_BitIndex : End_BitIndex"
static inline OVLSostream &operator<<(OVLSostream &OS, const BitRange &BR) {
  OS << BR.getBitIndex() << ":" << BR.getBitIndex() + BR.getNumBits() - 1;
  return OS;
}

/// Edge represents a move of a specified bit-range 'BR' from 'Src' to 'Dst'.
/// Src can be nullptr, which means an undefined source. For an undefined
/// source, BR still represents a valid bitrange. A bit-range with an undefined
/// source is used to represent a gap in the destination GraphNode.
class Edge {
private:
  GraphNode *Src;
  GraphNode *Dst;
  BitRange BR;
  uint32_t DstBitIndex;

public:
  Edge(GraphNode *S, GraphNode *D, BitRange BRange, uint32_t DBIdx)
      : Src(S), Dst(D), BR(BRange), DstBitIndex(DBIdx) {
    BR = BitRange(BRange.getBitIndex(), BRange.getNumBits());
  }

  GraphNode *getSource() const { return Src; }
  GraphNode *getDest() const { return Dst; }
  BitRange getBitRange() const { return BR; }
  uint32_t getBitIndex() const { return BR.getBitIndex(); }
  uint32_t getNumBits() const { return BR.getNumBits(); }
  uint32_t getDstBitIndex() const { return DstBitIndex; }

  void updateSource(GraphNode *NewSrc, uint32_t NewBIndex) {
    Src = NewSrc;
    // Update BitIndex.
    BR.updateBitIndex(NewBIndex);
  }
  void updateDest(GraphNode *NewDest) { Dst = NewDest; }
};

/// GraphNode can be thought of as a result of some logical instruction
/// (mainly rearrangement instruction such as shift, shuffle, etc) on
/// its ‘IncomingEdges’(/source bit-ranges). These ‘IncomingEdges’
/// particularly show which source bit-range maps to which bit-index of this
/// (which helps defining (/elaborates on) the logical instruction semantics).
/// A ‘GraphNode’ basically allows us to define an expected behavior
/// (/semantic) first which then evolves into a particular valid
/// OVLSinstruction ‘Inst’ if there is any for that semantic. A node is
/// allowed to have any permutation of bit-ranges coming from a maximum of
/// two source nodes.
class GraphNode {
  /// Provides a unique id to each instruction node. It helps printing
  /// tracable node information.
  uint32_t Id;

  /// Initially when a GraphNode is created, Inst can be nullptr
  /// which means undefined instruction. An undefined instruction can
  /// still have valid IncomingEdges which would define the semantics of
  /// this logical instruction (GraphNode), helps specifying the actual
  /// instruction later.
  /// A GraphNode is also used for holding the result of a load/store
  /// instruction, in such case, Inst should point to a valid load/store
  /// instruction.
  OVLSInstruction *Inst;

  /// A ‘GraphNode’ is a result of some logical instruction on its incoming
  /// edges where ‘IncomingEdges’ contains that result. The output value of
  /// the GraphNode is the concatenation of the source bit-ranges which shows
  /// which source bit-range maps to which bit index of this node. Depending
  /// on the order of the edges (in IncomingEdges) that bitindex gets
  /// determined. Multiple edges can be drawn between two nodes with different
  /// bit ranges. When there are no edges to a certain bit-index, a dummy edge
  /// (an edge with Src=nullptr) gets inserted into IncomingEdges to represent
  /// the whole. IncomingEdges for a memory instruction can be empty.
  OVLSVector<Edge *> IncomingEdges;
  OVLSVector<Edge *> OutgoingEdges;

  /// Current total number of incoming bits from IncomingEdges.
  uint32_t TotalIncomingBits;

  /// Keeps track of the number of incoming edges. The main purpose of this is
  /// to allow liner time topological node traversal.
  uint32_t TotalIncomingEdges;

  /// Type of the result node gets set during the output node(gather node)
  /// construction. In order to generate an OVLSInstruction we will
  /// need an OVLSType. It is important that we generate the right typed
  /// output node(gather-node). Having to deduce it from the incoming bits
  /// might be erroneous due to mixed typed group of memrefs. If we know the
  /// expected type we will be able to bitcast the source-nodes if need be.
  OVLSType Type;

public:
  explicit GraphNode(OVLSInstruction *I, OVLSType T) : Inst(I), Type(T) {
    static uint32_t NodeId = 1;
    Id = NodeId++;
    TotalIncomingBits = 0;
    TotalIncomingEdges = 0;
  }

  ~GraphNode() {
    for (Edge *E : IncomingEdges)
      delete E;
  }

  typedef OVLSVector<Edge *>::iterator iterator;
  inline iterator begin() { return IncomingEdges.begin(); }
  inline iterator end() { return IncomingEdges.end(); }

  typedef OVLSVector<Edge *>::const_iterator const_iterator;
  inline const_iterator begin() const { return IncomingEdges.begin(); }
  inline const_iterator end() const { return IncomingEdges.end(); }

  inline iterator_range<const_iterator> incomingEdges() const {
    return make_range(begin(), end());
  }
  inline iterator_range<const_iterator> outgoingEdges() const {
    return make_range(OutgoingEdges.begin(), OutgoingEdges.end());
  }

  uint32_t getId() const { return Id; }
  OVLSInstruction *getInstruction() const { return Inst; }
  bool isALoad() const { return Inst && isa<OVLSLoad>(Inst); }
  bool isAStore() const { return Inst && isa<OVLSStore>(Inst); }
  bool isUndefined() const { return Inst == nullptr; }
  OVLSType type() const { return Type; }
  void setType(OVLSType T) { Type = T; }

  void setTotalIncomingEdges() { TotalIncomingEdges = IncomingEdges.size(); }
  void decrementTotalIncomingEdges() { TotalIncomingEdges--; }

  uint32_t getTotalIncomingEdges() const { return TotalIncomingEdges; }

  bool hasNoOutgoingEdges() const { return OutgoingEdges.size() == 0; }
  uint32_t getNumIncomingEdges() const { return IncomingEdges.size(); }
  void clearEdges() {
    IncomingEdges.clear();
    OutgoingEdges.clear();
  }

  // Update the value of its instruction. It asserts if it's not a store
  // instruction. Here the value is computed from the incoming node.
  void updateStoredValue() {
    std::set<GraphNode *> UniqueSources;
    assert(getNumUniqueSources(UniqueSources) == 1 && "Unexpected sources!!!");
    std::set<GraphNode *>::iterator It = UniqueSources.begin();
    OVLSStore *StoreInst = dyn_cast<OVLSStore>(Inst);
    assert(StoreInst && "Expected Store Inst!");
    StoreInst->updateValue(((*It)->getInstruction()));
  }

  // Returns the current total number of incoming bits.
  uint32_t size() const { return TotalIncomingBits; }

  // print GraphNode;
  void print(OVLSostream &OS, uint32_t NumSpaces) const {
    uint32_t Counter = 0;
    while (Counter++ != NumSpaces)
      OS << " ";
    OS << "V" << Id << ":";

    // print the associated instruction.
    if (isALoad())
      OS << " Load ";

    OS << "\n";
    // print the incoming edges as [BR] = Srd-Id[BR].
    uint32_t CurrentBitIndex = 0;
    for (Edge *E : IncomingEdges) {
      Counter = 0;
      while (Counter++ != NumSpaces)
        OS << " ";

      BitRange SrcBR = E->getBitRange();
      BitRange DstBR = BitRange(CurrentBitIndex, SrcBR.getNumBits());
      OS << " [" << DstBR << "] "
         << "= V";
      if (E->getSource()) {
        OS << E->getSource()->getId();
        OS << "[" << SrcBR << "]\n";
      } else
        OS << "UnDef";

      CurrentBitIndex += SrcBR.getNumBits();
    }
  }

  void removeOutgoingEdge(Edge *E) {
    auto IT = std::find(OutgoingEdges.begin(), OutgoingEdges.end(), E);
    assert(IT != OutgoingEdges.end() && "Invalid edge to be removed!!!");
    OutgoingEdges.erase(IT);
  }

  // Add 'E' to the outgoing edge-list.
  void addAnOutgoingEdge(Edge *E) {
    assert(E != nullptr && "Invalid Edge!!!");
    OutgoingEdges.push_back(E);
  }

  // Assigns an edge 'E' to the BIndex if BIndex is available,
  // If BIndex is larger than the current bit index, it fills up the gap by
  // creating a dummy edge. This function does not allow overwriting an edge,
  // it throws an exception in such case.
  void addAnIncomingEdge(uint32_t BIndex, Edge *E) {
    assert(E != nullptr && "Invalid Edge!!!");
    assert(BIndex >= TotalIncomingBits &&
           "Overwriting an (element)edge is not allowed!!!");

    if (BIndex > TotalIncomingBits) {
      // BIndex is larger than the current bit-index, create a dummy edge
      // to fillup the gap.
      uint32_t GapSize = (BIndex - TotalIncomingBits);

      Edge *Gap =
          new Edge(nullptr, this, BitRange(0, GapSize), TotalIncomingBits);
      IncomingEdges.push_back(Gap);
      TotalIncomingBits += GapSize;
    }

    IncomingEdges.push_back(E);
    // Update current bit-index.
    TotalIncomingBits += E->getBitRange().getNumBits();
    TotalIncomingEdges++;
  }

  /// Returns the total number of unique source nodes.
  /// A node can have multiple edges coming from the same source node.
  /// Therefore, the size of IncomingEdges will not be equal to the number of
  /// unique source nodes.
  uint32_t getNumUniqueSources(std::set<GraphNode *> &UniqueSources) const {
    for (Edge *E : IncomingEdges)
      UniqueSources.insert(E->getSource());
    return UniqueSources.size();
  }

  /// Split the edge E:(src->dst) by inserting a NewNode such as
  /// src->NewNode->dst.
  ///  src        src
  ///   |          |
  ///   |    =>  NewNode
  ///  dst         |
  ///             dst
  void splitEdge(Edge *E, GraphNode *NewNode) {
    // Create an edge from src to NewNode.
    uint32_t NewNodeBIndex = NewNode->size();
    Edge *NewEdge =
        new Edge(E->getSource(), NewNode, E->getBitRange(), NewNodeBIndex);
    NewNode->addAnIncomingEdge(NewNodeBIndex, NewEdge);

    // Update outgoing edgelist of NewNode.
    NewNode->addAnOutgoingEdge(E);

    GraphNode *Src = E->getSource();

    // Remove this from Src's outgoing list.
    Src->removeOutgoingEdge(E);

    // Update E with the new source NewNode;
    // src->dst => NewNode->dst.
    E->updateSource(NewNode, NewNodeBIndex);

    // Update outgoing edge-list of src
    Src->addAnOutgoingEdge(NewEdge);
  }

  /// Split the unique source nodes into half by replacing this node
  /// (the destination node) by 3 nodes where 2 new nodes each has half the
  /// unique source nodes.
  ///                \\         //
  ///    \\ // =>  NewNode1  NewNode2
  ///     dst          \\      //
  ///                      dst
  /// Note, multiple edges coming from the same source node appear
  /// consecutively in the destination. Here is an example:
  /// src1(0, 1, 2, 3) src2(4, 5, 6, 7)
  /// dst could be dst(0, 3, 6) but not dst(0, 6, 3)
  bool splitSourceNodes(GraphNodeList &NewNodes) {
    std::set<GraphNode *> UniqueSources;
    uint32_t NumUniqueSources = getNumUniqueSources(UniqueSources);

    if (NumUniqueSources <= 2)
      return false;

    OVLSType T;
    GraphNode *NewNode1 = new GraphNode(nullptr, T);
    GraphNode *NewNode2 = new GraphNode(nullptr, T);

    NewNodes.push_back(NewNode1);
    NewNodes.push_back(NewNode2);

    uint32_t NumElem = 0, N1NumElem = 0;
    uint32_t FirstHalf = NumUniqueSources / 2;

    uint32_t TotalUniqueSources = 0;
    GraphNode *NewNode = NewNode1;
    GraphNode *PrevSrc = nullptr;
    std::set<GraphNode *> VisitedSources;
    for (Edge *E : IncomingEdges) {
      GraphNode *Src = E->getSource();
      std::set<GraphNode *>::iterator it = VisitedSources.find(Src);
      assert(it == VisitedSources.end() &&
             "Unexpected dispersed edges from the same source!!!");

      // Keep track of the unique sources.
      if (Src != PrevSrc) {
        TotalUniqueSources++;
        VisitedSources.insert(Src);
      }

      // Redirect the second half of the edges to the newnode2.
      if (TotalUniqueSources == FirstHalf + 1) {
        NewNode = NewNode2;
        N1NumElem = NumElem;
        NumElem = 0;
      }
      NumElem++;
      splitEdge(E, NewNode);
    }

    NewNode1->setType(OVLSType(Type.getElementSize(), N1NumElem));
    NewNode2->setType(OVLSType(Type.getElementSize(), NumElem));

    return true;
  }

  /// Creates a unique source for each incoming edge. An edge can be coming
  /// from a node that contains bit-fields other than the specified
  /// bit-filed(associated with the edge). This way simplifyEdges() creates
  /// singularity in the graph which gives optimizer the highest flexibility
  /// for finding the lowest cost combination.
  /// E.g.
  ///    V3             V3
  ///   |  |           |  |
  ///   |  |    =>    V4  V5
  ///   /   \         |    |
  ///  V1   V2        V1   V2
  /// TODO: Consider overlapping.
  void simplifyEdges(GraphNodeList &NewNodes) {
    for (Edge *E : IncomingEdges) {
      GraphNode *Src = E->getSource();
      if (Src) {
        OVLSType T = OVLSType(E->getNumBits(), 1);
        GraphNode *NewNode = new GraphNode(nullptr, T);
        splitEdge(E, NewNode);
        NewNodes.push_back(NewNode);
      }
    }
  }

  // Generate a shuffle instruction for this node.
  void genShuffle() {
    OVLSType T;
    OVLSOperand *Op1 = new OVLSUndef(T);
    OVLSOperand *Op2 = new OVLSUndef(T);

    OVLSConstant *ShuffleMask = nullptr;

    // Use 'Type' which is the type of this result node to define the
    // instruction
    uint32_t ElemSize = Type.getElementSize();
    uint32_t NumElems = Type.getNumElements();

    const uint32_t MaxNumElems = 256;
    assert(NumElems <= MaxNumElems && "Increase MaxNumElems");

    int32_t IntShuffleMask[MaxNumElems];
    uint32_t MaskIndex = 0;

    uint32_t Op2StartIndex;
    // Traverse through the incoming edges, collect the input vectors and the
    // correspondent vector indices
    for (Edge *E : IncomingEdges) {
      OVLSOperand *Src = E->getSource()->getInstruction();
      BitRange BR = E->getBitRange();

      // Temporarily assuming each edge moves exactly 'ElemSize' number of bits
      // and the BitIndex is divisibly by ElemSize.

      assert(BR.getBitIndex() % ElemSize == 0 &&
             "BitIndex needs to be divisible by"
             " element-size!");
      assert(BR.getNumBits() == ElemSize &&
             "Each edge should move element-size "
             "number of bits!!!");
      if (isa<OVLSUndef>(Src))
        IntShuffleMask[MaskIndex++] = -1;
      else {
        if (isa<OVLSUndef>(Op1)) {
          Op1 = Src;
          Op2StartIndex = Op1->getType().getNumElements();
        } else if (Src != Op1 && isa<OVLSUndef>(Op2))
          Op2 = Src;
        else if (Src != Op1 && Src != Op2)
          assert("Invalid number of operands for OVLSShuffle!!!");

        if (Src == Op1)
          IntShuffleMask[MaskIndex++] = BR.getBitIndex() / ElemSize;
        else
          IntShuffleMask[MaskIndex++] =
              (BR.getBitIndex() / ElemSize) + Op2StartIndex;
      }
    }

    assert(MaskIndex == NumElems && "IntShuffleMask got out of range!!!");
    if (isa<OVLSUndef>(Op2))
      Op2->setType(Op1->getType());

    ShuffleMask =
        new OVLSConstant(OVLSType(32, NumElems), (int8_t *)IntShuffleMask);
    Inst = new OVLSShuffle(Op1, Op2, ShuffleMask);
  } // end of genShuffle()

}; // end of GraphNode

/// This directed graph is used to automatically build the network (of
/// required instructions) of computing the result of a set of adjacent
/// gathers from a set of contiguous loads. In this directed graph, an edge
/// represents a move of a bit-range, and a node can be thought of as a logical
/// operation on incoming bit-ranges and corresponding result.
///
/// NEXT: describe how the graph is used to automatically compute the
/// rearrangement instructions.
class Graph {
  /// When a node is created, it gets pushed into the NodeVector. Therefore,
  /// nodes in the NodeVector don't maintain any order. A destination node could
  /// appear before a source node in the NodeVector. But each node gets an id
  /// which corresponds to the container's index. So, a node can easily be
  /// accessed by its id also.
  GraphNodeList Nodes;

  uint32_t VectorLength;

  const OVLSCostModel &CM;

  // \brief Holds the total number of load nodes in the graph.
  uint32_t TotalLoadNodes;

public:
  explicit Graph(uint32_t VLen, const OVLSCostModel &CostModel)
      : VectorLength(VLen), CM(CostModel), TotalLoadNodes(0) {}

  ~Graph() {
    for (GraphNode *N : Nodes)
      delete N;
  }
  void insert(GraphNode *N) {
    Nodes.push_back(N);
    if (N->isALoad())
      TotalLoadNodes++;
  }

  void removeNode(GraphNode *N) {
    Nodes.remove(N);
    if (N->isALoad())
      TotalLoadNodes--;
  }

  GraphNodeList::iterator begin() { return Nodes.begin(); }

  uint32_t getGSNumElements() {
    // We know that the first few nodes are the gather/scatter nodes.
    GraphNode *GSNode = *(begin());
    return GSNode->type().getNumElements();
  }
  // Return nodes in a topological order.
  void getTopSortedNodes(GraphNodeList &TopSortedNodes) const {
    // TopSortedNodes is an empty list that will contain the sorted elements.
    std::deque<GraphNode *> NodesToProcess;

    // Collect all the nodes with no incoming edges.
    for (GraphNode *Node : Nodes) {
      if (Node->getTotalIncomingEdges() == 0)
        NodesToProcess.push_back(Node);
    }

    // Process the nodes with no incoming edges.
    while (!NodesToProcess.empty()) {
      GraphNode *Node = NodesToProcess.front();
      NodesToProcess.pop_front();

      // Delete outgoing edges.
      for (Edge *E : Node->outgoingEdges()) {
        GraphNode *Dst = E->getDest();
        // Delete Edge
        Dst->decrementTotalIncomingEdges();
        // If the destination has no incoming edges push it to the
        // NodesToProcess.
        if (Dst->getTotalIncomingEdges() == 0)
          NodesToProcess.push_back(Dst);
      }

      // Push it to the TopSortedNodes
      TopSortedNodes.push_back(Node);
    }
    // Reset incoming edge count.
    for (GraphNode *Node : Nodes)
      Node->setTotalIncomingEdges();
  }

  // Print the nodes in a topological order. It prints a node only if its
  // producers are printed.
  void print(OVLSostream &OS, uint32_t NumSpaces) const {
    GraphNodeList TopSortedNodes;
    getTopSortedNodes(TopSortedNodes);

    for (GraphNode *N : TopSortedNodes) {
      N->print(OS, NumSpaces + 2);
      OS << "\n";
    }
  }

  /// Simplifies the graph into a singular-form and returns true, otherwise
  /// returns false. It creates a unique source for each incoming edge.
  /// This gives optimizer the highest flexibility
  /// in order to find the lowest cost combination.
  bool simplify() {
    GraphNodeList NewNodes;

    // Don't simplify if the graph has a single load-node. The outcome of
    // the simplification and then merge will be the same node as the load-node.
    // Which leaves the graph with an extra node.
    // Here is an exmaple of simplifying a graph with a single load-node. It
    // leaves
    // the final graph with v4 extra node that is same as the load-node.
    //    V3:Load                 V3:Load                     V3:Load
    //   |\  /|  After Simplify  |  |  |  |   After Merge      \ || /
    //   | \/ |       = >        V4 V5 V6 V7      =>             V4
    //   | /\ |                  | /    \/                      |/ \|
    //   V1  V2                  V1     V2                      V1  V2
    //
    if (TotalLoadNodes == 1)
      return false;

    // Don't simplify if the gather/scatter nodes have maximum 2 elements.
    // In that case, there is nothing to optimize(assuming we have maximum
    // two sources; we can extract maximum two elements using one instrution)
    // E.g.
    //   v3:L  v4:L
    //     |\  /|
    //     | \/ |
    //     | /\ |
    //     |/  \|
    //     V1   V2
    if (getGSNumElements() <= 2)
      return false;

    for (GraphNode *N : Nodes)
      N->simplifyEdges(NewNodes);

    for (GraphNode *NewNode : NewNodes)
      insert(NewNode);

    return true;
  }

  /// Split unique source nodes of shuffle nodes recursively until each node
  /// has no more than two unique source nodes.
  void reduceNSourcesToTwoSources() {
    for (GraphNode *N : Nodes) {
      GraphNodeList NewNodes;
      if (N->splitSourceNodes(NewNodes)) {
        // insert newly created nodes in to the graph.
        for (GraphNode *NewNode : NewNodes)
          insert(NewNode);
        NewNodes.clear();
      }
    }
  }

  /// When we are trying to merge two nodes, there are various ways to merge
  /// them.
  /// For example,
  /// N1:
  ///     [0:63] = V5[0:63]
  ///	    [64:127] = V6[0:63]
  /// N2:
  ///     [0:63] = V5[64:127]
  ///	    [64:127] = V6[64:127]
  /// We can merge them as N12: N1 N2 N1 N2, N1 N1 N2 N2, N2 N2 N1 N1,
  /// N2 N1 N2 N1, etc.
  /// It makes more sense to merge as N1 N1 N2 N2 which will most likely lead
  /// to vperm or vinsert.
  OVLSVector<int> getPossibleIncomingMask(const GraphNode &N1,
                                          const GraphNode &N2) const {
    OVLSVector<int> Mask;
    std::set<GraphNode *> UniqueSources;
    N1.getNumUniqueSources(UniqueSources);
    uint32_t NumUniqueSources = N2.getNumUniqueSources(UniqueSources);
    (void)NumUniqueSources;
    assert(NumUniqueSources <= 2 && "Invalid total sources!");

    std::set<GraphNode *>::iterator It = UniqueSources.begin();
    GraphNode *Src1 = *It;

    OVLSType SrcType = Src1->type();
    uint32_t S1StartIndex = 0;
    uint32_t S2StartIndex = SrcType.getNumElements();
    uint32_t ElemSize = SrcType.getElementSize();

    // Traverse through the edges of N1 and compute mask.
    for (const auto &Edge : N1.incomingEdges()) {
      GraphNode *Src = Edge->getSource();
      uint32_t BIndex = Edge->getBitRange().getBitIndex();

      if (Src == nullptr)
        Mask.push_back(-1);
      else if (Src == Src1)
        Mask.push_back(BIndex / ElemSize + S1StartIndex);
      else
        Mask.push_back(BIndex / ElemSize + S2StartIndex);
    }

    // Traverse through the edges of N2 and compute mask.
    for (const auto &Edge : N2.incomingEdges()) {
      GraphNode *Src = Edge->getSource();
      uint32_t BIndex = Edge->getBitRange().getBitIndex();

      if (Src == nullptr)
        Mask.push_back(-1);
      else if (Src == Src1)
        Mask.push_back(BIndex / ElemSize + S1StartIndex);
      else
        Mask.push_back(BIndex / ElemSize + S2StartIndex);
    }

    // Mask size needs to match the source size.
    while (Mask.size() < S2StartIndex)
      Mask.push_back(-1);

    return Mask;
  }

  /// Computes the masks created by the outgoing edges of N to its
  /// destinations. Here N's starting index starts from NodeStartIndex
  /// (in bits)
  ///
  /// For example,
  ///  V1[0:63] = N[0:63]
  ///  V2[0:63] = N[64:127]
  /// Computes the following two masks with NodeStartIndex 64:
  ///   V1-Mask: <1, -1, 1, -1>
  ///   V2-Mask: <2, -1, 1, -1>
  void getPossibleOutgoingMasks(
      const GraphNode &N, uint32_t NodeStartIndex,
      std::map<int, SmallVector<int, 16>> &NodeMaskMap) const {
    uint32_t ElemSize = N.type().getElementSize();
    for (const auto &E : N.outgoingEdges()) {
      GraphNode *Dst = E->getDest();
      uint32_t Id = E->getDest()->getId();
      BitRange BR = E->getBitRange();
      uint32_t SrcBitIndex = BR.getBitIndex();
      uint32_t NumBits = BR.getNumBits();
      OVLSType DstTy = Dst->type();
      uint32_t NumElems = DstTy.getNumElements();

      // According to the destination's element size.
      int32_t MaskIndex = E->getDstBitIndex() / DstTy.getElementSize();
      uint32_t SrcIndex = NodeStartIndex + SrcBitIndex;
      assert((SrcIndex % ElemSize == 0) && (NumBits % ElemSize == 0) &&
             "Unexpected BitIndex and/or NumBits");

      // According to the source's element size.
      uint32_t MaskElem = SrcIndex / ElemSize;

      std::map<int, SmallVector<int, 16>>::iterator MapIt =
          NodeMaskMap.find(Id);
      SmallVector<int, 16> Mask;
      if (MapIt == NodeMaskMap.end())
        // Mask size needs to match the source size.
        Mask = SmallVector<int, 16>(NumElems, -1);
      else
        Mask = MapIt->second;

      Mask[MaskIndex] = MaskElem;

      // An edge can move multiple elements and they are contiguous.
      while (NumBits -= ElemSize)
        Mask[++MaskIndex] = ++MaskElem;

      NodeMaskMap[Id] = Mask;
    }
  }

  /// Computes the masks created by the outgoing edges of merge(N1,N2) to its
  /// destinations.
  ///
  /// For example,
  ///  V1[0:63] = N1[0:63]
  ///  V2[128:192] = N2[0:63]
  /// Computes the following two masks:
  ///   V1-Mask: <0, -1, -1, -1>
  ///   V2-Mask: <-1, -1, 1, -1> // After merging the first element of N2
  ///                            // becomes the 2nd element of the merged N12.
  ///
  /// This needs to follow the merging pattern of getPossibleIncomingMask which
  /// assumes merging of N1 and N2 happened as N1 N1 .. N2 N2..
  SmallVector<SmallVector<int, 16>, 16>
  getPossibleOutgoingMergeMasks(const GraphNode &N1,
                                const GraphNode &N2) const {
    SmallVector<SmallVector<int, 16>, 16> Masks;
    std::map<int, SmallVector<int, 16>> NodeMaskMap;

    // Compute masks that are created by the outgoing edges of N1
    getPossibleOutgoingMasks(N1, 0, NodeMaskMap);

    // Compute masks that are created by the outgoing edges of N2
    getPossibleOutgoingMasks(N2, N1.type().getSize(), NodeMaskMap);

    std::map<int, SmallVector<int, 16>>::iterator MapIt, MapItE;

    for (MapIt = NodeMaskMap.begin(), MapItE = NodeMaskMap.end();
         MapIt != MapItE; ++MapIt)
      Masks.push_back(MapIt->second);

    return Masks;
  }

  // Returns the cost of merging N1 and N2.
  uint64_t getMergeCost(const GraphNode &N1, const GraphNode &N2) const {
    // Assumes N1 and N2 has the same element size.
    uint32_t ElemSize = N1.type().getElementSize();
    Type *ElemType = Type::getIntNTy(CM.getContext(), ElemSize);

    // Compute inward impact.
    SmallVector<int, 16> Mask = getPossibleIncomingMask(N1, N2);
    VectorType *VecTy = VectorType::get(ElemType, Mask.size());
    uint64_t Cost = CM.getShuffleCost(Mask, VecTy);

    // Compute outward impact.
    SmallVector<SmallVector<int, 16>, 16> Masks =
        getPossibleOutgoingMergeMasks(N1, N2);
    for (auto Mask : Masks) {
      VecTy = VectorType::get(ElemType, Mask.size());
      Cost += CM.getShuffleCost(Mask, VecTy);
    }

    return Cost;
  }

  /// N1 and N2 can be merged if
  ///  - Sources have the same type
  ///  - The total number of unique sources of N1 and N2 is no more than 2
  ///  - Total size of N1 and N2 fits into the vector register
  ///  - elem_size of N1 matches the elem_size of N2
  bool canBeMerged(const GraphNode &N1, const GraphNode &N2) const {
    if (N1.type().getElementSize() != N2.type().getElementSize())
      return false;

    if (N1.size() == 0 || N2.size() == 0 ||
        N1.size() + N2.size() > VectorLength * BYTE)
      return false;

    std::set<GraphNode *> UniqueSources;
    N1.getNumUniqueSources(UniqueSources);
    uint32_t NumUniqueSources = N2.getNumUniqueSources(UniqueSources);

    if (NumUniqueSources > 2)
      return false;

    std::set<GraphNode *>::iterator It = UniqueSources.begin();
    GraphNode *Src = *It;

    OVLSType SrcType = Src->type();
    if (SrcType.getElementSize() != N1.type().getElementSize())
      return false;

    if (++It != UniqueSources.end() && SrcType != (*It)->type())
      return false;

    return true;
  }

  // Iterate through the WorkList once and merge two nodes if they meet
  // the criteria of canBeMerged() and returns true. Returns false if no
  // merging happen. If there are too many choices for a node to be merged
  // with, it picks the one with the lowest cost that comes first.
  // TODO: Current approach of merging two nodes is very limited. It
  // only estimates merging two nodes by inserting N2 at the end of N1.
  // At some point, we should support merging two nodes in various ways.
  bool mergeNodes(GraphNodeList &WorkList) {
    bool CanbeOptimized = false;

    assert(WorkList.size() <= 80 && "Cannot merge, too big of a WorkList!!!");

    for (GraphNodeList::iterator It1 = WorkList.begin(); It1 != WorkList.end();
         ++It1) {
      GraphNode *N1 = *It1;
      GraphNodeList::iterator ToBeMergedIT = WorkList.end();

      int MinCost = INT_MAX;

      for (GraphNodeList::iterator It2 = std::next(It1); It2 != WorkList.end();
           ++It2) {
        GraphNode *N2 = *It2;
        if (canBeMerged(*N1, *N2)) {
          int MergingCost = getMergeCost(*N1, *N2);

          // Compute the lowest cost.
          if (MergingCost < MinCost) {
            MinCost = MergingCost;
            ToBeMergedIT = It2;
          }
        }
      }
      if (ToBeMergedIT != WorkList.end()) {
        GraphNode *ToBeMerged = *ToBeMergedIT;
        merge(N1, ToBeMerged);
        WorkList.erase(ToBeMergedIT);
        delete ToBeMerged;

        // If size of N1 reached the vector length, there is nothing that can
        // be optimized further.
        // Allow another iteration of optimization only if the size is less
        // than vector length.
        if (N1->size() < VectorLength * BYTE)
          CanbeOptimized = true;
      }
    }

    return CanbeOptimized;
  }

  // Merge N2 to N1, concatenate the edges of N2 at the end of N1.
  // TODO: merge according to a mask.
  void merge(GraphNode *N1, GraphNode *N2) {
    // Update outgoing edges.
    uint32_t N1BitIndex = N1->size();

    for (auto &E : N2->incomingEdges()) {
      E->updateDest(N1);
      N1->addAnIncomingEdge(N1->size(), E);
    }

    for (auto &E : N2->outgoingEdges()) {
      E->updateSource(N1, N1BitIndex + E->getBitIndex());
      N1->addAnOutgoingEdge(E);
    }

    // delete N2
    N2->clearEdges();
    removeNode(N2);

    // update type.
    uint32_t ElementSize = N1->type().getElementSize();
    N1->setType(OVLSType(ElementSize, N1->size() / ElementSize));
  }

  /// Returns false if verification fails. Verification fails if
  ///  1. Total number of gathers/scatters in the \p Group does not match the
  ///     total number of gather/scatter-nodes in this(graph).
  ///  2. If the number of bytes in a gather-node does not match the number of
  ///     bytes of the correspondent gather in the group. We find this
  ///     correspondence assuming the order of the gathers in a group is the
  ///     same as in G).
  ///  3. This function should only be called right after the load/store-
  ///     generation before any other optimization of the graph. Otherwise,
  ///     it will produce incorrect result.
  /// FIXME: Support masked gathers/scatters, duplicates;
  bool verifyInitialGraph(const OVLSGroup &Group) {
    uint32_t TotalGSNodes = 0;

    bool GroupOfGathers = false;
    if (Group.hasGathers())
      GroupOfGathers = true;

    for (GraphNode *N : Nodes) {
      // This initial graph has only two kinds of nodes: nodes for loads/stores
      // and nodes for gather/scatter results. Therefore, a node that is not a
      // load nor a store is a gather or a scatter node.
      if (!N->isALoad() && !N->isAStore()) {
        // Found a gather/scatter-result-node
        OVLSMemref *GSNode = Group.getMemref(TotalGSNodes);
        if (N->type().getSize() != GSNode->getType().getSize())
          // Size of the gather/scatter-node does not match the size of the
          // actual gather/scatter memref.
          return false;

        // FIXME: currently assuming all elements are masked on elements.
        // Support masked gather/scatter.
        // One way to verify that the elements are loaded/stored for unmasked
        // gather/scatter is to verify that there are no elements without valid
        // source/destination.
        GraphNode *Node;
        for (GraphNode::iterator I = N->begin(), E = N->end(); I != E; ++I)
          if (GroupOfGathers) {
            // Check if gather-element comes from a load node.
            if (!(Node = (*I)->getSource()) || !Node->isALoad())
              return false;
          }
          // Check if this scatter element flows into a store node.
          else if (!(Node = (*I)->getDest()) || !Node->isAStore())
            return false;
        TotalGSNodes++;
      }
    }

    if (TotalGSNodes != Group.size())
      return false;

    return true;
  }

  // Returns false if a node in the graph has more than two source nodes.
  bool verifyGraph() {
    std::set<GraphNode *> UniqueSources;
    for (GraphNode *N : Nodes) {
      if (N->getNumUniqueSources(UniqueSources) > 2)
        return false;
      UniqueSources.clear();
    }
    return true;
  }

  // Visit the graph in a topological order and push the instruction into the
  // InstVector. While we are at it, generate instruction if the node does not
  // have one already.
  void getInstructions(OVLSInstructionVector &InstVector,
                       GraphNodeToOVLSMemrefMap &NodeToMemrefMap,
                       OVLSMemrefToInstMap *Map = nullptr) {
    GraphNodeList TopSortedNodes;
    getTopSortedNodes(TopSortedNodes);

    for (GraphNode *N : TopSortedNodes) {
      if (N->isUndefined())
        N->genShuffle();

      // Update the stored value. Initially when we created the store the
      // initial
      // value was undef since instruction for the def-node was not defined at
      // that
      // point.
      if (N->isAStore())
        N->updateStoredValue();

      OVLSInstruction *I = N->getInstruction();
      assert(I != nullptr && "Inst cannot be null!!!");
      InstVector.push_back(I);

      if (Map == nullptr)
        continue;
      GraphNodeToOVLSMemrefMap::iterator IT = NodeToMemrefMap.find(N);
      if (IT != NodeToMemrefMap.end())
        Map->insert(std::pair<OVLSMemref *, OVLSInstruction *>(IT->second, I));
    }
  }

  // This is the main wrapper function. It calls multiple methods to simplify
  // the graph so that the graph contains nodes with maximum of two sources. It
  // also optimizes (reduces the total number of nodes) the graph by merging
  // multiple nodes together.
  void simplifyAndOptimize() {
    if (!simplify()) {
      // Simplification did not happen which means the graph contains only
      // the load-nodes and the gather-nodes. There are no extra nodes that
      // can be optimized futher, so return.
      return;
    }

    // TODO: support a verifier that will ensure that each node has incoming
    // edges coming from maximum of 2 nodes.

    // We simplified the graph. Let's optimize it.
    OptVLS::GraphNodeList WorkList;
    for (GraphNode *N : Nodes)
      // TODO: Support store
      if (!N->isALoad() && !N->hasNoOutgoingEdges())
        WorkList.push_back(N);

    bool CanbeOptimized = true;
    // Keep merging until there are no changes in the graph.
    while (CanbeOptimized)
      CanbeOptimized = mergeNodes(WorkList);

    // TODO: Assess the optimized sequence
  }

}; // end of class Graph

#ifndef NDEBUG
static void dumpOVLSGroupVector(OVLSostream &OS, const OVLSGroupVector &Grps) {
  OS << "\n  Printing Groups- Total Groups " << Grps.size() << "\n";
  unsigned GroupId = 1;
  for (unsigned i = 0, S = Grps.size(); i < S; i++) {
    OS << "  Group#" << GroupId++ << " ";
    Grps[i]->print(OS, 3);
  }
}
static void dumpOVLSMemrefVector(OVLSostream &OS,
                                 const OVLSMemrefVector &MemrefVec,
                                 unsigned NumSpaces) {
  for (OVLSMemref *Memref : MemrefVec) {
    Memref->print(OS, NumSpaces);
    OS << "\n";
  }
}
static void
dumpMemrefDistanceMapVector(OVLSostream &OS,
                            const MemrefDistanceMapVector &AdjMemrefSetVec) {
  unsigned Id = 0;
  for (MemrefDistanceMap *AdjMemrefSet : AdjMemrefSetVec) {
    OS << "  Set #" << ++Id << "\n";
    // print each adjacent memref-set
    for (const auto &AMapElem : *AdjMemrefSet) {
      (AMapElem.second)->print(OS, 2);
      OS << "   Dist: " << AMapElem.first << "\n";
    }
  }
}
#endif

static unsigned genMask(uint64_t Mask, uint32_t shiftcount,
                        uint32_t bitlocation) {
  assert(bitlocation + shiftcount <= MAX_VECTOR_LENGTH &&
         "Invalid bit location for a bytemask");

  uint64_t NewMask = (shiftcount == 64) ? ~0LL : (1LL << shiftcount) - 1;
  Mask |= NewMask << bitlocation;

  return Mask;
}

static void printMask(OVLSostream &OS, uint64_t Mask) {
  // Convert int AccessMask to binary
  char SRMask[MAX_VECTOR_LENGTH + 1], *MaskPtr;
  SRMask[0] = '\0';
  MaskPtr = &SRMask[1];
  while (Mask) {
    if (Mask & 1)
      *MaskPtr++ = '1';
    else
      *MaskPtr++ = '0';

    Mask >>= 1;
  }
  // print the mask reverse
  while (*(--MaskPtr) != '\0') {
    OS << *MaskPtr;
  }
}

// Form OptVLSgroups for each set of adjacent memrefs in the MemrefSetVec
// where memrefs in the OptVLSgroup being together do not violate any program
// semantics nor any memory dependencies. Also, make sure the total size of
// the memrefs does not exceed the group size.
static void formGroups(const MemrefDistanceMapVector &AdjMrfSetVec,
                       OVLSGroupVector &OVLSGrps, unsigned VectorLength,
                       OVLSMemrefToGroupMap *MemrefToGroupMap) {
  for (MemrefDistanceMap *AdjMemrefSet : AdjMrfSetVec) {
    assert(!AdjMemrefSet->empty() && "Adjacent memref-set cannot be empty");
    MemrefDistanceMapIt AdjMemrefSetIt = (*AdjMemrefSet).begin();

    OVLSAccessType AccType = (AdjMemrefSetIt->second)->getAccessType();
    OVLSGroup *CurrGrp = new OVLSGroup(VectorLength, AccType);
    int GrpFirstMDist = AdjMemrefSetIt->first;

    // Group memrefs in each set using a greedy approach, keep inserting the
    // memrefs into the same group until the group is full. Form a new group
    // for a memref that cannot be moved into the group.
    for (MemrefDistanceMapIt E = (*AdjMemrefSet).end(); AdjMemrefSetIt != E;
         ++AdjMemrefSetIt) {
      OVLSMemref *Memref = AdjMemrefSetIt->second;
      unsigned ElemSize = Memref->getType().getElementSize() / BYTE; // in bytes

      int Dist = AdjMemrefSetIt->first;

      uint64_t AccMask = CurrGrp->getNByteAccessMask();

      if (!CurrGrp->empty() &&
          (( // capacity exceeded.
               Dist - GrpFirstMDist + ElemSize) > VectorLength ||
           !Memref->canMoveTo(*CurrGrp->getFirstMemref()))) {
        OVLSGrps.push_back(CurrGrp);
        CurrGrp = new OVLSGroup(VectorLength, AccType);

        // Reset GrpFirstMDist
        GrpFirstMDist = Dist;
        // Generate AccessMask
        AccMask = OptVLS::genMask(0, ElemSize, Dist - GrpFirstMDist);
      } else
        AccMask = OptVLS::genMask(AccMask, ElemSize, Dist - GrpFirstMDist);

      CurrGrp->insert(Memref, AccMask);
      if (MemrefToGroupMap)
        (*MemrefToGroupMap)
            .insert(std::pair<OVLSMemref *, OVLSGroup *>(Memref, CurrGrp));
    }

    OVLSGrps.push_back(CurrGrp);
  }

  // dump OVLSGroups
  OVLSDebug(OptVLS::dumpOVLSGroupVector(OVLSdbgs(), OVLSGrps));
  return;
}

// Split the memref-vector into groups where memrefs in each group
// are neighbors(adjacent), means they
//   1) have the same access type
//   2) have same number of vector elements
//   3) are a constant distance apart
//
// Adjacent memrefs in the group are sorted based on their distance from the
// first memref in the group in an ascending order.
static void splitMrfs(const OVLSMemrefVector &Memrefs,
                      MemrefDistanceMapVector &AdjMemrefSetVec) {
  OVLSDebug(OVLSdbgs() << "\n  Split the vector memrefs into sub groups of "
                          "adjacacent memrefs: \n");
  OVLSDebug(OVLSdbgs() << "    Distance is (in bytes) from the first memref of"
                          " the set\n");

  for (unsigned i = 0, Size = Memrefs.size(); i < Size; ++i) {
    OVLSMemref *Memref = Memrefs[i];

    int64_t Dist = 0;
    bool AdjMrfSetFound = false;

    // TODO: Currently finding the appropriate group is done using a linear
    // search. It would be better to use a hashing algorithm for this search.
    for (MemrefDistanceMap *AdjMemrefSet : AdjMemrefSetVec) {

      OVLSMemref *SetFirstSeenMrf = (*AdjMemrefSet).find(0)->second;

      if ( // same access type
          Memref->getAccessType() == SetFirstSeenMrf->getAccessType() &&
          // same number of vector elements
          Memref->haveSameNumElements(*SetFirstSeenMrf) &&
          // are a const distance apart
          Memref->isAConstDistanceFrom(*SetFirstSeenMrf, &Dist)) {
        // Found a set
        AdjMrfSetFound = true;
        (*AdjMemrefSet).insert(std::pair<int, OVLSMemref *>(Dist, Memref));
        break;
      }
    }
    if (!AdjMrfSetFound) {
      // No adjacent memref set exits for this memref, create a new set.
      MemrefDistanceMap *AdjMemrefSet = new MemrefDistanceMap();
      // Assign 0 as a distance to the first memref in the set.
      (*AdjMemrefSet).insert(std::pair<int, OVLSMemref *>(0, Memref));
      AdjMemrefSetVec.push_back(AdjMemrefSet);
    }
  }

  // dump sorted set
  OVLSDebug(OptVLS::dumpMemrefDistanceMapVector(OVLSdbgs(), AdjMemrefSetVec));
  return;
}

// Returns true if this acceess-mask has contiguous accesses means no 0s
// between 1s
static bool hasContiguousAccesses(uint64_t ByteAccessMask,
                                  uint64_t TotalBytes) {
  // All bytes are set to 1
  if ((((uint64_t)1 << TotalBytes) - 1) == ByteAccessMask)
    return true;

  // Look for the first zero in the mask.
  while ((ByteAccessMask & 0x1) == 1)
    ByteAccessMask = ByteAccessMask >> 1;

  // mask value (starting from the first zero in the mask)is zero,
  // that means there are no more accesses after a gap.
  if (ByteAccessMask == 0)
    return true;

  return false;
}

static bool isSupported(const OVLSGroup &Group) {
  int64_t Stride = 0;
  if (Group.size() < 2) {
    OVLSDebug(
        OVLSdbgs() << "Minimum Two neighbors required!!!\n");
    return false;
  }

  if (!Group.hasAConstStride(Stride)) {
    OVLSDebug(
        OVLSdbgs() << "Optimized sequence is only supported for a group"
                      " of gathers/scatters that has a constant stride!!!\n");
    return false;
  }

  // Currently, AOS with heterogeneous members or adjacent memrefs with gaps
  // in between the accesses are not supported.
  if (!OptVLS::hasContiguousAccesses(Group.getNByteAccessMask(),
                                     Group.getVectorLength())) {
    OVLSDebug(OVLSdbgs() << "Cost analysis is only supported for a group of"
                            " contiguous ith accesses!!!\n");
    return false;
  }

  // Compute the number of bytes in the i-th elements of the memrefs
  uint64_t NBMask = Group.getNByteAccessMask();
  int32_t UsedBytes = 0;
  while (NBMask != 0) {
    NBMask = NBMask >> 1;
    UsedBytes++;
  }

  // Group with overlapping accesses is not supported
  if ((Stride + 1) < UsedBytes)
    return false;

  return true;
}

/// Generate an OVLSShuffle for Mrf where the shuffle-mask starts from the
/// Index. Here shuffle instruction demonstrates the memory access pattern.
/// E.g.
///    Mrf = <2 x 64> SSload   Dist: 0 Stride 16
///    StartIndex = 1
///
///    The Equivalent shuffle would be:
///    Shuffle = shufflevector <2 x 64>* &Mrf, <2 x 64> %undef, <2 x 32><1, 3>
OVLSInstruction *genShuffleForMemref(const OVLSMemref &Mrf, int64_t Index) {
  OVLSType Ty = Mrf.getType();

  OVLSOperand *Op1 = new OVLSAddress(&Mrf, 0);
  OVLSOperand *Op2 = new OVLSUndef(Ty);

  OVLSConstant *ShuffleMask = nullptr;

  uint32_t ElemSize = Ty.getElementSize();
  uint32_t ElemSizeInByte = ElemSize / BYTE;
  uint32_t NumElems = Ty.getNumElements();

  const uint32_t MaxNumElems = 256;
  assert(NumElems <= MaxNumElems && "Increase MaxNumElems");

  int64_t Stride = 0;
  assert(Mrf.hasAConstStride(&Stride) && "Constant stride expected!!!");
  Stride = Stride / ElemSizeInByte;

  int32_t IntShuffleMask[MaxNumElems];
  for (uint32_t MaskIndex = 0; MaskIndex < NumElems; MaskIndex++) {
    IntShuffleMask[MaskIndex] = Index;
    Index += Stride;
  }

  ShuffleMask =
      new OVLSConstant(OVLSType(32, NumElems), (int8_t *)IntShuffleMask);

  return new OVLSShuffle(Op1, Op2, ShuffleMask);
}

/// This function returns a vector of contiguous loads/stores for a group of
/// gathers/scatters that has a constant stride. It considers elements in each
/// ith accesses coming in a stream and tries to fit as many elements
/// as possible into each single load/store. Each load/store size is determined
/// by the vector-length.
/// It also returns a mapping of the load/store-elements to the
/// gather/scatter-elements using a directed graph. Each node in the graph
/// represents either a load or a gather and each edge shows which loaded
/// elements contribute to which gather results.
/// FIXME: Support masked gathers/scatters.
  static void getLoadsOrStores(const OVLSGroup &Group, Graph &G,
                               GraphNodeToOVLSMemrefMap &NodeToMemrefMap) {
  int64_t Stride = 0;
  if (!Group.hasAConstStride(Stride))
    assert("Group with a variable stride is not supported!!!");

  // If it's not a group of gathers that means it's a group of scatters.
  bool GroupOfGathers = false;
  if (Group.hasGathers())
    GroupOfGathers = true;

  // Assuming the highest element size is 64.
  uint32_t LowestElemSize = 64; // in bits
  // Step1: Create a graph-node for each gather/scatter-result. We need to get
  // them ready to be connected with the load/store-nodes during each load/store
  // generation at the later phase. While we are at it, compute the lowest
  // element size of the group. This lowest element size (which is the common
  // divisor of all the other sizes of the memrefs in the group) will determine
  // the load-size and load mask.
  // Scatter-results are the source-nodes of a group of scatters which should
  // not be undefined. Therefore, we generate dummy shuffles to mimic the
  // behavior of scatter-node(register).
  int64_t StartIndex = 0;
  OVLSMemref *Prev = nullptr;
  for (OVLSGroup::const_iterator I = Group.begin(), E = Group.end(); I != E;
       ++I) {
    OVLSMemref *Curr = *I;
    int64_t Dist = 0;
    // Don't create nodes for the duplicates. We will replace the duplicates
    // with
    // the final shuffle instruction at the end.
    if (Prev && Curr->isAConstDistanceFrom(*Prev, &Dist) && Dist == 0)
      continue;

    // At this point, we don't know the desired instruction/opcode,
    // initialize it(associated OVLSInstruction) to nullptr.
    OVLSMemref *Mrf = *I;
    OVLSType MemrefType = Mrf->getType();

    GraphNode *GSNode;
    if (GroupOfGathers)
      GSNode = new GraphNode(nullptr, MemrefType);
    else {
      // TODO: Currenlty, assumes there are no gaps between the memrefs in the
      // Group, so incrementing StartIndex by 1. Support gaps between the
      // memrefs.
      OVLSInstruction *ScatterdRes = genShuffleForMemref(*Mrf, StartIndex++);
      GSNode = new GraphNode(ScatterdRes, MemrefType);
    }
    G.insert(GSNode);
    NodeToMemrefMap.insert(std::pair<GraphNode *, OVLSMemref *>(GSNode, *I));

    uint32_t ElemSize = MemrefType.getElementSize(); // in bits
    assert(ElemSize <= 64 && "Unexpected element size!!!");
    if (ElemSize < LowestElemSize)
      LowestElemSize = ElemSize;
    Prev = *I;
  }

  // Access mask of the group in bytes.
  uint64_t AccessMask = Group.getNByteAccessMask();
  uint64_t AMask = AccessMask;

  // The number of elements in each gather.
  uint32_t NumElems = Group.getNumElems();

  // Load mask
  uint64_t ElementMask = 0;
  // Offset of the load address.
  uint32_t Offset = 0;
  // Holds the total number of elements in a load. It could be different
  // between the generated loads.
  uint32_t NumElemsInALoad = 0;
  // Same as NumElemsInALoad. LoadType could be different between multiple
  // generated loads.
  OVLSType LSType;

  OVLSMemref *GrpFirstMemref = Group.getFirstMemref();

  uint32_t VecLen = Group.getVectorLength();
  // Bit location of the load mask, gets computed though elements iterations
  // of multiple gathers.
  uint32_t BitLocation = 0;
  // Holds each load size in bytes, is used in deciding the maximum size of
  // an load instruction which should not exceed the vector length.
  uint32_t LSSize = 0; // in byte
  // Holds the size of missing gathers, random gaps between the elements due
  // to padding or the distance between ith accesses.
  uint32_t GapSize = 0; // in byte
  // Holds the distance between the last element of ith access and the first
  // element of the ith+1 access. Distance gets computed at the end of ist
  // access of the group.
  uint32_t Distance;

  // Step2: Generate loads/stores for loading/storing the contiguous bytes
  // created by a group of gathers/scatters. The main idea is to consider
  // a group of gathers/scatters as an stream of elements such as:
  // (g/s1, g/s2, g/s3, ....g/s1, g/s2, g/s3). So, our goal to consider loading
  // /storing all the elements in the group (total number of
  // gathers/scatters x n elements). While we are it, consider any gaps in
  // between
  // the elements.

  // Generate the first load/store.
  OVLSOperand *SrcOrDst = new OVLSAddress(GrpFirstMemref, Offset);
  OVLSInstruction *CurrLSInst;

  if (GroupOfGathers)
    CurrLSInst = new OVLSLoad(LSType, *SrcOrDst, ElementMask);
  else {
    OVLSOperand *Undef = new OVLSUndef(LSType);
    CurrLSInst = new OVLSStore(Undef, *SrcOrDst, ElementMask);
  }
  // Generate the graph-node for the load/store.
  GraphNode *CurrLSNode = new GraphNode(CurrLSInst, CurrLSInst->getType());
  G.insert(CurrLSNode);

  uint32_t IthElem = 0;
  // Traverse total (NumElems * number of gathers) times.
  while (IthElem < NumElems) {
    GraphNodeList::iterator It = G.begin();
    // Load/store ith-elem of each gather/scatter
    for (unsigned i = 0; i < Group.size(); i++) {
      // Addressed an element, find out its type.
      GraphNode *GSNode = *It++;
      OVLSType GSType = GSNode->type();
      uint32_t ElemSizeInBit = GSType.getElementSize();
      uint32_t ShiftCount = ElemSizeInBit / LowestElemSize;
      uint32_t ElemSize = ElemSizeInBit / BYTE; // in bytes

      // If this element cannot be loaded using the current load, create a new
      // load.
      if (LSSize + ElemSize + GapSize > VecLen || GapSize % ElemSize != 0) {
        SrcOrDst = new OVLSAddress(GrpFirstMemref, Offset);
        if (GroupOfGathers)
          // create a new load.
          CurrLSInst = new OVLSLoad(LSType, *SrcOrDst, ElementMask);
        else {
          // create a new store
          OVLSOperand *Undef = new OVLSUndef(LSType);
          CurrLSInst = new OVLSStore(Undef, *SrcOrDst, ElementMask);
        }
        CurrLSNode = new GraphNode(CurrLSInst, CurrLSInst->getType());
        G.insert(CurrLSNode);

        // Reset all the intializers.
        ElementMask = 0;
        LSSize = 0;
        NumElemsInALoad = 0;
        BitLocation = 0;
        GapSize = 0;
      }

      // If there is a gap preceding this element, update element mask, type etc
      // for the gap.
      if (GapSize != 0) {
        uint32_t GapShiftCount = GapSize / LowestElemSize;
        BitLocation += GapShiftCount;
        LSSize += GapSize;
        ShiftCount += GapShiftCount;

        // Resent gap size
        GapSize = 0;
      }

      // Update element mask and load/store type for this element.
      NumElemsInALoad += ShiftCount;
      // Create element mask using lowest element size for this element..
      ElementMask = OptVLS::genMask(ElementMask, ShiftCount, BitLocation);
      CurrLSInst->setMask(ElementMask);
      LSType = OVLSType(LowestElemSize, NumElemsInALoad);
      CurrLSInst->setType(LSType);
      CurrLSNode->setType(LSType);

      // Draw an edge from the loaded element to the gather node or
      // from the scatter node to the stored element
      if (GroupOfGathers) {
        BitRange LoadBR = BitRange(LSSize * BYTE, ElemSizeInBit);
        Edge *E = new Edge(CurrLSNode, GSNode, LoadBR, IthElem * ElemSizeInBit);
        GSNode->addAnIncomingEdge(IthElem * ElemSizeInBit, E);
        CurrLSNode->addAnOutgoingEdge(E);
      } else {
        BitRange StoreBR = BitRange(IthElem * ElemSizeInBit, ElemSizeInBit);
        Edge *E = new Edge(GSNode, CurrLSNode, StoreBR, LSSize * BYTE);
        CurrLSNode->addAnIncomingEdge(LSSize * BYTE, E);
        GSNode->addAnOutgoingEdge(E);
      }
      // Update
      LSSize += ElemSize;
      Offset += ElemSize;
      BitLocation += ShiftCount;

      // Update AccessMask
      uint32_t AMaskCounter = ElemSize;
      while (AMaskCounter-- != 0)
        AMask = AMask >> 1;

      // If we have hit the end of the AccessMask, reset it.
      if (AMask == 0) {
        AMask = AccessMask;
        // Compute any gap size between the accesses which will be used
        // during the load of the 1st element of next ith accesses.
        if (IthElem == 0)
          Distance = abs(Stride) - LSSize;

        GapSize = Distance;
      } else {
        // Account any physical gap existing between the memrefs.
        while ((AMask & 0x1) == 0) {
          AMask = AMask >> 1;
          GapSize++;
        }
      }
    }
    IthElem++;
  }
} // end of getLoadsOrStores
} // end of OptVLS namespace

void OVLSGroup::print(OVLSostream &OS, unsigned NumSpaces) const {

  OS << "\n    Vector Length(in bytes): " << getVectorLength();
  // print accessType
  OS << "\n    AccType: ";
  getAccessType().print(OS);

  // Print result mask
  OS << "\n    AccessMask(per byte, R to L): ";
  uint64_t AMask = getNByteAccessMask();
  OptVLS::printMask(OS, AMask);
  OS << "\n";

  // Print vector of memrefs that belong to this group.
  for (unsigned i = 0, S = MemrefVec.size(); i < S; i++) {
    MemrefVec[i]->print(OS, NumSpaces);
    OS << "\n";
  }
}

/// print the load instruction like this:
///   %1 = mask.load.32.3 (<Base:0x165eca0 Offset:0>, 111)
///   %2 = mask.load.32.3 (<Base:0x165eca0 Offset:12>, 111)
void OVLSLoad::print(OVLSostream &OS, unsigned NumSpaces) const {
  while (NumSpaces-- != 0)
    OS << " ";

  OS << "%" << getId();
  OS << " = ";
  OS << "mask.load." << getType().getElementSize() << ".";
  OS << getType().getNumElements();
  OS << " (";
  Src.print(OS);
  OS << ", ";
  OptVLS::printMask(OS, getMask());
  OS << ")";
}

/// print the store instruction like this:
///   mask.store.32.4 (<32x4>%1, <Base:0x165eca0 Offset:0>, 1101)
void OVLSStore::print(OVLSostream &OS, unsigned NumSpaces) const {
  while (NumSpaces-- != 0)
    OS << " ";

  OS << "call void @mask.store." << getType().getElementSize() << ".";
  OS << getType().getNumElements();
  OS << " (";
  Value->printAsOperand(OS);
  OS << ", ";
  Dst.print(OS);
  OS << ", ";
  OptVLS::printMask(OS, getMask());
  OS << ")";
}

/// print the shuffle instruction like below:
/// %3 = shufflevector <2 x 64> %1, <2 x 64> %2, <2 x 32><0, 2>
void OVLSShuffle::print(OVLSostream &OS, unsigned NumSpaces) const {
  while (NumSpaces-- != 0)
    OS << " ";

  OS << "%" << getId();
  OS << " = ";
  OS << "shufflevector ";

  Op1->printAsOperand(OS);
  OS << ", ";

  Op2->printAsOperand(OS);
  OS << ", ";

  OS << *Op3;
}

bool OVLSShuffle::hasValidOperands(OVLSOperand *Op1, OVLSOperand *Op2,
                                   OVLSOperand *Mask) const {
  assert(Op1 != nullptr && "A minimum of one defined input vector required!!!");
  if (!Op1->getType().isValid())
    return false;

  // O1 and O2 must be vectors of the same type.
  if (Op2 != nullptr && Op1->getType() != Op2->getType())
    return false;

  // Mask needs to be a vector of constants.
  if (!Mask || !Mask->getType().isValid() || !isa<OVLSConstant>(Mask))
    return false;

  uint32_t MaxNumElems = Op1->getType().getNumElements();

  MaxNumElems *= 2;

  if (Mask->getType().getNumElements() > MaxNumElems)
    return false;

  return true;
}

// getGroups() takes a vector of OVLSMemrefs and a group size in bytes
// (which is the the maximum length of the underlying vector register
// or any other desired size that clients want to consider, maximum size
// can be 64), and returns a vector of OVLSGroups. It also optionally returns
// a map where each memref is mapped to the group that it belongs to. Each
// group contains one or more OVLSMemrefs, (and each OVLSMemref is contained by
// 1 (and only 1) OVLSGroup) in a way where having all the memrefs in
// OptVLSgroup (at one single point in the program, the location of first
// memref in the group)does not violate any program semantics nor any memory
// dependencies.
void OptVLSInterface::getGroups(const OVLSMemrefVector &Memrefs,
                                OVLSGroupVector &Grps, unsigned VectorLength,
                                OVLSMemrefToGroupMap *MemrefToGroupMap) {
  OVLSDebug(OVLSdbgs() << "Received a request from Client---FORM GROUPS\n");

  if (Memrefs.empty())
    return;

  if (VectorLength > MAX_VECTOR_LENGTH) {
    OVLSDebug(OVLSdbgs() << "!!!Group size above " << MAX_VECTOR_LENGTH
                         << " bytes is not supported currently\n");
    return;
  }

  OVLSDebug(OVLSdbgs() << "  Recieved a vector of memrefs: \n");
  OVLSDebug(OptVLS::dumpOVLSMemrefVector(OVLSdbgs(), Memrefs, 2));

  // Split the vector of memrefs into sub groups where memrefs in each sub group
  // are neighbors, means they
  //   1) have the same access type
  //   2) are a constant distance apart
  MemrefDistanceMapVector AdjMemrefSetVec;
  OptVLS::splitMrfs(Memrefs, AdjMemrefSetVec);

  // Form OptVLSgroups for each sub group where having all the memrefs in
  // OptVLSgroup (at one single point in the program, the location of first
  // memref in the group)does not violate any program semantics nor any memory
  // dependencies.
  // Also, make sure the total size of the memrefs does not exceed the group
  // size.
  OptVLS::formGroups(AdjMemrefSetVec, Grps, VectorLength, MemrefToGroupMap);

  // Release memory
  for (MemrefDistanceMap *AdjMemrefSet : AdjMemrefSetVec)
    delete AdjMemrefSet;

  return;
}

// \brief getSequence() takes a group of gathers/scatters (collectively
// represents both strided and indexed loads/stores). Returns true
// if it is able to generate a vector of instructions (basically a set of
// contiguous loads/stores followed by shuffles) that can replace (which is
// semantically equivalent of) the gathers/scatters.
// Returns false if it is unable to generate the sequence. This function
// tries to generate the best optimized sequence without doing any
// relative cost/benefit analysis (which is gather/scatter vs. the generated
// sequence). The main purpose of this function is to help diagnostics.
//
// Current implementation status:
// Current implementation only supports a group of adjacent strided-loads/stores
// with no gaps in between the accesses. E.g. a series of strided loads like
// below will not be supported since access to a[3i+2] is missing in this set.
// loop:
//      t1 = a[3i];
//      t2 = a[3i+1];
//      t3 = a[3i+3];
//
// Therefore, it will also not supoport array of structs where members have
// different data types.
// E.g.
// struct S{ float f, double d}s[100];
// loop:
//   t1 = s[i].f;
//   t2 = s[i].d;
//
// Strided-loads/stores with constant stride (a uniform distance between the
// vector elements) is supported.
bool OptVLSInterface::getSequence(const OVLSGroup &Group,
                                  const OVLSCostModel &CM,
                                  OVLSInstructionVector &InstVector,
                                  OVLSMemrefToInstMap *MemrefToInstMap) {
  if (!OptVLS::isSupported(Group))
    return false;

  OptVLS::Graph G(Group.getVectorLength(), CM);

  OptVLS::GraphNodeToOVLSMemrefMap NodeToMemrefMap;
  OptVLS::getLoadsOrStores(Group, G, NodeToMemrefMap);

  /// At this point, we have generated loads (to load the contiguous chunks of
  /// memory created by the \p Group of gathers) and a graph that shows which
  /// loaded elements contribute to which gather results (gather-node) by
  /// drawing edges from load-nodes(nodes with load instruction) to
  /// gather-result-nodes(nodes with null instruction).
  /// To ensure that we are on the right track of producing the gather results,
  /// make sure the graph contains a node for each gather in the group with
  /// number of bytes (matches the number of bytes of the gather in the group)
  /// coming from no other nodes than load nodes.
  if (!G.verifyInitialGraph(Group))
    return false;

  G.simplifyAndOptimize();

  if (!G.verifyGraph())
    return false;

  G.getInstructions(InstVector, NodeToMemrefMap, MemrefToInstMap);

  return true;
}

// The members of Group can be either vectorized individually using a
// gather/scatter each, or can be vectorized together with their neighbors
// in the Group using wide loads/stores and shuffles. This function
// returns the minimum cost between these two options (i.e. the absolute
// cost of the best way to vectorize this Group).
int64_t OptVLSInterface::getGroupCost(const OVLSGroup &Group,
                                      const OVLSCostModel &CM) {
  int64_t Cost = 0;

  // 1. Obtain the cost of vectorizing this group using wide loads/stores
  // + shuffle-sequence
  OVLSInstructionVector InstVector;
  if (getSequence(Group, CM, InstVector)) {
    for (OVLSInstruction *I : InstVector) {
      OVLSdbgs() << *I;
      int64_t C = CM.getInstructionCost(I);
      // OVLSdbgs() << "Cost = " << C << "\n";
      Cost += C;
    }
  }

  // 2. Obtain the cost of vectorizing this group using gathers/scatters.
  // FORNOW: If gathers/scatters are not supported the cost is 0.
  // TODO: We want to return the cost of the scalarized gathers/scatters
  // if they are not supported, instead of zero.
  int64_t GatherScatterCost = 0;
  const OVLSMemrefVector &MemrefVec = Group.getMemrefVec();
  for (OVLSMemref *Memref : MemrefVec) {
    int64_t C = CM.getGatherScatterOpCost(*Memref);
    // OVLSdbgs() << "Cost = " << C << "\n";
    GatherScatterCost += C;
  }
  // OVLSdbgs() << "Shuffle Cost = " << Cost << "\n";
  // OVLSdbgs() << "Gather/Scatter Cost = " << GatherScatterCost << "\n";

  // 3. Return the minimum of the two costs.
  if (GatherScatterCost && GatherScatterCost <= Cost)
    Cost = GatherScatterCost;

  // Both gathers/scatters and shuffles not supported; return some high dummy
  // cost. TODO: Instead, return the scalarization cost.
  // Once the cost utilities are fully implemented we don't expect to get a
  // zero cost.
  if (Cost == 0)
    Cost =
        MemrefVec.size() * Group.getFirstMemref()->getType().getNumElements();

  return Cost;
}
