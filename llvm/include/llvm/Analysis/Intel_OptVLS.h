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

// Currently, maximum group size can be 64 bytes.
#define MAX_GROUP_SIZE 64
#define BYTE_SIZE 8

// OptVLS Abstract Types
typedef class OVLSMemref OVLSMemref;
typedef OVLSVector<OVLSMemref*> OVLSMemrefVector;

typedef class OVLSGroup OVLSGroup;
typedef OVLSVector<OVLSGroup> OVLSGroupVector;

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

  bool isStridedAccess() const { return AccType == SLoad || AccType == SStore; }

  bool isIndexedAccess() const { return AccType == ILoad || AccType == IStore; }

};

class OVLSMemref {
public:
  explicit OVLSMemref(unsigned ElementSize,
                      unsigned NumElements, const OVLSAccessType& AccType);

  unsigned getNumElements() const { return NumElements; }
  unsigned getElementSize() const { return ElementSize; }
  OVLSAccessType getAccessType() const { return AccType; }

  unsigned getId() const { return Id; }

  void print(OVLSostream &OS, unsigned SpaceCount) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  /// This method is used for debugging.
  ///
  void dump() const;
#endif

  // Returns true if this->Memref and Memref are (neighbors) a constant
  // distance apart (for each ith element), distance is computed in bytes.
  // Otherwise, returns false;
  virtual bool isAConstDistanceFrom(const OVLSMemref& Memref, int *Dist) = 0;

  // Returns true if this can move to the location of Memref. This means it
  // does not violate any program/control flow semantics nor any memory dependencies.
  // I.e., this is still alive at the location of Memref and there are no
  // loads/stores of this in between the location of this and the location of
  // Memref.
  virtual bool canMoveTo(OVLSMemref *Memref) = 0;

private:
  unsigned Id;          // A unique Id, helps debugging.
  unsigned ElementSize; // in bits
  unsigned NumElements;
  OVLSAccessType AccType; // Access type of the Memref, e.g {S|I}{Load|store}
};

class OVLSGroup {
public:
  explicit OVLSGroup(int GSize, const OVLSAccessType& AType)
    : GrpSize(GSize), AccType(AType) {
    ByteAccessMask = 0;
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
  unsigned getAccessMask() const { return ByteAccessMask; }
  void setAccessMask(uint64_t Mask) { ByteAccessMask = Mask; }
  OVLSAccessType getAccessType() const { return AccType; }
  unsigned getGroupSize() const { return GrpSize; }

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
  unsigned GrpSize;        // Group size in bytes, maximum size can be 64.
                           // Group size can be the maximum length of the
                           // underlying vector register or any other desired
                           // size that clients want to consider.
                           // The default size is 64.
  uint64_t ByteAccessMask; // ByteAccessMask is a byte mask, represents the
                           // group byte access pattern.
                           // Each bit in the mask corresponds to a byte.
                           // Specifically, it shows how many bytes of the
                           // group size are currently occupied and which ones
                           // are occupied.
                           // Maximum 64 bytes can be represented.
  OVLSAccessType AccType;  // AccessType of the group.
};

// OptVLS public Interface class that operates on OptVLS Abstract types.
class OptVLSInterface {
public:
  // getGroups() takes a vector of OVLSMemrefs and a group size in bytes
  // (which is the the maximum length of the underlying vector register
  // or any other desired size that clients want to consider, maximum size
  // can be 64), and returs a vector of OVLSGroups. Each group contains one or more
  // OVLSMemrefs, and each OVLSMemref is contained by 1 (and only 1) OVLSGroup
  // such that being together these memrefs in a group do not violate any program
  // semantics or memory dependencies.
  static OVLSGroupVector& getGroups(const OVLSMemrefVector &Memrefs,
                                    unsigned GroupSize);
};

}
#endif
