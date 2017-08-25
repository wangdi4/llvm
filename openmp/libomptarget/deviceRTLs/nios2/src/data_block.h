//===------ data_block.h - Automatic data partitioning --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
/// \file Interface of a runtime library used by the compiler to implement
///  automatic data partitioning and loop blocking.
///  See comments in llvm/lib/Target/Nios2/LoopAndDataBlockPass.cpp
//===----------------------------------------------------------------------===//

#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#if (defined(i386) || defined(_M_IX86) || \
     defined(__x86_64__) || defined(_M_X64))
#define X86_TARGET
#endif

#include <stddef.h> // size_t
#ifdef __cplusplus
#include <new> // for placement new
#endif // __cplusplus

#if defined(_WIN32) || defined(WIN32)
  #ifdef LIBDATABLOCK_EXPORTS
    #define DLL_INTERFACE __declspec(dllexport)
  #else
    #define DLL_INTERFACE __declspec(dllimport)
  #endif // LIBDATABLOCK_EXPORTS
#else
  #define DLL_INTERFACE
#endif // defined(_WIN32) || defined(WIN32)

#ifdef __cplusplus
  #define EXTERN_C extern "C"
#else
  #define EXTERN_C
#endif // __cplusplus

#define EXPORT EXTERN_C DLL_INTERFACE

// debug print is enabled only on X86/X64 in debug builds
#if defined(__cplusplus) && (defined(_DEBUG) || defined(DEBUG))
#define DATABLOCK_DEBUG
#endif

#define DATABLOCK_MAX_PAR_DEPTH 1
#define DATABLOCK_MAX_SEQ_DEPTH 8
#define DATABLOCK_MAX_NEST_DEPTH (DATABLOCK_MAX_PAR_DEPTH+DATABLOCK_MAX_SEQ_DEPTH)

//-----------------------------------------------------------------------------
// Data type declatations
//-----------------------------------------------------------------------------

// Represents a simple vector of integers of arbitrary size defined externally
struct IntVector {
  int *comps;

#ifdef __cplusplus
  IntVector(int *mem) : comps(mem) {}

  // Dot product operation over given slice of vector components
  int dotProduct(IntVector v, int start, int len);

  // Subtraction operation over given slice of vector components
  void sub(IntVector v, int start, int len);

#ifdef DATABLOCK_DEBUG
  void print(int n, int tab);
#endif // DATABLOCK_DEBUG
#endif // __cplusplus
};

// Loop nest iteration space
struct IterSpace {
  int nPar; // number of parallel loops in the nest (outermost)
  int nSeq; // number of "sequential" loops (executed entirely by 1 thread)
  // The two vectors below have nPar+nSeq components,
  // component 0 corresponds to the outermost loop
  IntVector lb; // upper bounds
  IntVector ub; // lower bounds
  IntVector st; // strides

#ifdef __cplusplus
  IterSpace(int n_par, int n_seq, int* _lb, int* _ub, int* _st) :
    nPar(n_par), nSeq(n_seq), lb(_lb), ub(_ub), st(_st)
  {}

  int getDepth() {
    return nPar + nSeq;
  }

#ifdef DATABLOCK_DEBUG
  void print(int tab);
#endif // DATABLOCK_DEBUG
#endif // __cplusplus
};

// One-dimensional data and loop partition parameters for current chunk of
// iterations.
struct PartitionData1D {
  int parIndLo; // current lower bound of the parallel loop
  int parIndUp; // current upper bound of the parallel loop
  size_t *arrIndOffsets; // the offset added to index expressions of each array

#ifdef __cplusplus
  PartitionData1D(size_t *offs) :
    parIndLo(0), parIndUp(0), arrIndOffsets(offs)
  {}

  int getNumParIters() { return parIndUp - parIndLo + 1; }

#ifdef DATABLOCK_DEBUG
  void print(int n_arrs, int tab);
#endif // DATABLOCK_DEBUG
#endif // __cplusplus
};

// Linear array index function depending on indices of a loop nest.
struct ArrayIndFunc {
  IntVector coeffs;  // coefficients in the linear expression
  IterSpace *ispace; // iteration space of the loop nest

#ifdef __cplusplus
  ArrayIndFunc(int* _coeffs, IterSpace* _ispace) :
    coeffs(_coeffs), ispace(_ispace)
  {}

  // Returns the offset member in the linear expression
  int getAddend() {
    return coeffs.comps[ispace->getDepth()];
  }

  // Selects two points in the iteration sub-space corresponding to minimal
  // and maximal array index function value with all other indices (beyond the
  // sub-space) being constant. start and len define the sub-space.
  void getIspaceMinMaxPoints(
    IntVector minp, IntVector maxp, int start, int len);

  // Does the same as the getIspaceMinMaxPoints function, but subspace points
  // are taken from given vectors. Effectively, swaps corresponding components
  // between minp and maxp where needed.
  void adjIspaceMinMaxPoints(
    IntVector minp, IntVector maxp, int start, int len);

  // Calculates the value of the function on given vector slice
  size_t calc(IntVector ind, int start, int len) {
    return coeffs.dotProduct(ind, start, len);
  }

  size_t calc(IntVector ind) {
    return coeffs.dotProduct(ind, 0, ispace->getDepth()) + getAddend();
  }

  int getDepth() {
    return ispace->getDepth();
  }

#ifdef DATABLOCK_DEBUG
  void print(int tab);
#endif // DATABLOCK_DEBUG
#endif // __cplusplus
};

// Array access one-dimensional workset characteristics
struct Workset1D {
  // 1D for now
  size_t extent1;   // the extent of the workset (single parallel iteration)
  size_t extentN;   // the extent of the workset (n parallel iterations)
  int nParIters; // the number of parellel iterations extentN is calculated for
  int offset;    // minimal value of all array's index functions
  int overlap;   // overlap between two adjacent parallel iterations
  int align;     // how the local array holding the workset must be aligned

#ifdef __cplusplus
  Workset1D(int _extent, int _offset, int _overlap, int _align) :
    extent1(_extent), extentN(_extent), nParIters(1), offset(_offset),
    overlap(_overlap), align(_align)
  {}

  Workset1D() {
    new (this) Workset1D(0, 0, 0, 0);
  }

#ifdef DATABLOCK_DEBUG
  void print(int tab);
#endif // DATABLOCK_DEBUG
#endif // __cplusplus
};

// An aggregate of all accesses to a single array within a parallel loop
struct ArrayAccess {
  int n;                   // the number of accesses
  ArrayIndFunc** indFuncs; // index function for each
  int elemSize;            // array element size
  Workset1D wset;          // workset of all accesses, valid after calcSingleParIterWorkset()

#ifdef __cplusplus
  ArrayAccess(int _n, ArrayIndFunc** ind_funcs, int el_size, int ws_align) :
    n(_n), indFuncs(ind_funcs), elemSize(el_size), wset(0, 0, 0, ws_align)
  {}

  void calcSingleParIterWorkset();
  void updateWorkset(int n_par_iters);

  // Returns the coefficient at the parallel loop index in the index function.
  int getSingleCommonParLoopIndexCoeff();

  size_t getWorksetOverlap()              { return wset.overlap; }
  size_t getWorksetSize()                 { return wset.extent1; }
  size_t getWorksetSize(int n_par_iters);
  size_t getWorksetOffset()               { return wset.offset;  }
  size_t getWorksetAlignment()            { return wset.align;   }

  // In current implementation the coefficient at the parallel loop index must
  // be the same for all accesses to this array. This function asserts that.
  void checkParCoeffs();

#ifdef DATABLOCK_DEBUG
  void print(int tab);
#endif // DATABLOCK_DEBUG
#endif
};

// An aggregate of all array accesses to all arrays within a parallel loop nest.
struct ParLoopNestDataAccess {
  int n;              // The number of arrays aggregated here
  ArrayAccess** accs; // data for each array
  void** bigArrs;     // the big<->local array map,
  void** locArrs;     //   valid after setArrayMap
  int iChunk;         // the max par i-space chunk with data fitting into memory
  int nChunks;        // the number of such chunks

#ifdef __cplusplus
  ParLoopNestDataAccess(int _n, ArrayAccess** _accs) :
    n(_n), accs(_accs), bigArrs((void**)0), locArrs((void**)0),
    iChunk(0), nChunks(0)
    {}

  int getNumArrays() { return n; }

  int getParLoopUpperBound(int loop_num);
  int getParLoopLowerBound(int loop_num);

  ArrayIndFunc* getArrayIndFunc(int arr_id, int acc_id) {
    return accs[arr_id]->indFuncs[acc_id];
  }

  // The main function which finds the chunk size and the number of chunks
  // to split the parallel iteration space into
  void partition(size_t mem_size);

  // Calculates lower/upper bounds for current par i-space chunk (writes it into
  // the block parameter) and copies parts of the big arrays into their local
  // windows. setArrayMap(...) must have been called beforehands.
  void setupBlockIterationAndCopyIn(int iter_num, PartitionData1D *block);
  void copyImpl(PartitionData1D *block, bool in);

  void copyIn(PartitionData1D *block)  { copyImpl(block, true);  }
  void copyOut(PartitionData1D *block) { copyImpl(block, false); }

  // Tells the actual addresses of the partitioned arrays and their
  // corresponding local windows and establishes a mapping between them.
  // big_arr[i] partitioned array correponds to loc_arrs[i] local window.
  void setArrayMap(void** big_arrs, void** loc_arrs);

#ifdef DATABLOCK_DEBUG
  void print(int tab);
#endif // DATABLOCK_DEBUG
#endif // __cplusplus
};

//-----------------------------------------------------------------------------
// C interface functions called by compiler-generated code
//-----------------------------------------------------------------------------

// C interface for ParLoopNestDataAccess::partition
EXPORT void data_block_ParLoopNestDataAccess_partition(
  struct ParLoopNestDataAccess* obj, size_t mem_size);

// C interface for ParLoopNestDataAccess::setArrayMap
EXPORT void data_block_ParLoopNestDataAccess_setArrayMap(
  struct ParLoopNestDataAccess* obj, void** big, void **loc);

// C interface for ParLoopNestDataAccess::setupBlockIterationAndCopyIn
EXPORT void data_block_ParLoopNestDataAccess_setupBlockIterationAndCopyIn(
  struct ParLoopNestDataAccess* obj, int iter, PartitionData1D *block);

// C interface for ParLoopNestDataAccess::copyOut
EXPORT void data_block_ParLoopNestDataAccess_copyOut(
  struct ParLoopNestDataAccess* obj, PartitionData1D *block);

EXPORT unsigned char* data_block_device_alloc_l2(size_t n, int align);

#endif // DATA_BLOCK_H
