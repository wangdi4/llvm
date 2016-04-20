//===- OptVLS.h - Optimization of Vector Loads/Stores ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===---------------------------------------------------------------------===//
///
/// \file
/// OptVLS performs two optimizations:
/// 1. Replaces a set of complex loads/stores(indexed, strided) by a set of
///    simple loads/stores(contiguous) followed by shuffle/permute
/// 2. Replaces a set of overlapping accesses by a set of fewer loads/stores
///    followed by shuffle/permute.
///
/// OptVLS is IR agnostic. It provides abstract types to communicate with its
/// clients. Various clients of OptVLS are Intel loop optimizer, Intel
/// vectorizer, OptVLSPass, etc which have their own IR such as HIR, AL,
/// LLVM IR respectively.
/// This file contains the declarations of OptVLS abstract types and the
/// interface classes that operate on and return these OptVLS abstract types.
/// The interface classes expose the core functionalities of OptVLS such as
/// grouping, cost-analysis, sequence generation, etc which are used as the
/// common interfaces for all the clients of OptVLS.
///
/// Clients must implement the various virtual methods which provide the
/// necessary information of client APIs to OptVLS.
///
//===---------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_OPTVLS_H
#define LLVM_ANALYSIS_INTEL_OPTVLS_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include <map>

namespace llvm {

// OptVLS data structures
template <typename T>
class OVLSVector : public SmallVector<T, 8> {};

template <typename KeyT, typename ValueT>
class OVLSMap : public std::multimap<KeyT, ValueT> {};

// For printing under debug.
typedef class raw_ostream OVLSostream;
#define OVLSdbgs() dbgs()

#define OVLSDebug(x) DEBUG(x)

// Current maximum supported vector length is 64 bytes (512 bits).
#define MAX_VECTOR_LENGTH 64
#define BYTE 8

// OptVLS Abstract Types
typedef class OVLSMemref OVLSMemref;
typedef OVLSVector<OVLSMemref*> OVLSMemrefVector;

typedef class OVLSGroup OVLSGroup;
typedef OVLSVector<OVLSGroup*> OVLSGroupVector;

typedef class OVLSInstruction OVLSInstruction;

// AccessType: {Strided|Indexed}{Load|Store}
class OVLSAccessType {
private:
  // S:Strided I:Indexed
  enum ATypeE { Unknown, SLoad, SStore, ILoad, IStore };

  ATypeE AccType;
public:
  explicit OVLSAccessType(ATypeE AccType) {
    this->AccType = AccType;
  }
  bool operator==(ATypeE AccType) const {
    return this->AccType == AccType;
  }
  bool operator==(const OVLSAccessType& Rhs) const {
    return AccType == Rhs.AccType;
  }
  bool operator!=(ATypeE AccType) const {
    return this->AccType != AccType;
  }
  bool isUnknown() const {
    if (AccType > IStore || AccType < SLoad) return true;
    return false;
  }

  void print(OVLSostream &OS) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// This method is used for debugging.
  ///
  void dump() const;
#endif

  static OVLSAccessType getStridedLoadTy() {
    return OVLSAccessType(SLoad);
  }
  static OVLSAccessType getStridedStoreTy() {
    return OVLSAccessType(SStore);
  }
  static OVLSAccessType getIndexedLoadTy() {
    return OVLSAccessType(ILoad);
  }
  static OVLSAccessType getIndexedStoreTy() {
    return OVLSAccessType(IStore);
  }
  static OVLSAccessType getUnknownTy() {
    return OVLSAccessType(Unknown);
  }

  bool isStridedAccess() const { return AccType == SLoad ||
                                        AccType == SStore; }
  bool isStridedLoad()   const { return AccType == SLoad; }

  bool isIndexedAccess() const { return AccType == ILoad ||
                                        AccType == IStore; }

};

class OVLSMemref {
public:
  /// Discriminator for LLVM-style RTTI (dyn_cast<> et al.)
  enum OVLSMemrefKind {
    VLSK_ClientMemref,
    VLSK_HIRVLSClientMemref
  };
private:
  const OVLSMemrefKind Kind;
public:
  OVLSMemrefKind getKind() const { return Kind; }

  explicit OVLSMemref(OVLSMemrefKind K, unsigned ElementSize,
                      unsigned NumElements, const OVLSAccessType& AccType);

  virtual ~OVLSMemref() {}

  unsigned getNumElements() const { return NumElements; }
  void setNumElements(unsigned nelems) { NumElements = nelems; }
  unsigned getElementSize() const { return ElementSize; }
  OVLSAccessType getAccessType() const { return AccType; }
  void setAccessType(const OVLSAccessType& Type) { AccType = Type; }

  unsigned getId() const { return Id; }

  void print(OVLSostream &OS, unsigned SpaceCount) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// This method is used for debugging.
  ///
  void dump() const;
#endif

  /// \brief If the references are scalar, returns true if this and the Memref
  /// are a constant distance apart. If the memrefs are vectors returns true if
  /// all ith elements of this and the ith elements of the \p Memref are a
  /// constant distance apart. Otherwise, returns false. When true is returned
  /// the constant distance is returned in \p Dist in terms of bytes, otherwise
  /// \p Dist is undefined.
  ///
  /// Please note that, VLS requires the distance that LLVM-IR maintains between
  /// the memrefs. Therefore, this distance computation in the client should
  /// follow the LLVM/vectorizer standard address computation formula for the
  /// memrefs.
  ///
  /// This restriction can be waved in the future if it shows that optimizing
  /// memrefs with non-uniform distances between the ith elements is profitable.
  ///
  /// An example of non-uniform distances between the ith elements:
  /// int32_t a[n];
  /// for (i = 0, n)
  ///      = a[3i+1] {stride: j(12)-bytes} accessing every jth byte
  ///      = a[3i+2] {stride: k(16)-bytes} accessing every kth byte
  /// This function will return false for the above two memrefs since distances
  /// between the ith elements are not uniform(distance between the 1st two
  /// elements is 4 bytes, 2nd two elements is 8 bytes). But this function will
  /// return true for the following two memrefs,
  /// for (i = 0, n)
  ///      = a[3i+1] {stride: j(12)-bytes} accessing every jth byte
  ///      = a[3i+2] {stride: j(12)-bytes} accessing every jth byte
  ///
  virtual bool isAConstDistanceFrom(const OVLSMemref& Memref,
                                    int64_t *Dist) = 0;

  // Returns true if this and Memref have the same number of elements.
  virtual bool haveSameNumElements(const OVLSMemref& Memref) = 0;

  // Returns true if this can move to the location of Memref. This means it
  // does not violate any program/control flow semantics nor any memory
  // dependencies. I.e., this is still alive at the location of Memref and
  // there are no loads/stores of this in between the location of this and the
  // location of Memref.
  virtual bool canMoveTo(const OVLSMemref& Memref) = 0;

  /// \brief Returns true if this is a strided access and it has a constant
  /// uniform distance between the elements, that constant integer distance (in
  /// bytes) is provided in \p Stride. Otherwise, returns false.
  /// Inverting the return value does not invert the functionality(false does
  /// not mean that it has a variable stride)
  virtual bool hasAConstStride(int64_t *Stride) = 0;

private:
  unsigned Id;          // A unique Id, helps debugging.
  unsigned ElementSize; // in bits
  unsigned NumElements;
  OVLSAccessType AccType; // Access type of the Memref, e.g {S|I}{Load|store}
};

class OVLSGroup {
public:
  explicit OVLSGroup(int VLen, const OVLSAccessType& AType)
    : VectorLength(VLen), AccType(AType) {
    NByteAccessMask = 0;
  }

  typedef OVLSMemrefVector::iterator iterator;
  inline iterator                begin() { return MemrefVec.begin(); }
  inline iterator                end  () { return MemrefVec.end();   }

  // Returns true if the group is empty.
  bool empty() const {
    return MemrefVec.empty();
  }
  // Inserts an element into the Group.
  void insert(OVLSMemref *Mrf) {
    MemrefVec.push_back(Mrf);
  }

  // Returns group access mask.
  uint64_t getNByteAccessMask() const { return NByteAccessMask; }
  void setAccessMask(uint64_t Mask) { NByteAccessMask = Mask; }
  OVLSAccessType getAccessType() const { return AccType; }
  uint32_t getVectorLength() const { return VectorLength; }

  bool hasStridedAccesses() const {
    return AccType.isStridedAccess();
  }
  // Return the first OVLSMemref of this group.
  OVLSMemref* getFirstMemref() const {
    if (!MemrefVec.empty()) return MemrefVec[0];
    return nullptr;
  }

  void print(OVLSostream &OS, unsigned SpaceCount) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// This method is used for debugging.
  ///
  void dump() const;
#endif

private:
  OVLSMemrefVector MemrefVec;// Group element-vector
  uint32_t VectorLength;   // Vector length in bytes, default/maximum supported
                           // length is 64. VectorLength can be the maximum
                           // length of the underlying vector register or any
                           // other desired size that clients want to consider.
  uint64_t NByteAccessMask;// NByteAccessMask is a byte mask, represents the
                           // access pattern for each N bytes comprising the
                           // i-th element of the memrefs in the MemrefVec,
                           // here N <= VectorLength.
                           // Each bit in the mask corresponds to a byte.
                           // Specifically, it tells us if there are any gaps
                           // in between the i-th accesses (since access
                           // pattern information is not recorded in the
                           // MemrefVec to save memory)
                           // Maximum 64 bytes can be represented.
  OVLSAccessType AccType;  // AccessType of the group.
};

/// OVLS server works in a target independent manner. In order to estimate
/// more accurate cost for a specific target (architecture), client needs to
/// provide the necessary target-specific information.
/// This cost-model interface class defines all the necessary
/// parameters/functions that are needed by the server to estimate more accurate
/// cost. In order to get cost, client needs to provide an object of this class
/// filled up with the necessary target-specific cost information that are
/// defined by the underlying targets. Consequently, it's the clients that
/// decide on the cost accuracy level.
class OVLSCostModelAnalysis {

public:
  /// \brief Returns target-specific cost for an OVLSInstruction, different
  /// cost parameters are defined by each specific target.
  /// Returns -1 if the cost is unknown. This function needs to be overriden by
  /// the OVLS clients to help getting the target-specific instruction cost.
  virtual uint64_t getInstructionCost(const OVLSInstruction *I) const = 0;

};

// OptVLS public Interface class that operates on OptVLS Abstract types.
class OptVLSInterface {
public:
  /// \brief getGroups() groups the memrefs that are adjacent and returns
  /// the formed groups in \p Grps.
  /// getGroups() takes a vector of OVLSMemrefs, a vector of OVLSGroups for
  /// containing the return group-vector and a vector length in bytes (which is
  /// the maximum length of the underlying vector register or any other
  /// desired size that clients want to consider, maximum size can be 64).
  /// Each group contains one or more OVLSMemrefs, and each OVLSMemref is
  /// contained by 1 (and only 1) OVLSGroup such that being
  /// together these memrefs in a group do not violate any program semantics or
  /// memory dependencies.
  /// Current grouping is done using a greedy approach; i.e. it keeps inserting
  /// adjacent memrefs into the same group until the total element size(
  /// considering a single element from each memref) is less than or equal to
  /// vector length. Currently, it only tries to form a group at the location
  /// of a memref that has a lowest distance from the base, it does not try
  /// other adjacent-memref-locations. Because of this greediness it can miss
  /// some opportunities. This can be improved in the future if needed.
  static void getGroups(const OVLSMemrefVector &Memrefs,
                        OVLSGroupVector &Grps,
                        uint32_t VectorLength);

  /// \brief getGroupCost() returns a relative cost/benefit of performing
  /// adjacent gather/scatter optimization for a group (of gathers/scatters).
  /// Adj. gather/scatter optimization replaces a set of gathers/scatters by a
  /// set of contiguous loads/stores followed by a sequence of shuffle
  /// instructions.
  static int64_t getGroupCost(const OVLSGroup& Group,
                               const OVLSCostModelAnalysis& CM);

};

}
#endif
