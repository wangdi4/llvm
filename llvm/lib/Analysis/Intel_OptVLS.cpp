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

OVLSMemref::OVLSMemref(unsigned ESize, unsigned NumElements, const OVLSAccessType& AType)
  : ElementSize(ESize), NumElements(NumElements), AccType(AType) {
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
  OS << " <" << getNumElements() << " x " << getElementSize()
            << ">";

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

void OVLSGroup::print(OVLSostream &OS, unsigned NumSpaces) const {

  OS << "\n    Group Size(in bytes): " << getGroupSize();
  // print accessType
  OS << "\n    AccType: ";
  getAccessType().print(OS);

  // Print result mask
  OS << "\n    AccessMask(per byte, R to L): ";
  uint64_t AMask = getAccessMask();

  // Convert int AccessMask to binary
  char SRMask[MAX_GROUP_SIZE+1], *MaskPtr;
  SRMask[0] = '\0';
  MaskPtr = &SRMask[1];
  while (AMask) {
    if (AMask & 1)
      *MaskPtr++ = '1';
    else
      *MaskPtr++ = '0';

    AMask >>= 1;
  }
  // print the mask reverse
  while (*(--MaskPtr) != '\0') {
    OS << *MaskPtr;
  }
  OS << "\n";

  // Print vector of memrefs that belong to this group.
  for (unsigned i = 0, S = MemrefVec.size(); i < S; i++) {
    MemrefVec[i]->print(OS, NumSpaces);
    OS << "\n";
  }
}

namespace OptVLS {
  static void dumpOVLSGroupVector(OVLSostream &OS, const OVLSGroupVector &Grps) {
    OS << "\n  Printing Groups- Total Groups " << Grps.size() << "\n";
    unsigned GroupId = 1;
    for (unsigned i = 0, S = Grps.size(); i < S; i++) {
      OS << "  Group#" << GroupId++ << " ";
      Grps[i].print(OS, 3);
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
    assert(bitlocation + shiftcount <= MAX_GROUP_SIZE &&
          "Invalid bit location for a bytemask");

    uint64_t NewMask = (shiftcount == 64) ? ~0LL : (1LL << shiftcount) - 1;
    Mask |= NewMask << bitlocation;

    return Mask;
  }

  // Form OptVLSgroups for each set of adjacent memrefs in the MemrefSetVec
  // where memrefs in the OptVLSgroup being together do not violate any program
  // semantics nor any memory dependencies. Also, make sure the total size of
  // the memrefs does not exceed the group size.
  static OVLSGroupVector& formGroups(const MemrefDistanceMapVector &AdjMrfSetVec,
                                     unsigned GrpSize) {
    OVLSGroupVector *OVLSGrps = new OVLSGroupVector();

    for (MemrefDistanceMap *AdjMemrefSet : AdjMrfSetVec) {
      assert (!AdjMemrefSet->empty() && "Adjacent memref-set cannot be empty");
      MemrefDistanceMapIt AdjMemrefSetIt = (*AdjMemrefSet).begin();

      OVLSAccessType AccType = (AdjMemrefSetIt->second)->getAccessType();
      OVLSGroup *CurrGrp = new OVLSGroup(GrpSize, AccType);
      int GrpFirstMDist = AdjMemrefSetIt->first;

      // Group memrefs in each set using a greedy approach, keep inserting the
      // memrefs into the same group until the group is full. Form a new group
      // for a memref that cannot be moved into the group.
      for (MemrefDistanceMapIt E = (*AdjMemrefSet).end();
                                     AdjMemrefSetIt != E; ++AdjMemrefSetIt) {
        OVLSMemref *Memref = AdjMemrefSetIt->second;
        unsigned ElemSize = Memref->getElementSize() / BYTE_SIZE; // in bytes
        int Dist = AdjMemrefSetIt->first;

        uint64_t AccMask = CurrGrp->getAccessMask();

        if ((Dist - GrpFirstMDist + ElemSize) > GrpSize || // capacity exceeded.
            !Memref->canMoveTo(CurrGrp->getFirstMemref())) {
          OVLSGrps->push_back(*CurrGrp);
          CurrGrp = new OVLSGroup(GrpSize, AccType);

          // Reset GrpFirstMDist
          GrpFirstMDist = Dist;
          // Generate AccessMask
          AccMask = OptVLS::genMask(0, ElemSize, Dist - GrpFirstMDist);
        } else
          AccMask = OptVLS::genMask(AccMask, ElemSize, Dist - GrpFirstMDist);

        CurrGrp->insert(Memref);
        // Set Access Mask.
        CurrGrp->setAccessMask(AccMask);
      }
      OVLSGrps->push_back(*CurrGrp);
    }

    // dump OVLSGroups
    OVLSDebug(OptVLS::dumpOVLSGroupVector(OVLSdbgs(), *OVLSGrps));
    return *OVLSGrps;
  }

  // Split the memref-vector into groups where memrefs in each group
  // are neighbors(adjacent), means they
  //   1) have the same access type
  //   2) are a constant distance apart
  //
  // Adjacent memrefs in the group are sorted based on their distance from the first
  // memref in the group in an ascending order.
  static MemrefDistanceMapVector* splitMrfs(const OVLSMemrefVector &Memrefs) {
    OVLSDebug(OVLSdbgs() << "\n  Split the vector memrefs into sub groups of "
                                 "adjacacent memrefs: \n");
    OVLSDebug(OVLSdbgs() << "    Distance is (in bytes) from the first memref of"
                                " the set\n");

    MemrefDistanceMapVector *AdjMemrefSetVec = new MemrefDistanceMapVector();
    for (unsigned i = 0, Size = Memrefs.size(); i < Size; ++i) {
      OVLSMemref *Memref = Memrefs[i];

      int Dist = 0;
      bool AdjMrfSetFound = false;

      // TODO: Currently finding the appropriate group is done using a linear search.
      // It would be better to use a hashing algorithm for this search.
      for (MemrefDistanceMap *AdjMemrefSet : *AdjMemrefSetVec) {

        MemrefDistanceMapIt I = (*AdjMemrefSet).begin();
        OVLSMemref *SetFirstMrf = I->second;

        if (Memref->getAccessType() == SetFirstMrf->getAccessType() && // same access type
            Memref->isAConstDistanceFrom(*SetFirstMrf, &Dist)) { // are a const distance apart
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
        AdjMemrefSetVec->push_back(AdjMemrefSet);
      }
    }

    // dump sorted set
    OVLSDebug(OptVLS::dumpMemrefDistanceMapVector(OVLSdbgs(), *AdjMemrefSetVec));
    return AdjMemrefSetVec;
  }
} // end of namespace

// getGroups() takes a vector of OVLSMemrefs and a group size in bytes
// (which is the the maximum length of the underlying vector register
// or any other desired size that clients want to consider, maximum size
// can be 64), and returs a vector of OVLSGroups. Each group contains one or more
// OVLSMemrefs, (and each OVLSMemref is contained by 1 (and only 1) OVLSGroup)
// in a way where having all the memrefs in OptVLSgroup (at one single point in
// the program, the location of first memref in the group)does not violate
// any program semantics nor any memory dependencies.
OVLSGroupVector& OptVLSInterface::getGroups(const OVLSMemrefVector &Memrefs,
                                            unsigned GrpSize) {
  OVLSGroupVector *Grps =  new OVLSGroupVector();

  OVLSDebug(OVLSdbgs() << "Received a request from Client---FORM GROUPS\n");

  if (Memrefs.empty()) return *Grps;
  if (GrpSize > MAX_GROUP_SIZE) {
    OVLSDebug(OVLSdbgs() << "!!!Group size above " << MAX_GROUP_SIZE <<
                             " bytes is not supported currently\n");
    return *Grps;
  }

  OVLSDebug(OVLSdbgs() << "  Recieved a vector of memrefs: \n");
  OVLSDebug(OptVLS::dumpOVLSMemrefVector(OVLSdbgs(), Memrefs, 2));

  // Split the vector of memrefs into sub groups where memrefs in each sub group
  // are neighbors, means they
  //   1) have the same access type
  //   2) are a constant distance apart
  MemrefDistanceMapVector *AdjMemrefSetVec = OptVLS::splitMrfs(Memrefs);

  // Form OptVLSgroups for each sub group where having all the memrefs in
  // OptVLSgroup (at one single point in the program, the location of first
  // memref in the group)does not violate any program semantics nor any memory
  // dependencies.
  // Also, make sure the total size of the memrefs does not exceed the group size.
  *Grps = OptVLS::formGroups(*AdjMemrefSetVec, GrpSize);

  // Release memory
  for (MemrefDistanceMap *AdjMemrefSet : *AdjMemrefSetVec) {
    delete AdjMemrefSet;
  }
  delete AdjMemrefSetVec;

  return *Grps;
}
