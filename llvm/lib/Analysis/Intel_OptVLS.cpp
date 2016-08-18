//===- OptVLS.cpp - Optimization of Vector Loads/Stores ----------*- C++ -*-===//
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
/// The core functionality of Opt_VLS is provided by three functions: getGroups(),
/// getCost(), getSequence(). This implementation is IR and machine agnostic.
/// For any required information to compute groups, cost and sequence OptVLS
/// communicates to its clients.
///
//===---------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptVLS.h"
#define DEBUG_TYPE "ovls"
#include <deque>

using namespace llvm;

// MemrefDistanceMap contains a set of distances where each distance gets
// mapped to a memref. Basically, it contains a set of adjacent memrefs.
// Keeping them in a map helps eliminate any duplicate distances and keeps them
// sorted.
typedef OVLSMap<int, OVLSMemref*> MemrefDistanceMap;
typedef MemrefDistanceMap::iterator MemrefDistanceMapIt;

// MemrefDistanceMapVector is a vector of groups where each group contains a set
// of adjacent memrefs.
typedef OVLSVector<MemrefDistanceMap*> MemrefDistanceMapVector;
typedef MemrefDistanceMapVector::iterator MemrefDistanceMapVectorIt;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void OVLSAccessType::dump() const {
  print(OVLSdbgs());
  OVLSdbgs() << '\n';
}
#endif

void OVLSAccessType::print(OVLSostream &OS) const {
  switch(AccType) {
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
                       const OVLSAccessType& AType)
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

namespace OptVLS {
  class GraphNode;
  /// Represents a range of bits using a bit-location of the leftmost bit and
  /// a number of consecutive bits immediately to the right that are included
  /// in the range. {0, 0} means undefined bit-range.
  class BitRange {
    uint32_t BIndex;
    uint32_t NumBits;

  public:
    BitRange(uint32_t BI, uint32_t NBits) {
      assert(BI % BYTE == 0 && "BitIndex in a BitRange needs to be divisible by a byte size!!!");
      assert(NBits % BYTE == 0 && "NumBits in a BitRange needs to be divisible by a byte size!!!");
      BIndex = BI;
      NumBits = NBits;
    }

    // Increment BitIndex by NumBits.
    BitRange& operator++() {
      BIndex += NumBits;
      return *this;
    }
    uint32_t getNumBits() const { return NumBits; }
    uint32_t getBitIndex() const { return BIndex; }
  };

  /// print BitRange as "Start_BitIndex : End_BitIndex"
  static inline OVLSostream &operator<<(OVLSostream &OS,
                                        const BitRange &BR) {
    OS << BR.getBitIndex() << ":" << BR.getBitIndex() + BR.getNumBits() - 1;
    return OS;
  }

  /// Edge represents a move of a specified bit-range 'BR' from 'Src' GraphNode.
  /// Src can be nullptr, which means an undefined source. For an undefined
  /// source, BR still represents a valid bitrange. A bit-range with an
  /// undefined source is used to represent a gap in the destination GraphNode.
  class Edge {
  private:
    GraphNode *Src;
    BitRange BR;

  public:
    Edge(GraphNode *S, BitRange BRange) : Src(S), BR(BRange) {
      BR = BitRange(BRange.getBitIndex(), BRange.getNumBits());
    }
    GraphNode* getSource() const { return Src; }
    BitRange getBitRange() const { return BR; }
  };

  /// GraphNode can be thought of as a result of some logical instruction
  /// (mainly rearrangement instruction such as shift, shuffle, etc) on
  /// its ‘IncomingEdges’(/source bit-ranges). These ‘IncomingEdges’
  /// particularly show which source bit-range maps to which bit-index of this
  /// (which helps defining (/elaborates on) the logical instruction semantics).
  /// A ‘GraphNode’ basically allows us to define an expected behavior
  /// (/semantic) first which then evolves into a particular valid OVLSinstruction
  /// ‘Inst’ if there is any for that semantic. A node is allowed to have any
  /// permutation of bit-ranges coming from a maximum of two source nodes.
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

    /// Current total number of incoming bits from IncomingEdges.
    uint32_t TotalIncomingEdgeBits;

  public:
    explicit GraphNode(OVLSInstruction *I) : Inst(I) {
      static uint32_t NodeId = 1;
      Id = NodeId++;
      TotalIncomingEdgeBits = 0;
    }

    ~GraphNode() {
      for (Edge *E : IncomingEdges)
        delete E;
    }

    typedef OVLSVector<Edge*>::iterator iterator;
    inline iterator                begin() { return IncomingEdges.begin(); }
    inline iterator                end  () { return IncomingEdges.end();   }

    uint32_t getId() const { return Id; }
    OVLSInstruction* getInstruction() const { return Inst; }
    bool isALoad() const { return Inst && isa<OVLSLoad>(Inst); }

    // Returns the current total number of incoming bits.
    uint32_t size() const { return TotalIncomingEdgeBits; }

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
        OS << " [" << DstBR << "] " << "= V";
        if (E->getSource()) {
          OS << E->getSource()->getId();
          OS << "[" << SrcBR << "]\n";
        } else
          OS << "UnDef";

        CurrentBitIndex += SrcBR.getNumBits();
      }
    }

    // Assigns an edge 'E' to the BIndex if BIndex is available,
    // If BIndex is larger than the current bit index, it fills up the gap by
    // creating a dummy edge. This function does not allow overwriting an edge,
    // it throws an exception in such case.
    void addAnIncomingEdge(uint32_t BIndex, Edge *E) {
      assert(E != nullptr && "Invalid Edge!!!");
      assert(BIndex >= TotalIncomingEdgeBits && "Overwriting an (element)edge is not allowed!!!");

      if (BIndex > TotalIncomingEdgeBits) {
        // BIndex is larger than the current bit-index, create a dummy edge
        // to fillup the gap.
        uint32_t GapSize = (BIndex - TotalIncomingEdgeBits);

        Edge *Gap = new Edge(nullptr, BitRange(0, GapSize));
        IncomingEdges.push_back(Gap);
        TotalIncomingEdgeBits += GapSize;
      }

      IncomingEdges.push_back(E);
      // Update current bit-index.
      TotalIncomingEdgeBits += E->getBitRange().getNumBits();
    }
  }; // end of GraphNode

  typedef OVLSVector<GraphNode*> GraphNodeVector;
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
    /// appear before a source node in the NodeVector.
    GraphNodeVector Nodes;

  public:
    ~Graph() {
      for (GraphNode *N : Nodes)
        delete N;
    }
    void insert(GraphNode *N) {
      Nodes.push_back(N);
    }

    // Return nodes in a topological order.
    // FIXME: Current topological traversal is very inefficient, support a
    // linear time topological walk by adding a member of a vector of outgoing
    // edges to the GraphNode.
    void getTopSortedNodes(GraphNodeVector& TopSortedNodes) const {
      std::deque<GraphNode *> NodesToProcess;
      OVLSSmallPtrSet<const GraphNode *> ProcessedNodes;

      // Get the nodes with no incoming edges.
      for (GraphNode *Node : Nodes) {
        if (Node->begin() == Node->end()) {
          TopSortedNodes.push_back(Node);
          ProcessedNodes.insert(Node);
        }
        else
          NodesToProcess.push_back(Node);
      }

      // Get the nodes if its producers are processed.
      while (!NodesToProcess.empty()) {
        GraphNode *Node = NodesToProcess.front();
        NodesToProcess.pop_front();

        // check if its producers are processed
        bool ProducersProcessed = true;
        for (GraphNode::iterator I = Node->begin(), E = Node->end(); I != E; ++I) {
          GraphNode *Parent = (*I)->getSource();
          if (!ProcessedNodes.count(Parent)) {
            ProducersProcessed = false;
            break;
          }
        }

        if (ProducersProcessed) {
          // Its producers are already processed, so process the node.
          TopSortedNodes.push_back(Node);
          ProcessedNodes.insert(Node);
        }
        else
          // Its producers are not processed yet, push the node back to the stack
          NodesToProcess.push_back(Node);
      }
    }

    // Print the nodes in a topological order. It prints a node only if its
    // producers are printed.
    void print(OVLSostream &OS, uint32_t NumSpaces) const {
      GraphNodeVector TopSortedNodes;
      getTopSortedNodes(TopSortedNodes);

      for (GraphNode *N : TopSortedNodes) {
        N->print(OS, NumSpaces + 2);
        OS << "\n";
      }
    }

    /// Returns false if verification fails. Verification fails if total number
    /// of gathers in the \p Group does not match the total number of gather-nodes
    /// (nodes with the incoming edges) in this(graph). Verification also fails
    /// if number of bytes in a gather-node does not match the number of bytes
    /// of the correspondent gather in the group. We find this correspondence
    /// assuming the order of the gathers in a group is the same as in G).
    /// This function should only be called right after load-generation before
    /// any other optimization of the graph, otherwise, it will produce incorrect
    /// result.
    /// FIXME: Support scatters, masked gathers;
    bool verifyInitialGraph(const OVLSGroup& Group) {
      // This initial graph is considered to be invalid for a group of scatters.
      if (!Group.hasGathers())
        return false;

      uint32_t TotalGatherNodes = 0;
      for (GraphNode *N : Nodes) {
        // This initial graph has only two kinds of nodes, nodes for loads and
        // nodes for final gather results. Therefore, a node that is not a load
        // is a gather.
        if (!N->isALoad()) {
          // Found a gather-result-node
          OVLSMemref *Gather = Group.getMemref(TotalGatherNodes);
          // FIXME: currently assuming all elements are masked on elements.
          // Support masked gather.
          if (N->size() != Gather->getType().getSize())
            // Size of the gather-node does not match the size of the actual
            // gather memref.
            return false;

          // One way to verify that the elements are loaded for unmasked
          // gather is to verify there are no gaps
          for (GraphNode::iterator I = N->begin(), E = N->end(); I != E; ++I)
            if (!(*I)->getSource())
              return false;

          TotalGatherNodes++;
        }
      }

      if (TotalGatherNodes != Group.size())
        return false;

      return true;
    }

    // Currently returns only the load instructions.
    // Visit the graph in a topological order and push the load
    // instructions into the InstVector.
    // FIXME: Once getShuffles() is in, this function should return
    // all the other instructions as well. At that time, an instruction
    // can not be null, so an assertion should be thrown.
    void getInstructions(OVLSInstructionVector &InstVector) {
      GraphNodeVector TopSortedNodes;
      getTopSortedNodes(TopSortedNodes);

      for (GraphNode *N : TopSortedNodes) {
        OVLSInstruction *I = N->getInstruction();
        if (I && isa<OVLSLoad>(I))
          InstVector.push_back(I);
      }
    }

    GraphNode* getNode(uint32_t Id) const {
      assert(Id < Nodes.size() && "Invalid Id!!!");
      return Nodes[Id];
    }
  }; // end of class Graph

  static void dumpOVLSGroupVector(OVLSostream &OS, const OVLSGroupVector &Grps) {
    OS << "\n  Printing Groups- Total Groups " << Grps.size() << "\n";
    unsigned GroupId = 1;
    for (unsigned i = 0, S = Grps.size(); i < S; i++) {
      OS << "  Group#" << GroupId++ << " ";
      Grps[i]->print(OS, 3);
    }
  }
  static void dumpOVLSMemrefVector(OVLSostream &OS, const OVLSMemrefVector &MemrefVec,
                                   unsigned NumSpaces) {
    for (OVLSMemref *Memref : MemrefVec) {
      Memref->print(OS, NumSpaces);
      OS << "\n";
    }
  }
  static void dumpMemrefDistanceMapVector(OVLSostream &OS,
                                          const MemrefDistanceMapVector &AdjMemrefSetVec) {
    unsigned Id = 0;
    for (MemrefDistanceMap *AdjMemrefSet : AdjMemrefSetVec) {
      OS << "  Set #" << ++Id << "\n";
      // print each adjacent memref-set
      for (const auto& AMapElem : *AdjMemrefSet) {
        (AMapElem.second)->print(OS, 2);
        OS << "   Dist: " << AMapElem.first << "\n";
      }
    }
  }

  static unsigned genMask(unsigned Mask, unsigned shiftcount,
                          unsigned bitlocation) {
    assert(bitlocation + shiftcount <= MAX_VECTOR_LENGTH &&
          "Invalid bit location for a bytemask");

    uint64_t NewMask = (shiftcount == 64) ? ~0LL : (1LL << shiftcount) - 1;
    Mask |= NewMask << bitlocation;

    return Mask;
  }

  static void printMask(OVLSostream &OS, uint64_t Mask) {
    // Convert int AccessMask to binary
    char SRMask[MAX_VECTOR_LENGTH+1], *MaskPtr;
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
                         OVLSGroupVector &OVLSGrps,
                         unsigned VectorLength,
                         OVLSMemrefToGroupMap *MemrefToGroupMap) {
    for (MemrefDistanceMap *AdjMemrefSet : AdjMrfSetVec) {
      assert (!AdjMemrefSet->empty() && "Adjacent memref-set cannot be empty");
      MemrefDistanceMapIt AdjMemrefSetIt = (*AdjMemrefSet).begin();

      OVLSAccessType AccType = (AdjMemrefSetIt->second)->getAccessType();
      OVLSGroup *CurrGrp = new OVLSGroup(VectorLength, AccType);
      int GrpFirstMDist = AdjMemrefSetIt->first;

      // Group memrefs in each set using a greedy approach, keep inserting the
      // memrefs into the same group until the group is full. Form a new group
      // for a memref that cannot be moved into the group.
      for (MemrefDistanceMapIt E = (*AdjMemrefSet).end();
                                     AdjMemrefSetIt != E; ++AdjMemrefSetIt) {
        OVLSMemref *Memref = AdjMemrefSetIt->second;
        unsigned ElemSize = Memref->getType().getElementSize() / BYTE; // in bytes
        int Dist = AdjMemrefSetIt->first;

        uint64_t AccMask = CurrGrp->getNByteAccessMask();

        if (!CurrGrp->empty() &&
            ((Dist - GrpFirstMDist + ElemSize) > VectorLength || // capacity exceeded.
            !Memref->canMoveTo(*CurrGrp->getFirstMemref()))) {
          OVLSGrps.push_back(CurrGrp);
          CurrGrp = new OVLSGroup(VectorLength, AccType);

          // Reset GrpFirstMDist
          GrpFirstMDist = Dist;
          // Generate AccessMask
          AccMask = OptVLS::genMask(0, ElemSize, Dist - GrpFirstMDist);
        } else
          AccMask = OptVLS::genMask(AccMask, ElemSize, Dist - GrpFirstMDist);

        uint64_t ElementMask = CurrGrp->getElementMask();
        // TODO: Support heterogeneous types. Currently, heterogeneous types
        // are not supported, therefore, a group cannot have different element
        // sizes.
        ElementMask = OptVLS::genMask(ElementMask, 1,
                                      (Dist - GrpFirstMDist)/ElemSize);

        CurrGrp->insert(Memref, AccMask, ElementMask);
        if (MemrefToGroupMap)
          (*MemrefToGroupMap).insert(std::pair<OVLSMemref*, OVLSGroup*>
                                     (Memref, CurrGrp));
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
  // Adjacent memrefs in the group are sorted based on their distance from the first
  // memref in the group in an ascending order.
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

      // TODO: Currently finding the appropriate group is done using a linear search.
      // It would be better to use a hashing algorithm for this search.
      for (MemrefDistanceMap *AdjMemrefSet : AdjMemrefSetVec) {

        OVLSMemref *SetFirstSeenMrf = (*AdjMemrefSet).find(0)->second;

        if (Memref->getAccessType() == SetFirstSeenMrf->getAccessType() && // same access type
            // Memref->haveSameNumElements(*SetFirstSeenMrf) && // same number of vector elements
            // TODO: Support heterogeneous types
            Memref->getType() == SetFirstSeenMrf->getType() &&
            Memref->isAConstDistanceFrom(*SetFirstSeenMrf, &Dist)) { // are a const distance apart
          // Found a set
          AdjMrfSetFound = true;
          (*AdjMemrefSet).insert(std::pair<int, OVLSMemref*>(Dist, Memref));
          break;
        }
      }
      if (!AdjMrfSetFound) {
        // No adjacent memref set exits for this memref, create a new set.
        MemrefDistanceMap *AdjMemrefSet = new MemrefDistanceMap();
        // Assign 0 as a distance to the first memref in the set.
        (*AdjMemrefSet).insert(std::pair<int, OVLSMemref*>(0, Memref));
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
    if ((((uint64_t)1 << TotalBytes)-1) == ByteAccessMask)
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

  static bool isSupported(const OVLSGroup& Group) {
    int64_t Stride = 0;
    if (!Group.hasGathers() || !Group.hasAConstStride(Stride)) {
      OVLSDebug(OVLSdbgs() << "Optimized sequence is only supported for a group"
                            " of gathers that has a constant stride!!!\n");
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
    if ((Stride+1) < UsedBytes)
      return false;

    return true;
  }

  /// This function returns a vector of contiguous loads for a group of
  /// gathers that has a constant stride using a greedy approach. This default
  /// approach generates a contiguous vector load for each i-th elements.
  /// It also returns a mapping of the load-elements to the gather-elements
  /// using a directed graph. Each node in the graph represents either a load or
  /// a gather and each edge shows which loaded elements contribute to which
  /// gather results.
  /// FIXME: Support masked gathers.
  static void getDefaultLoads(const OVLSGroup& Group,
                              Graph &G) {
    int64_t Stride = 0;
    if (!Group.hasGathers() || !Group.hasAConstStride(Stride))
      assert("Unexpected Group!!!");

    // Create a graph-node for each gather-result.
    for (OVLSGroup::const_iterator I = Group.begin(),
                                   E = Group.end(); I != E; ++I) {
      // At this point, we don't know the desired instruction/opcode,
      // initialize it(associated OVLSInstruction) to nullptr.
      GraphNode *GatherNode = new GraphNode(nullptr);
      G.insert(GatherNode);
    }

    // Generate load mask
    uint64_t ElementMask = Group.getElementMask();
    int64_t Offset = 0;

    // Compute Load Type
    uint32_t ElemSize = Group.getElemSize();
    uint32_t NumElemsInALoad = 0;
    uint64_t EMask = ElementMask;
    while (EMask != 0) {
      EMask = EMask >> 1;
      NumElemsInALoad++;
    }
    OVLSType LoadType = OVLSType(ElemSize, NumElemsInALoad);

    uint32_t NumElems = Group.getNumElems();

    OVLSMemref *GrpFirstMemref = Group.getFirstMemref();

    uint32_t IthElem = 0;
    while (IthElem < NumElems) {
      // Generate a load for loading the contiguous bytes created by each
      // i-th accesses of the adjacent gathers.
      OVLSOperand *Src = new OVLSAddress(GrpFirstMemref, Offset);
      OVLSInstruction *MemInst = new OVLSLoad(LoadType, *Src, ElementMask);

      // Create a graph-node for a load.
      GraphNode *LoadNode = new GraphNode(MemInst);
      G.insert(LoadNode);

      // Draw edges from the load elements that show which load element
      // contributes to which gather element.
      EMask = ElementMask;
      uint32_t GSIndex = 0;
      BitRange LoadBR = BitRange(0, ElemSize);
      while (EMask) {
        if (EMask & 1) {
          // A gather element is loaded, draw an edge from this element
          // to its correspondent gather-element.
          GraphNode *GatherNode = G.getNode(GSIndex);
          Edge *E = new Edge(LoadNode, LoadBR);
          GatherNode->addAnIncomingEdge(IthElem * ElemSize, E);
        }
        ++LoadBR;
        EMask >>= 1;
        GSIndex++;
      }

      Offset += Stride;
      IthElem++;
    }
  } // end of getDefaultLoads
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
  uint32_t Counter = 0;
  while (Counter++ != NumSpaces)
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
  OS << "\n";
}

bool OVLSShuffle::hasValidOperands(OVLSOperand O1, OVLSOperand O2,
                                   OVLSOperand Mask) {
  // O1 and O2 must be vectors of the same type.
  if (!O1.getType().isValid() || O1.getType() != O2.getType())
    return false;

  // Mask needs to be a vector of constants.
  if (!Mask.getType().isValid() || !isa<OVLSConstant>(&Mask))
    return false;

  if (Mask.getType().getNumElements() >
      (O1.getType().getNumElements() + O2.getType().getNumElements()))
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
                                OVLSGroupVector &Grps,
                                unsigned VectorLength,
                                OVLSMemrefToGroupMap *MemrefToGroupMap) {
  OVLSDebug(OVLSdbgs() << "Received a request from Client---FORM GROUPS\n");

  if (Memrefs.empty()) return;

  if (VectorLength > MAX_VECTOR_LENGTH) {
    OVLSDebug(OVLSdbgs() << "!!!Group size above " << MAX_VECTOR_LENGTH <<
                             " bytes is not supported currently\n");
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
  // Also, make sure the total size of the memrefs does not exceed the group size.
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
// Current implementation only supports a group of adjacent strided-loads
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
// Only strided-loads with constant stride (a uniform distance between the vector
// elements) is supported.
//
// Right now, it only generates the load sequence, shuffle sequence are not
// supported.
bool OptVLSInterface::getSequence(const OVLSGroup& Group,
                                  OVLSInstructionVector& InstVector) {
  if (!OptVLS::isSupported(Group))
    return false;

  OptVLS::Graph G;

  OptVLS::getDefaultLoads(Group, G);

  /// At this point, we have generated loads (to load the contiguous chunks of
  /// memory created by the \p Group of gathers) and a graph that shows which
  /// loaded elements contribute to which gather results (gather-node) by drawing
  /// edges from load-nodes(nodes with load instruction) to gather-result-nodes(nodes
  /// with null instruction).
  /// To ensure that we are on the right track of producing the gather results,
  /// make sure the graph contains a node for each gather in the group with
  /// number of bytes (matches the number of bytes of the gather in the group)
  /// coming from no other nodes than load nodes.
  if (!G.verifyInitialGraph(Group))
    return false;

  G.getInstructions(InstVector);
  return true;
}
