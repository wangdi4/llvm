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
  struct BitRange {
    uint32_t BIndex;
    uint32_t NumBits;

    // Increment BitIndex by NumBits.
    BitRange& operator++() {
      BIndex += NumBits;
      return *this;
    }
  };

  /// print BitRange as "BitIndex : NumBits"
  static inline OVLSostream &operator<<(OVLSostream &OS,
                                        const BitRange &BR) {
    OS << BR.BIndex << ":" << BR.NumBits;
    return OS;
  }

  /// Edge represents a move of a specified bit-range 'BR' from 'Src' GraphNode.
  /// Src can be nullptr, which means an undefined source. For an undefined
  /// source, BR still represents a valid bitrange. A bit-range with an
  /// undefined source is used to represent a gap in the destination GraphNode.
  struct Edge {
    GraphNode *Src;
    BitRange BR;
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

  public:
    explicit GraphNode(OVLSInstruction *I) : Inst(I) {
      static uint32_t NodeId = 1;
      Id = NodeId++;
    }

    ~GraphNode() {
      for (Edge *E : IncomingEdges)
        delete E;
    }

    typedef OVLSVector<Edge*>::iterator iterator;
    inline iterator                begin() { return IncomingEdges.begin(); }
    inline iterator                end  () { return IncomingEdges.end();   }

    uint32_t getId() const { return Id; }

    // print GraphNode;
    void print(OVLSostream &OS, uint32_t NumSpaces) const {
      uint32_t Counter = 0;
      while (Counter++ != NumSpaces)
          OS << " ";
      OS << "V" << Id << ":";

      // print the associated instruction.
      if (Inst)
        if (isa<OVLSLoad>(Inst)) OS << " Load ";

      // print the incoming edges as [BIndex] = Srd-Id[BR].
      uint32_t CurrentBitIndex = 0;
      for (Edge *E : IncomingEdges) {
        BitRange SrcBR = E->BR;
        OS << " [" << CurrentBitIndex << "] " << "= V";
        OS << E->Src->getId() << "[" << SrcBR << "] ";
        CurrentBitIndex += SrcBR.NumBits;
      }
    }
    /// FIXME
    void isValid() {}
  };

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

    // print the nodes.
    void print(OVLSostream &OS, uint32_t NumSpaces) const {
      for (uint32_t i = 0; i < Nodes.size(); ++i) {
        Nodes[i]->print(OS, NumSpaces + 2);
        printf("\n");
      }
    }

    GraphNode* getNode(uint32_t Id) const {
      return Nodes[Id];
    }
  };

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
  static void getDefaultLoads(const OVLSGroup& Group,
                              OVLSInstructionVector &InstVector) {
    int64_t Stride = 0;
    if (!Group.hasGathers() || !Group.hasAConstStride(Stride))
      assert("Unexpected Group!!!");

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

    int32_t MemrefVectorLength = Group.getNumElems();

    OVLSMemref *GrpFirstMemref = Group.getFirstMemref();

    while (MemrefVectorLength-- > 0) {
      OVLSOperand *Src = new OVLSAddress(GrpFirstMemref, Offset);
      OVLSInstruction *MemInst = new OVLSLoad(LoadType, *Src, ElementMask);
      InstVector.push_back(MemInst);
      Offset += Stride;
    }
  }

} // end of namespace

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

  OptVLS::getDefaultLoads(Group, InstVector);

  return true;
}

// The members of Group can be either vectorized individually using a 
// gather/scatter each, or can be vectorized together with their neighbors 
// in the Group using wide loads/stores and shuffles. This function 
// returns the minimum cost between these two options (i.e. the absolute
// cost of the best way to vectorize this Group).
int64_t OptVLSInterface::getGroupCost(const OVLSGroup& Group,
                                      const OVLSCostModelAnalysis& CM) {
  int64_t Cost = 0; 

  // 1. Obtain the cost of vectorizing this group using wide loads/stores 
  // + shuffle-sequence
  OVLSInstructionVector InstVector;
  if (getSequence(Group, InstVector)) {
    for (OVLSInstruction *I : InstVector) {
      OVLSdbgs() << *I;
      int64_t C = CM.getInstructionCost(I);
      //OVLSdbgs() << "Cost = " << C << "\n";
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
    //OVLSdbgs() << "Cost = " << C << "\n";
    GatherScatterCost += C;
  }
  //OVLSdbgs() << "Shuffle Cost = " << Cost << "\n";
  //OVLSdbgs() << "Gather/Scatter Cost = " << GatherScatterCost << "\n";

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

