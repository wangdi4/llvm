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
constexpr uint32_t OVLSShuffle::UndefMask;

// A set of adjacent memrefs and their distances from the first memref. The
// choice of the vector is intentional, so that the original order of memrefs is
// preserved.
using MemrefDistanceMap = std::vector<std::pair<OVLSMemref *, int64_t>>;

// MemrefDistanceMapVector is a vector of groups where each group contains a set
// of adjacent memrefs.
typedef OVLSVector<MemrefDistanceMap *> MemrefDistanceMapVector;
typedef MemrefDistanceMapVector::iterator MemrefDistanceMapVectorIt;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void OVLSAccessKind::print(OVLSostream &OS) const {
  switch (AccessKind) {
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

void OVLSAccessKind::dump() const {
  print(OVLSdbgs());
  OVLSdbgs() << '\n';
}
#endif

OVLSMemref::OVLSMemref(OVLSMemrefKind K, OVLSType T, OVLSAccessKind AKind)
    : Kind(K), AccessKind(AKind) {
  DType = T;
  static unsigned MemrefId = 1;
  Id = MemrefId++;

  if (AccessKind == OVLSAccessKind::Unknown) {
    OVLSDebug(OVLSdbgs() << "#" << Id
                         << " .Created an OVLSMemref of Unknown AccesType.");
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
  getAccessKind().print(OS);
}

void OVLSMemref::dump() const {
  print(OVLSdbgs(), 0);
  OVLSdbgs() << '\n';
}
#endif

uint64_t OVLSCostModel::getShuffleCost(SmallVectorImpl<uint32_t> &Mask,
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
    return TTI.getShuffleCost(TargetTransformInfo::SK_Select, Tp, 0,
                              nullptr);

  // TODO: Support SK_Insert
  uint32_t TotalElems = Mask.size();
  for (uint32_t MaskElem : Mask)
    if (MaskElem == OVLSShuffle::UndefMask)
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
    OVLSVector<GraphNode *> UniqueSources;
    assert(getNumUniqueSources(UniqueSources) == 1 && "Unexpected sources!!!");
    OVLSVector<GraphNode *>::iterator It = UniqueSources.begin();
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
  uint32_t getNumUniqueSources(OVLSVector<GraphNode *> &UniqueSources) const {
    std::set<GraphNode *> VisitedSources;
    for (Edge *E : IncomingEdges) {
      GraphNode *Src = E->getSource();

      if (VisitedSources.find(Src) != VisitedSources.end())
        continue;

      UniqueSources.push_back(Src);
      VisitedSources.insert(Src);
    }
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
    OVLSVector<GraphNode *> UniqueSources;
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
      assert(VisitedSources.find(Src) == VisitedSources.end() &&
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
        IntShuffleMask[MaskIndex++] = OVLSShuffle::UndefMask;
      else {
        if (isa<OVLSUndef>(Op1)) {
          Op1 = Src;
          Op2StartIndex = Op1->getType().getNumElements();
        } else if (Src != Op1 && isa<OVLSUndef>(Op2))
          Op2 = Src;
        else if (Src != Op1 && Src != Op2)
          llvm_unreachable("Invalid number of operands for OVLSShuffle!!!");

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
  OVLSVector<uint32_t> getPossibleIncomingMask(const GraphNode &N1,
                                               const GraphNode &N2) const {
    OVLSVector<uint32_t> Mask;
    OVLSVector<GraphNode *> UniqueSources;
    N1.getNumUniqueSources(UniqueSources);
    uint32_t NumUniqueSources = N2.getNumUniqueSources(UniqueSources);
    (void)NumUniqueSources;
    assert(NumUniqueSources <= 2 && "Invalid total sources!");

    OVLSVector<GraphNode *>::iterator It = UniqueSources.begin();
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
        Mask.push_back(OVLSShuffle::UndefMask);
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
        Mask.push_back(OVLSShuffle::UndefMask);
      else if (Src == Src1)
        Mask.push_back(BIndex / ElemSize + S1StartIndex);
      else
        Mask.push_back(BIndex / ElemSize + S2StartIndex);
    }

    // Mask size needs to match the source size.
    while (Mask.size() < S2StartIndex)
      Mask.push_back(OVLSShuffle::UndefMask);

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
      std::map<int, SmallVector<uint32_t, 16>> &NodeMaskMap) const {
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

      auto MapIt = NodeMaskMap.find(Id);
      SmallVector<uint32_t, 16> Mask;
      if (MapIt == NodeMaskMap.end())
        // Mask size needs to match the source size.
        Mask = SmallVector<uint32_t, 16>(NumElems, OVLSShuffle::UndefMask);
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
  SmallVector<SmallVector<uint32_t, 16>, 16>
  getPossibleOutgoingMergeMasks(const GraphNode &N1,
                                const GraphNode &N2) const {
    SmallVector<SmallVector<uint32_t, 16>, 16> Masks;
    std::map<int, SmallVector<uint32_t, 16>> NodeMaskMap;

    // Compute masks that are created by the outgoing edges of N1
    getPossibleOutgoingMasks(N1, 0, NodeMaskMap);

    // Compute masks that are created by the outgoing edges of N2
    getPossibleOutgoingMasks(N2, N1.type().getSize(), NodeMaskMap);

    std::map<int, SmallVector<uint32_t, 16>>::iterator MapIt, MapItE;

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
    SmallVector<uint32_t, 16> Mask = getPossibleIncomingMask(N1, N2);
    VectorType *VecTy = VectorType::get(ElemType, Mask.size());
    uint64_t Cost = CM.getShuffleCost(Mask, VecTy);

    // Compute outward impact.
    SmallVector<SmallVector<uint32_t, 16>, 16> Masks =
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

    OVLSVector<GraphNode *> UniqueSources;
    N1.getNumUniqueSources(UniqueSources);
    uint32_t NumUniqueSources = N2.getNumUniqueSources(UniqueSources);

    if (NumUniqueSources > 2)
      return false;

    OVLSVector<GraphNode *>::iterator It = UniqueSources.begin();
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
    if (Group.getAccessKind().isLoad())
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
    OVLSVector<GraphNode *> UniqueSources;
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
      AMapElem.first->print(OS, 2);
      OS << "   Dist: " << AMapElem.second << "\n";
    }
  }
}
#endif

static uint64_t genMask(uint64_t Mask, uint32_t shiftcount,
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
    auto AdjMemrefSetIt = AdjMemrefSet->begin();

    OVLSAccessKind AccessKind = AdjMemrefSetIt->first->getAccessKind();
    OVLSGroup *CurrGrp =
        new OVLSGroup(AdjMemrefSetIt->first, VectorLength, AccessKind);
    int64_t GrpFirstMDist = 0;
    assert(AdjMemrefSetIt->second == 0 &&
           "Zero distance between first Memref and itself is expected");

    // Group memrefs in each set using a greedy approach, keep inserting the
    // memrefs into the same group until the group is full. Form a new group
    // for a memref that cannot be moved into the group.
    for (auto E = AdjMemrefSet->end(); AdjMemrefSetIt != E; ++AdjMemrefSetIt) {
      OVLSMemref *Memref = AdjMemrefSetIt->first;
      unsigned ElemSize = Memref->getType().getElementSize() / BYTE; // in bytes

      int64_t Dist = AdjMemrefSetIt->second;

      uint64_t AccMask = CurrGrp->getNByteAccessMask();

      // Adjust mask and distance if the new memref precedes all the previously
      // seen memrefs.
      if (Dist < GrpFirstMDist) {
        AccMask <<= GrpFirstMDist - Dist;
        GrpFirstMDist = Dist;
      }

      // FIXME: We assume that the first memory reference in AdjMemrefSet is the
      //        best InsertPoint to form a new group. Here we only check if it
      //        is legal to group Memref with InsertPoint, but do not try to
      //        find the really best InsertPoint yet. Though, we should use
      //        dominance information to find the optimal InsertPoint.

      // If it is not safe to insert the memref to the group, or if the group
      // capacity has been exceeded, then we create a new group.
      if (!CurrGrp->isSafeToInsert(*Memref) ||
          (Dist - GrpFirstMDist + ElemSize) > VectorLength) {
        // Sort memrefs in the group using their offsets.
        sort(*CurrGrp, [](OVLSMemref *LHS, OVLSMemref *RHS) {
          return *RHS->getConstDistanceFrom(*LHS) > 0;
        });
        OVLSGrps.push_back(CurrGrp);
        CurrGrp = new OVLSGroup(Memref, VectorLength, AccessKind);

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

    // Sort memrefs in the group using their offsets.
    sort(*CurrGrp, [](OVLSMemref *LHS, OVLSMemref *RHS) {
      return *RHS->getConstDistanceFrom(*LHS) > 0;
    });
    OVLSGrps.push_back(CurrGrp);
  }

  // dump OVLSGroups
  OVLSDebug(OptVLS::dumpOVLSGroupVector(OVLSdbgs(), OVLSGrps));
  return;
}

static void splitMrfsStep(OVLSMemref *Memref,
                          MemrefDistanceMapVector &AdjMemrefSetVec) {
  bool AdjMrfSetFound = false;

  // TODO: Currently finding the appropriate group is done using a linear
  // search. It would be better to use a hashing algorithm for this search.
  for (MemrefDistanceMap *AdjMemrefSet : AdjMemrefSetVec) {
    assert(!AdjMemrefSet->empty() && "Set cannot be empty");
    OVLSMemref *SetFirstMrf = AdjMemrefSet->begin()->first;

    if (Memref->getAccessKind() != SetFirstMrf->getAccessKind())
      continue;

    if (Memref->getNumElements() != SetFirstMrf->getNumElements())
      continue;

    // Currently we support grouping Memrefs with same elementsize.
    auto ElementSize = SetFirstMrf->getType().getElementSize();
    if (Memref->getType().getElementSize() != ElementSize)
      continue;

    // Dist is the distance to be added in Dist(SetFirstMrf) to get
    // Dist(Memref).
    // Dist = Dist(Memref) - Dist(SetFirstMrf)
    // Eg-Mrfs :int32 a[2*i+1] , a[2*i]
    //    FirstSeenMrf : a[2*i+1].
    //    Memref : a[2*i], Dist = -4.
    Optional<int64_t> Dist = Memref->getConstDistanceFrom(*SetFirstMrf);
    if (!Dist)
      continue;

    // Found a possible set. Check if the memref overlaps any of the memrefs
    // already in the set.
    auto OverlappedMemref =
        find_if(*AdjMemrefSet, [Dist, ElementSize](auto &x) {
          return std::abs(x.second - *Dist) < ElementSize / BYTE;
        });

    if (OverlappedMemref != AdjMemrefSet->end()) {
      OVLSDebug(OVLSdbgs() << "Cannot group a memref\n  " << *Memref
                           << "\ndue to overlapping with\n  "
                           << *OverlappedMemref->first << '\n');
      continue;
    }

    // FIXME: We assume that this function is invoked in the lexical order of
    //        memrefs for loads and in the reverse lexical order for stores. We
    //        preserve this order in AdjMemrefSet. We should switch from relying
    //        on such assumptions to proper ordering of incoming memrefs using
    //        dominance information.

    // Alright. Memref can be grouped.
    AdjMrfSetFound = true;
    AdjMemrefSet->emplace_back(Memref, *Dist);
    break;
  }

  if (!AdjMrfSetFound) {
    // No adjacent memref set exits for this memref, create a new set.
    MemrefDistanceMap *AdjMemrefSet = new MemrefDistanceMap();
    // Assign 0 as a distance to the first memref in the set.
    AdjMemrefSet->emplace_back(Memref, 0);
    AdjMemrefSetVec.push_back(AdjMemrefSet);
  }
}

// Split the memref-vector into groups where memrefs in each group
// are neighbors(adjacent), means they
//   1) have the same access kind
//   2) have same number of vector elements
//   3) are a constant distance apart
//
// The relative order of adjacent loads is preserved, relative order of stores
// is reversed (so that the first item in every set is a valid insertion point).
static void splitMrfs(const OVLSMemrefVector &Memrefs,
                      MemrefDistanceMapVector &AdjMemrefSetVec) {
  OVLSDebug(OVLSdbgs() << "\n  Split the vector memrefs into sub groups of "
                          "adjacacent memrefs: \n");
  OVLSDebug(OVLSdbgs() << "    Distance is (in bytes) from the first memref of"
                          " the set\n");

  // FIXME: Here we assume that refs inside \p Memref vector are in RPOT order.
  //        If this assumption holds true, for every grouppable set of memory
  //        references, the first reference to be processed by splitMrfsStep is
  //        either the lexically first load or the lexically last store.
  //        splitMrfsStep and formGroups rely on this and they are not able to
  //        form groups if the order of references is wrong. The algorithm
  //        should be changed in future to work even with shuffled \p Memref
  //        vector and use dominance information to sort it.

  // Process loads top down.
  for (OVLSMemref *Memref : Memrefs)
    if (Memref->getAccessKind().isLoad())
      splitMrfsStep(Memref, AdjMemrefSetVec);

  // Process stores bottom up.
  for (OVLSMemref *Memref : reverse(Memrefs))
    if (Memref->getAccessKind().isStore())
      splitMrfsStep(Memref, AdjMemrefSetVec);

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
  if (Group.size() < 2) {
    OVLSDebug(
        OVLSdbgs() << "Minimum Two neighbors required!!!\n");
    return false;
  }

  Optional<int64_t> Stride = Group.getConstStride();
  if (!Stride) {
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
  if (*Stride + 1 < UsedBytes)
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

  Optional<int64_t> Stride = Mrf.getConstStride();
  assert(Stride && "Constant stride is expected");

  int32_t IntShuffleMask[MaxNumElems];
  for (uint32_t MaskIndex = 0; MaskIndex < NumElems; MaskIndex++) {
    IntShuffleMask[MaskIndex] = Index;
    Index += *Stride / ElemSizeInByte;
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
  Optional<int64_t> Stride = Group.getConstStride();
  assert(Stride && "Group with a variable stride is not supported");

  // If it's not a group of gathers that means it's a group of scatters.
  bool GroupOfGathers = false;
  if (Group.getAccessKind().isLoad())
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
    // Don't create nodes for the duplicates. We will replace the duplicates
    // with
    // the final shuffle instruction at the end.
    if (Prev && Curr->getConstDistanceFrom(*Prev) == (int64_t)0)
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
          Distance = std::abs(*Stride) - LSSize;

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

bool OVLSGroup::isSafeToInsert(OVLSMemref &Mrf) const {
  if (!Mrf.canMoveTo(*InsertPoint))
    return false;

  if (Mrf.getAccessKind().isLoad())
    return InsertPoint->dominates(Mrf);
  else if (Mrf.getAccessKind().isStore())
    return InsertPoint->postDominates(Mrf);

  llvm_unreachable("Unknown AccessKind");
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void OVLSGroup::print(OVLSostream &OS, unsigned NumSpaces) const {

  OS << "\n    Vector Length(in bytes): " << getVectorLength();
  // print accessType
  OS << "\n    AccType: ";
  getAccessKind().print(OS);
  OS << ", Stride (in bytes): " << getConstStride();

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

void OVLSGroup::dump() const {
  print(OVLSdbgs(), 0);
  OVLSdbgs() << '\n';
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
  Src.print(OS, 0);
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
  Dst.print(OS, 0);
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
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

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

// getGroups() takes a vector of OVLSMemrefs and a group size in bytes (which is
// the the maximum length of the underlying vector register or any other desired
// size that clients want to consider, maximum size can be 64), and returns a
// vector of OVLSGroups. It also optionally returns a map where each memref is
// mapped to the group that it belongs to. Each group contains one or more
// OVLSMemrefs, (and each OVLSMemref is contained by 1 (and only 1) OVLSGroup)
// in a way where having all the memrefs in OptVLSgroup (at group InsertPoint
// location) does not violate any program semantics nor any memory dependencies.
void OptVLSInterface::getGroups(const OVLSMemrefVector &Memrefs,
                                OVLSGroupVector &Grps, unsigned VectorLength,
                                OVLSMemrefToGroupMap *MemrefToGroupMap) {
  OVLSDebug(OVLSdbgs() << "Received a request from Client---FORM GROUPS\n"
                       << "  Recieved a vector of memrefs (" << Memrefs.size()
                       << "): \n");
  OVLSDebug(OptVLS::dumpOVLSMemrefVector(OVLSdbgs(), Memrefs, 2));

  if (Memrefs.empty())
    return;

  if (VectorLength > MAX_VECTOR_LENGTH) {
    OVLSDebug(OVLSdbgs() << "!!!Group size above " << MAX_VECTOR_LENGTH
                         << " bytes is not supported currently\n");
    return;
  }

  // Split the vector of memrefs into sub groups where memrefs in each sub group
  // are neighbors, means they
  //   1) have the same access kind
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

  if (getSequencePredefined(Group, InstVector, MemrefToInstMap))
    return true;

  // Allowing execution of the graph algorithm only for this case.
  // Stride = 16 , represents factor =2.
  // Group.size = 2, has two neighbors.
  if (!(Group.getElemSize() == 64 && Group.size() == 2 &&
        Group.getAccessKind().isLoad() &&
        Group.getConstStride() == (int64_t)16))
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

// Function with hard coded sequences for the recognized interleaved pattern:
// %wide.vec = load <32 x i32>, <32 x i32>* %wide.vec.ptr, align 16
// %r1 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
//   <i32 0, i32 4, i32 8,  i32 12, i32 16, i32 20, i32 24, i32 28>
// %r2 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
//   <i32 1, i32 5, i32 9,  i32 13, i32 17, i32 21, i32 25, i32 29>
// %r3 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
//   <i32 2, i32 6, i32 10, i32 14, i32 18, i32 22, i32 26, i32 30>
// %r4 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
//   <i32 3, i32 7, i32 11, i32 15, i32 19, i32 23, i32 27, i32 31>
bool OptVLSInterface::genSeqLoadStride16Packed8xi32(
    const OVLSGroup &Group, OVLSInstructionVector &InstVector,
    OVLSMemrefToInstMap *MemrefToInstMap) {

  // Generate the instructions and push them into the InstVector, and make a
  // MemrefToInstMap.
  // I. Consecutive Loads:
  // %t0 = load from base <4xi32>
  // %t4 = base+4elem
  // %t8 = base+8elem
  // %t12 = base+12elem
  // %t2 = base+16elem
  // %t6 = base+20elem
  // %t10 = base+24elem
  // %t14 = base+28elem
  // Base gets provided by Client later when generating the LLVM IR.
  OVLSInstruction *LoadInst[8];
  OVLSInstruction *ShuffleOnLoad[4];
  const uint64_t ElementMask = 0x0F;
  OVLSType LType(32, 4); // <4xi32>
  int64_t Offset = 0;

  for (int i = 0; i < 8; i++) {
    OVLSMemref *FirstMemref = Group.getFirstMemref();
    OVLSAddress Src(FirstMemref, Offset);
    LoadInst[i] = new OVLSLoad(LType, Src, ElementMask);
    Offset = Offset + 16; // 4 elements offset.
    InstVector.push_back(LoadInst[i]);
  }
  // %t3 = shufflevector <4 x i32> %t0, <4 x i32> %t2, <8 x i32> <i32 0, i32 1,
  //       i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  // %t7 = shufflevector <4 x i32> %t4, <4 x i32> %t6, <8 x i32> <i32 0, i32 1,
  //       i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  // %t11=shufflevector <4 x i32> %t8, <4 x i32> %t10, <8 x i32> <i32 0, i32 1,
  //       i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  // %t15=shufflevector <4 x i32> %t12,<4 x i32> %t14, <8 x i32> <i32 0, i32 1,
  //       i32 2, i32 3, i32 4, i32 5,  i32 6, i32 7>
  const int32_t IntShuffleMask[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  OVLSConstant *ShuffleMask = new OVLSConstant(
      OVLSType(32, 8),
      reinterpret_cast<const int8_t *>(IntShuffleMask));
  for (int i = 0; i < 4; i++) {
    ShuffleOnLoad[i] =
        new OVLSShuffle(LoadInst[i], LoadInst[4 + i], ShuffleMask);
    InstVector.push_back(ShuffleOnLoad[i]);
  }
  // %t16 = shufflevector <8 x i32> %t3, <8 x i32> %t7, <8 x i32> <i32 0,
  //        i32 1, i32 9, i32 8, i32 4, i32 5,  i32 13, i32 12>
  // %t17 = shufflevector <8 x i32> %t15, <8 x i32> %t11, <8 x i32> <i32 1,
  //        i32 0,  i32 8, i32 9, i32 5, i32 4,  i32 12, i32 13>
  // %t18 = shufflevector <8 x i32> %t16, <8 x i32> %t17, <8 x i32> <i32 0,
  //        i32 3,  i32 10, i32 9, i32 4, i32 7,  i32 14, i32 13>
  // %t19 = %t18 // result0
  // %t20 = shufflevector <8 x i32> %t16, <8 x i32> %t17, <8 x i32> <i32 1,
  //        i32 2,  i32 11, i32 8, i32 5, i32 6,  i32 15, i32 12>
  // %t21 = %t20 // result1

  OVLSInstruction *ShuffleOnShuffles[8];
  OVLSInstruction *Results[4];

  const int32_t IntShuffleMask1[8] = {0, 1, 9, 8, 4, 5, 13, 12};
  const int32_t IntShuffleMask2[8] = {1, 0, 8, 9, 5, 4, 12, 13};
  const int32_t IntShuffleMask3[8] = {0, 3, 10, 9, 4, 7, 14, 13};
  const int32_t IntShuffleMask4[8] = {1, 2, 11, 8, 5, 6, 15, 12};

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask1));
  ShuffleOnShuffles[0] =
      new OVLSShuffle(ShuffleOnLoad[0], ShuffleOnLoad[1], ShuffleMask);

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask2));
  ShuffleOnShuffles[1] =
      new OVLSShuffle(ShuffleOnLoad[3], ShuffleOnLoad[2], ShuffleMask);

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask3));
  ShuffleOnShuffles[2] =
      new OVLSShuffle(ShuffleOnShuffles[0], ShuffleOnShuffles[1], ShuffleMask);

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask4));
  ShuffleOnShuffles[3] =
      new OVLSShuffle(ShuffleOnShuffles[0], ShuffleOnShuffles[1], ShuffleMask);

  Results[0] = ShuffleOnShuffles[2];
  Results[1] = ShuffleOnShuffles[3];

  // %t22=shufflevector <8 x i32> %t3, <8 x i32> %t7, <8 x i32> <i32 2, i32
  //    3,  i32 11, i32 10, i32 6, i32 7,  i32 15, i32 14>
  // %t23=shufflevector <8 x i32> %t15, <8 x i32> %t11, <8 x i32> <i32 3,
  //    i32 2,  i32 10, i32 11, i32 7, i32 6,  i32 14, i32 15>
  // %t24=shufflevector <8 x i32> %t22, <8 x i32> %t23, <8 x i32> <i32 0,
  //    i32 3,  i32 10, i32 9, i32 4, i32 7,  i32 14, i32 13>
  // %t25=%t24 // result2
  // %t26=shufflevector <8 x i32> %t22, <8 x i32> %t23, <8 x i32> <i32 1,
  //    i32 2,  i32 11, i32 8, i32 5, i32 6,  i32 15, i32 12>
  // %t27=%t26 // result3
  //

  const int32_t IntShuffleMask5[8] = {2, 3, 11, 10, 6, 7, 15, 14};
  const int32_t IntShuffleMask6[8] = {3, 2, 10, 11, 7, 6, 14, 15};
  const int32_t IntShuffleMask7[8] = {0, 3, 10, 9, 4, 7, 14, 13};
  const int32_t IntShuffleMask8[8] = {1, 2, 11, 8, 5, 6, 15, 12};

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask5));
  ShuffleOnShuffles[4] =
      new OVLSShuffle(ShuffleOnLoad[0], ShuffleOnLoad[1], ShuffleMask);

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask6));
  ShuffleOnShuffles[5] =
      new OVLSShuffle(ShuffleOnLoad[3], ShuffleOnLoad[2], ShuffleMask);

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask7));
  ShuffleOnShuffles[6] =
      new OVLSShuffle(ShuffleOnShuffles[4], ShuffleOnShuffles[5], ShuffleMask);

  ShuffleMask = new OVLSConstant(OVLSType(32, 8),
                                 reinterpret_cast<const int8_t *>(IntShuffleMask8));
  ShuffleOnShuffles[7] =
      new OVLSShuffle(ShuffleOnShuffles[4], ShuffleOnShuffles[5], ShuffleMask);

  Results[2] = ShuffleOnShuffles[6];
  Results[3] = ShuffleOnShuffles[7];

  for (int i = 0; i < 8; i++) {
    InstVector.push_back(ShuffleOnShuffles[i]);
  }

  if (MemrefToInstMap)
    // Populate the memrefmap.
    for (int i = 0; i < 4; i++)
      MemrefToInstMap->insert(std::pair<OVLSMemref *, OVLSInstruction *>(
          Group.getMemref(i), Results[i]));
  return true;
}

// Function with hard coded optimized sequences for the following recognized
// interleaved pattern:
//
// %store.data0 = shufflevector <8 x i32> %1, <8 x i32> %2, <16 x i32> <i32
//  0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10,
//  i32 11, i32 12, i32 13, i32 14, i32 15>
// %store.data1 = shufflevector <8 x i32> %3, <8 x i32> %4, <16 x i32>
//  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32
//  10, i32 11, i32 12, i32 13, i32 14, i32 15>
// %interleave.vec = shufflevector <16 x i32> %store.data0, <16 x i32>
//    %store.data1, <32 x i32> <i32 0, i32 8, i32 16, i32 24, i32 1, i32 9,
//    i32 17, i32 25, i32 2, i32 10, i32 18, i32 26, i32 3, i32 11, i32 19, i32
//    27, i32 4, i32 12, i32 20, i32 28, i32 5, i32 13, i32 21, i32 29, i32 6,
//    i32 14, i32 22, i32 30, i32 7, i32 15, i32 23, i32 31>
// store <32 x i32> %interleave.vec, <32 x i32>* %wide.vec.ptr, align 16
// Note: Shuffle vector instructions are generated with OVLSOperands of kind:
// 1. Undef : Undef is used for undef LLVMIR operands.
// Both Operands cannot be undef.
// 2. OVLSAddress : This is used when the operand is derived from an OVLSMemref.
// The client needs to link the corresponding instruction for the OVLSMemref. If
// it is not available, it needs to generate the required instructions. Eg:
// Shuffles when dealing with Adjacent Stores.
// 3. OVLSInstruction : This is used when the operand is the result of another
// OVLSInstruction - usually an OVLSLoad or OVLSShuffle.
//
// The use of OVLSAddress helps in 2 ways:
// First, The sequence is client independent. Eg, this can be called by a
// client dealing with gathers instead of shufflevectors.
// Second, The sequence will not consider these shuffle vectors in cost
// computation for the optimization.
bool OptVLSInterface::genSeqStoreStride16Packed8xi32(
    const OVLSGroup &Group, OVLSInstructionVector &InstVector) {

  OVLSInstruction *ShuffleOnShuffles[16];

  // Create the OVLSAddress with OVLSMemrefs as needed (as OVLSOperands) for the
  // shuffles that follow.
  OVLSAddress *MemrefAddresses[4];
  int64_t Offset = 0;
  // %t0 : Corresponding to Memref0 a[4*i]
  // %t1 : Corresponding to Memref1 a[4*i+1]
  // %t2 : Corresponding to Memref2 a[4*i+2]
  // %t3 : Corresponding to Memref3 a[4*i+3]
  for (int i = 0; i < 4; i++) {
    MemrefAddresses[i] = new OVLSAddress(Group.getMemref(i), Offset);
    Offset = Offset + 4; // i32 = 4 bytes.
  }

  // %t4 = shufflevector <8 x i32> %t0, <8 x i32> %t1, <8 x i32> <i32 0, i32
  //        8, i32 1, i32 9, i32 4, i32 12,  i32 5, i32 13>
  // %t5 = shufflevector <8 x i32> %t2, <8 x i32> %t3, <8 x i32> <i32 0, i32
  //        8, i32 1, i32 9, i32 4, i32 12,  i32 5, i32 13>
  // %t6 = shufflevector <8 x i32> %t4, <8 x i32> %t5, <8 x i32> <i32 0, i32
  //        1, i32 8, i32 9, i32 4, i32 5,  i32 12, i32 13>
  // %t7 = shufflevector <8 x i32> %t4, <8 x i32> %t5, <8 x i32> <i32 2, i32
  //        3, i32 10, i32 11, i32 6, i32 7,  i32 14, i32 15>
  const int32_t IntShuffleMask[6][8] = {
      /*IntShuffleMask[0][8] =*/{0, 8, 1, 9, 4, 12, 5, 13},
      /*IntShuffleMask[1][8] =*/{0, 1, 8, 9, 4, 5, 12, 13},
      /*IntShuffleMask[2][8] =*/{2, 3, 10, 11, 6, 7, 14, 15},
      /*IntShuffleMask[3][8] =*/{2, 10, 3, 11, 6, 14, 7, 15},
      /*IntShuffleMask[4][8] =*/{0, 1, 8, 9, 4, 5, 12, 13},
      /*IntShuffleMask[5][8] =*/{2, 3, 10, 11, 6, 7, 14, 15}};

  for (int i = 0; i < 2; ++i) {

    OVLSConstant *ShuffleMask = new OVLSConstant(
        OVLSType(32, 8),
        reinterpret_cast<const int8_t *>(IntShuffleMask[3 * i]));
    ShuffleOnShuffles[4 * i] =
        new OVLSShuffle(MemrefAddresses[0], MemrefAddresses[1], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8),
        reinterpret_cast<const int8_t *>(IntShuffleMask[3 * i]));
    ShuffleOnShuffles[4 * i + 1] =
        new OVLSShuffle(MemrefAddresses[2], MemrefAddresses[3], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8),
        reinterpret_cast<const int8_t *>(IntShuffleMask[3 * i + 1]));
    ShuffleOnShuffles[4 * i + 2] = new OVLSShuffle(
        ShuffleOnShuffles[4 * i], ShuffleOnShuffles[4 * i + 1], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8),
        reinterpret_cast<const int8_t *>(IntShuffleMask[3 * i + 2]));
    ShuffleOnShuffles[4 * i + 3] = new OVLSShuffle(
        ShuffleOnShuffles[4 * i], ShuffleOnShuffles[4 * i + 1], ShuffleMask);
  }
  // %t12= shufflevector <8 x i32> %t6, <8 x i32> %t6, <4 x i32> <i32 0, i32 1,
  //        i32 2, i32 3>
  // %t13= shufflevector <8 x i32> %t7, <8 x i32> %t7, <4 x i32> <i32 0, i32 1,
  //        i32 2, i32 3>
  // %t14= shufflevector <8 x i32> %t10, <8 x i32> %t10, <4 x i32> <i32 0, i32
  //        1,  i32 2, i32 3>
  // %t15= shufflevector <8 x i32> %t11, <8 x i32> %t11, <4 x i32> <i32 0, i32
  //        1,  i32 2, i32 3>
  // %t16= shufflevector <8 x i32> %t6, <8 x i32> %t6, <4 x i32> <i32 4, i32 5,
  //        i32 6, i32 7>
  // %t17= shufflevector <8 x i32> %t7, <8 x i32> %t7, <4 x i32> <i32 4, i32 5,
  //        i32 6, i32 7>
  // %t18= shufflevector <8 x i32> %t10 <8 x i32> %t10, <4 x i32> <i32 4, i32 5,
  //        i32 6, i32 7>
  // %t19= shufflevector <8 x i32> %11, <8 x i32> %t11, <4 x i32> <i32 4, i32 5,
  //        i32 6, i32 7>

  const int32_t IntShuffleMask2[2][4] = {
      /*IntShuffleMask2[0][4] =*/{0, 1, 2, 3},
      /*IntShuffleMask2[1][4] =*/{4, 5, 6, 7},
  };

  for (int i = 0; i < 2; i++) {
    OVLSConstant *ShuffleMask = new OVLSConstant(
        OVLSType(32, 4), reinterpret_cast<const int8_t *>(IntShuffleMask2[i]));
    ShuffleOnShuffles[4 * i + 8] = new OVLSShuffle(
        ShuffleOnShuffles[2], ShuffleOnShuffles[2], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 4), reinterpret_cast<const int8_t *>(IntShuffleMask2[i]));
    ShuffleOnShuffles[4 * i + 9] = new OVLSShuffle(
        ShuffleOnShuffles[3], ShuffleOnShuffles[3], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 4), reinterpret_cast<const int8_t *>(IntShuffleMask2[i]));
    ShuffleOnShuffles[4 * i + 10] = new OVLSShuffle(
        ShuffleOnShuffles[6], ShuffleOnShuffles[6], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 4), reinterpret_cast<const int8_t *>(IntShuffleMask2[i]));
    ShuffleOnShuffles[4 * i + 11] = new OVLSShuffle(
        ShuffleOnShuffles[7], ShuffleOnShuffles[7], ShuffleMask);
  }

  // Pushback all Shuffle instructions into InstVec.
  //
  for (int i = 0; i < 16; i++)
    InstVector.push_back(ShuffleOnShuffles[i]);

  Offset = 0;
  OVLSInstruction *StoreInst[8];
  uint64_t ElementMask = 0x0F;
  OVLSType LType(32, 4);

  // Create the store instructions.
  // %t12 store to base
  // %t13 store to base+4elem
  // %t14 store to base+8elem
  // %t15 store to base+12elem
  // %t16 store to base+16elem
  // %t17 store to base+20elem
  // %t18 store to base+24elem
  // %t19 store to base+28elem

  for (int i = 0; i < 8; i++) {
    OVLSMemref *FirstMemref = Group.getFirstMemref();
    OVLSAddress Src(FirstMemref, Offset);
    StoreInst[i] = new OVLSStore(ShuffleOnShuffles[i + 8], Src, ElementMask);
    Offset = Offset + 16; // 4 elements offset.
    InstVector.push_back(StoreInst[i]);
  }
  return true;
}

// Function with hard coded sequences for the recognized interleaved pattern:
//  %wide.vec = load <64 x i16>, <64 x i16>* %tmp, align 2
//  %v0 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56>
//  %v1 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      1, i32 9, i32 17, i32 25, i32 33, i32 41, i32 49, i32 57>
//  %v2 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      2, i32 10, i32 18, i32 26, i32 34, i32 42, i32 50, i32 58>
//  %v3 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      3, i32 11, i32 19, i32 27, i32 35, i32 43, i32 51, i32 59>
//  %v4 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      4, i32 12, i32 20, i32 28, i32 36, i32 44, i32 52, i32 60>
//  %v5 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      5, i32 13, i32 21, i32 29, i32 37, i32 45, i32 53, i32 61>
//  %v6 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      6, i32 14, i32 22, i32 30, i32 38, i32 46, i32 54, i32 62>
//  %v7 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
//      7, i32 15, i32 23, i32 31, i32 39, i32 47, i32 55, i32 63>
bool OptVLSInterface::genSeqLoadStride16Packed8xi16(
    const OVLSGroup &Group, OVLSInstructionVector &InstVector,
    OVLSMemrefToInstMap *MemrefToInstMap) {

  // Generate the instructions and push them into the InstVector, and make a
  // MemrefToInstMap.
  // I. Consecutive Loads:
  // %t0  = load from base <8xi16>
  // %t8  = base+8elem
  // %t16 = base+16elem
  // %t24 = base+24elem
  // %t32 = base+32elem
  // %t40 = base+40elem
  // %t48 = base+48elem
  // %t54 = base+56elem
  // Base gets provided by Client later when generating the LLVM IR.
  OVLSInstruction *LoadInst[8];
  OVLSInstruction *ShuffleOnLoad[8];
  const uint64_t ElementMask = 0x0FF;
  OVLSType LType(16, 8); // <8xi16>
  int64_t Offset = 0;

  for (int i = 0; i < 8; i++) {
    OVLSMemref *FirstMemref = Group.getFirstMemref();
    OVLSAddress Src(FirstMemref, Offset);
    LoadInst[i] = new OVLSLoad(LType, Src, ElementMask);
    Offset = Offset + 16; // 8 elements offset.
    InstVector.push_back(LoadInst[i]);
  }

  // %x1 = shufflevector <8 x i16> %t0, <8 x i16> %t8, <8 x i32>
  //        <i32 0, i32 8, i32 1, i32 9,  i32 2, i32 10, i32 3, i32 11>
  // %x2 = shufflevector <8 x i16> %t0, <8 x i16> %t8, <8 x i32>
  //        <i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  // %y1 = shufflevector <8 x i16> %t16, <8 x i16> %t24, <8 x i32>
  //        <i32 0, i32 8, i32 1, i32 9,  i32 2, i32 10, i32 3, i32 11>
  // %y2 = shufflevector <8 x i16> %t16, <8 x i16> %t24, <8 x i32>
  //        <i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  // %z1 = shufflevector <8 x i16> %t32, <8 x i16> %t40, <8 x i32>
  //        <i32 0, i32 8,  i32 1, i32 9,  i32 2, i32 10, i32 3, i32 11>
  // %z2 = shufflevector <8 x i16> %t32, <8 x i16> %t40, <8 x i32>
  //        <i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  // %a1 = shufflevector <8 x i16> %t48, <8 x i16> %t54, <8 x i32>
  //        <i32 0, i32 8,  i32 1, i32 9,  i32 2, i32 10, i32 3, i32 11>
  // %a2 = shufflevector <8 x i16> %t48, <8 x i16> %t54, <8 x i32>
  //        <i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  OVLSConstant *ShuffleMask;
  const int32_t IntShuffleMask0[8] = {0, 8, 1, 9, 2, 10, 3, 11};
  const int32_t IntShuffleMask1[8] = {4, 12, 5, 13, 6, 14, 7, 15};
  for (int i = 0; i < 4; i++) {

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask0));
    ShuffleOnLoad[2 * i] =
        new OVLSShuffle(LoadInst[2 * i], LoadInst[2 * i + 1], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask1));
    ShuffleOnLoad[2 * i + 1] =
        new OVLSShuffle(LoadInst[2 * i], LoadInst[2 * i + 1], ShuffleMask);
  }

  for (int i = 0; i < 8; i++) {
    InstVector.push_back(ShuffleOnLoad[i]);
  }

  //  %t1 = shufflevector <8 x i16> %x1, <8 x i16> %y1, <8 x i32> <i32 0, i32 1,
  //        i32 8, i32 9,  i32 2, i32 3, i32 10, i32 11>
  //  %t2 = shufflevector <8 x i16> %x1, <8 x i16> %y1, <8 x i32> <i32 4, i32 5,
  //        i32 12, i32 13,  i32 6, i32 7, i32 14, i32 15>
  //
  //  %t3 = shufflevector <8 x i16> %z1, <8 x i16> %a1, <8 x i32> <i32 0, i32 1,
  //        i32 8, i32 9,  i32 2, i32 3, i32 10, i32 11>
  //  %t4 = shufflevector <8 x i16> %z1, <8 x i16> %a1, <8 x i32> <i32 4, i32 5,
  //        i32 12, i32 13,  i32 6, i32 7, i32 14, i32 15>
  //
  //  %t5 = shufflevector <8 x i16> %x2, <8 x i16> %y2, <8 x i32> <i32 0, i32 1,
  //        i32 8, i32 9,  i32 2, i32 3, i32 10, i32 11>
  //  %t6 = shufflevector <8 x i16> %x2, <8 x i16> %y2, <8 x i32> <i32 4, i32 5,
  //        i32 12, i32 13,  i32 6, i32 7, i32 14, i32 15>
  //
  //  %t7 = shufflevector <8 x i16> %z2, <8 x i16> %a2, <8 x i32> <i32 0, i32 1,
  //        i32 8, i32 9,  i32 2, i32 3, i32 10, i32 11>
  //  %t8 = shufflevector <8 x i16> %z2, <8 x i16> %a2, <8 x i32> <i32 4, i32 5,
  //        i32 12, i32 13,  i32 6, i32 7, i32 14, i32 15>

  OVLSInstruction *ShuffleOnShuffles[8];

  const int32_t IntShuffleMask2[8] = {0, 1, 8, 9, 2, 3, 10, 11};
  const int32_t IntShuffleMask3[8] = {4, 5, 12, 13, 6, 7, 14, 15};

  for (int i = 0; i < 2; i++) {
    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask2));
    ShuffleOnShuffles[4 * i] =
        new OVLSShuffle(ShuffleOnLoad[i], ShuffleOnLoad[i + 2], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask3));
    ShuffleOnShuffles[4 * i + 1] =
        new OVLSShuffle(ShuffleOnLoad[i], ShuffleOnLoad[i + 2], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask2));
    ShuffleOnShuffles[4 * i + 2] = new OVLSShuffle(
        ShuffleOnLoad[i + 4], ShuffleOnLoad[i + 6], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask3));
    ShuffleOnShuffles[4 * i + 3] = new OVLSShuffle(
        ShuffleOnLoad[i + 4], ShuffleOnLoad[i + 6], ShuffleMask);
  }

  for (int i = 0; i < 8; i++) {
    InstVector.push_back(ShuffleOnShuffles[i]);
  }

  //  %r0 = shufflevector <8 x i16> %t1, <8 x i16> %t3, <8 x i32> <i32 0, i32 1,
  //        i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  //
  //  %r1 = shufflevector <8 x i16> %t1, <8 x i16> %t3, <8 x i32> <i32 4, i32 5,
  //        i32 6, i32 7, i32 12, i32 13, i32 14, i32 15>
  //
  //  %r2 = shufflevector <8 x i16> %t2, <8 x i16> %t4, <8 x i32> <i32 0, i32 1,
  //        i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  //
  //  %r3 = shufflevector <8 x i16> %t2, <8 x i16> %t4, <8 x i32> <i32 4, i32 5,
  //        i32 6, i32 7, i32 13, i32 13, i32 14, i32 15>
  //
  //  %r4 = shufflevector <8 x i16> %t5, <8 x i16> %t7, <8 x i32> <i32 0, i32 1,
  //        i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  //
  //  %r5 = shufflevector <8 x i16> %t5, <8 x i16> %t7, <8 x i32> <i32 4, i32 5,
  //        i32 6, i32 7, i32 12, i32 13, i32 14, i32 15>
  //
  //  %r6 = shufflevector <8 x i16> %t6, <8 x i16> %t8, <8 x i32> <i32 0, i32 1,
  //        i32 2, i32 3, i32 8, i32 9, i32 10, i32 11>
  //
  //  %r7 = shufflevector <8 x i16> %t6, <8 x i16> %t8, <8 x i32> <i32 4, i32 5,
  //        i32 6, i32 7, i32 12, i32 13, i32 14, i32 15>
  //
  OVLSInstruction *ResultShuffles[8];

  const int32_t IntShuffleMask4[8] = {0, 1, 2, 3, 8, 9, 10, 11};
  const int32_t IntShuffleMask5[8] = {4, 5, 6, 7, 12, 13, 14, 15};

  for (int i = 0; i < 2; i++) {
    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask4));
    ResultShuffles[4 * i] = new OVLSShuffle(
        ShuffleOnShuffles[4 * i], ShuffleOnShuffles[4 * i + 2], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask5));
    ResultShuffles[4 * i + 1] = new OVLSShuffle(
        ShuffleOnShuffles[4 * i], ShuffleOnShuffles[4 * i + 2], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask4));
    ResultShuffles[4 * i + 2] =
        new OVLSShuffle(ShuffleOnShuffles[4 * i + 1],
                        ShuffleOnShuffles[4 * i + 3], ShuffleMask);

    ShuffleMask = new OVLSConstant(
        OVLSType(32, 8), reinterpret_cast<const int8_t *>(IntShuffleMask5));
    ResultShuffles[4 * i + 3] =
        new OVLSShuffle(ShuffleOnShuffles[4 * i + 1],
                        ShuffleOnShuffles[4 * i + 3], ShuffleMask);
  }

  for (int i = 0; i < 8; i++) {
    InstVector.push_back(ResultShuffles[i]);
  }

  if (MemrefToInstMap)
    // Populate the memrefmap.
    for (int i = 0; i < 8; i++)
      MemrefToInstMap->insert(std::pair<OVLSMemref *, OVLSInstruction *>(
          Group.getMemref(i), ResultShuffles[i]));
  return true;
}

// This is for hand coded sequences.
// Group is input.
// InstVector is the vector containing the generated shuffles.
// MemrefToInstMap is the memref to Corresponding Instructions for a load
// group.
bool OptVLSInterface::getSequencePredefined(
    const OVLSGroup &Group, OVLSInstructionVector &InstVector,
    OVLSMemrefToInstMap *MemrefToInstMap) {

  // Recognize the group pattern and check if it matches any predefined pattern.
  // If the pattern is supported, call the correct function that lowers it.

  // Pattern To Recognize:
  //%wide.vec = load <32 x i32>, <32 x i32>* %wide.vec.ptr, align 16
  //%r1 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
  //  <i32 0, i32 4, i32 8,  i32 12, i32 16, i32 20, i32 24, i32 28>
  //%r2 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
  //  <i32 1, i32 5, i32 9,  i32 13, i32 17, i32 21, i32 25, i32 29>
  //%r3 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
  //  <i32 2, i32 6, i32 10, i32 14, i32 18, i32 22, i32 26, i32 30>
  //%r4 = shufflevector <32 x i32> %wide.vec, <32 x i32> undef,<8 x i32>
  //  <i32 3, i32 7, i32 11, i32 15, i32 19, i32 23, i32 27, i32 31>
  // Has a constant stride of value 16. i32*4 = 16 bytes.
  // Group size is 4. 4 memrefs.
  // Number elements = 8
  // Elem Size = 32 bits
  // Accessmask to be used for checking the gaps in memory accesses. All 1's
  // indicate that all members are present, and there are no gaps. AccessMask is
  // therefore 65535.
  // Memaccess is Strided Load.
  // No target specific information is needed here. The sequence is target
  // independent for now.
  Optional<int64_t> Stride = Group.getConstStride();
  if (!Stride)
    return false;

  if (*Stride == 16 && Group.getNumElems() == 8 && Group.getElemSize() == 32 &&
      // TODO: add utility function that checks for all ones using compare mask
      // of size stride instead of hard-coded 65535. See other uses of
      // getNByteAccessMask.
      Group.getNByteAccessMask() == 65535 &&
      Group.getAccessKind() == OVLSAccessKind::SLoad) {
    // Sequence can be safely generated.
    return genSeqLoadStride16Packed8xi32(Group, InstVector, MemrefToInstMap);
  }

  // Pattern To Recognize:
  //  %wide.vec = load <64 x i16>, <64 x i16>* %tmp, align 2
  //  %v0 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      0, i32 8, i32 16, i32 24, i32 32, i32 40, i32 48, i32 56>
  //  %v1 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      1, i32 9, i32 17, i32 25, i32 33, i32 41, i32 49, i32 57>
  //  %v2 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      2, i32 10, i32 18, i32 26, i32 34, i32 42, i32 50, i32 58>
  //  %v3 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      3, i32 11, i32 19, i32 27, i32 35, i32 43, i32 51, i32 59>
  //  %v4 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      4, i32 12, i32 20, i32 28, i32 36, i32 44, i32 52, i32 60>
  //  %v5 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      5, i32 13, i32 21, i32 29, i32 37, i32 45, i32 53, i32 61>
  //  %v6 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      6, i32 14, i32 22, i32 30, i32 38, i32 46, i32 54, i32 62>
  //  %v7 = shufflevector <64 x i16> %wide.vec, <64 x i16> undef, <8 x i32> <i32
  //      7, i32 15, i32 23, i32 31, i32 39, i32 47, i32 55, i32 63>
  // Has a constant stride of value 16. i16*8 = 16 bytes.
  // Group size is 8. 8 memrefs.
  // Number elements = 8
  // Elem Size = 16 bits
  // Accessmask to be used for checking the gaps in memory accesses. All 1's
  // indicate that all members are present, and there are no gaps. AccessMask is
  // therefore 65535.
  // MemAccess is Strided Load.
  // No target specific information is needed here. The sequence is target
  // independent for now.
  if (*Stride == 16 && Group.getNumElems() == 8 && Group.getElemSize() == 16 &&
      Group.getNByteAccessMask() == 65535 &&
      Group.getAccessKind() == OVLSAccessKind::SLoad) {
    // Sequence can be safely generated.
    return genSeqLoadStride16Packed8xi16(Group, InstVector, MemrefToInstMap);
  }

  // Pattern To Recognize:
  // %store.data0 = shufflevector <8 x i32> %1, <8 x i32> %2, <16 x i32> <i32
  //  0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10,
  //  i32 11, i32 12, i32 13, i32 14, i32 15>
  // %store.data1 = shufflevector <8 x i32> %3, <8 x i32> %4, <16 x i32>
  //  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32
  //  10, i32 11, i32 12, i32 13, i32 14, i32 15>
  // %interleave.vec = shufflevector <16 x i32> %store.data0, <16 x i32>
  //    %store.data1, <32 x i32> <i32 0, i32 8, i32 16, i32 24, i32 1, i32 9,
  //    i32
  //  17, i32 25, i32 2, i32 10, i32 18, i32 26, i32 3, i32 11, i32 19, i32 27,
  //  i32 4, i32 12, i32 20, i32 28, i32 5, i32 13, i32 21, i32 29, i32 6, i32
  //  14, i32 22, i32 30, i32 7, i32 15, i32 23, i32 31>
  // store <32 x i32> %interleave.vec, <32 x i32>* %wide.vec.ptr, align 16,
  // Has a constant stride of value 16. i32*4 = 16 bytes.
  // Number Elements = 8
  // Elem Size = 32 bits
  // Accessmask to be used for checking the gaps in memory accesses. All 1's
  // indicate that all members are present, and there are no gaps. AccessMask is
  // therefore 65535.
  // Memaccess is Strided Store.
  // No target specific information is needed here. The sequence is target
  // independent for now.
  if (*Stride == 16 && Group.getNumElems() == 8 && Group.getElemSize() == 32 &&
      Group.getNByteAccessMask() == 65535 &&
      Group.getAccessKind() == OVLSAccessKind::SStore) {
    // Sequence can be safely generated.
    return genSeqStoreStride16Packed8xi32(Group, InstVector);
  }

  return false;
}

// The members of \p Group can be either vectorized individually using a
// gather/scatter each, or can be vectorized together with their neighbors
// in the Group using wide loads/stores and shuffles. This function
// returns the cost of vectorizing the Group with Wide loads/stores and shuffles
// sequences, if the Group is supported. If not supported, it returns an
// absolute maximum value. (The client needs to compare it with the
// gatherScatterOpCost() to understand if it will be profitable to do the VLS
// transformation).
int64_t OptVLSInterface::getGroupCost(const OVLSGroup &Group,
                                      const OVLSCostModel &CM) {
  int64_t Cost = 0;

  // 1. Obtain the cost of vectorizing this group using wide loads/stores
  // + shuffle-sequence
  OVLSInstructionVector InstVector;
  if (getSequence(Group, CM, InstVector)) {
    for (OVLSInstruction *I : InstVector) {
      int64_t C = CM.getInstructionCost(I);
      if (C == OVLSCostModel::UnknownCost)
        return OVLSCostModel::UnknownCost;
      Cost += C;
    }
    OVLSDebug(OVLSdbgs() << "Optimized Cost for the Group = " << Cost << "\n");
    // If the returned cost is 0, it implies that the CostModel isn't accurate
    // and should do a better job in estimating the cost.
    return Cost;
  }
  // 2. If the Optimized Sequence isn't present, return Unknown Cost.
  OVLSDebug(OVLSdbgs() << "Returning UnknownCost for the Group.\n");
  return OVLSCostModel::UnknownCost;
}
