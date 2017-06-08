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
//===----------------------------------------------------------------------===//
//
// 
// Given the outlined offload block code (in a form of IR) below:
// |  void omp_outlined(int *a, int *b, int glob_up_i, ...) {
// |    int tid = get_tid();
// |    int lo_i = 0;
// |    int up_i = glob_up_i;
// |
// |    __kmpc_static_init(tid, &lo_i, &up_i);
// |
// |    for (int i = lo_i; i < up_i; i++) {
// |      for (int j = 0; j < up_j; j++) {
// |        for (int k = 0; k < up_k; k++) {
// |          a[a1i*i + a1j*j + a1k*k + a1_];
// |          a[a2i*i + a2j*j + a2k*k + a2_];
// |          b[b1i*i + b1j*j + b1k*k + b1_];
// |          b[b2i*i + b2j*j + b2k*k + b2_];
// |        }
// |      }
// |      for (int l = 0; l < up_l; l++) {
// |        a[a3i*i + a3l*l + a3_];
// |        a[a4i*i + a4l*l + a4_];
// |        b[b3i*i + b3l*l + b3_];
// |        b[b4i*i + b4l*l + b4_];
// |      }
// |    }
// |  }
// |
// |  void omp_offloading(int *a, int *b, ...) {
// |    __kmpc_fork_call(omp_outlined, a, b, up_i, ...);
// |  }
// 
// the compiler will transform it into
// 
// |  void omp_outlined(int *a, int *b, PartitionData1D *block, ...)
// |  {
// |    int tid = get_tid();
// |    int lo_i = block->lo_ind;
// |    int up_i = block->up_ind;
// |
// |    __kmpc_static_init(tid, &lo_i, &up_i);
// |
// |    int a_off = block->arrIndOffsets[0];
// |    int b_off = block->arrIndOffsets[1];
// |
// |    for (int i = lo_i; i < up_i; i++) {
// |      for (int j = 0; j < up_j; j++) {
// |        for (int k = 0; k < up_k; k++) {
// |          a[a1i*i + a1j*j + a1k*k + a1_ - a_off];
// |          a[a2i*i + a2j*j + a2k*k + a2_ - a_off];
// |          b[b1i*i + b1j*j + b1k*k + b1_ - b_off];
// |          b[b2i*i + b2j*j + b2k*k + b2_ - b_off];
// |        }
// |      }
// |      for (int l = 0; l < up_l; l++) {
// |        a[a3i*i + a3k*l + a3_ - a_off];
// |        a[a4i*i + a4k*l + a4_ - a_off];
// |        b[b3i*i + b3k*l + b3_ - b_off];
// |        b[b4i*i + b4k*l + b4_ - b_off];
// |      }
// |    }
// |  }
// |
// |  void omp_offloading(int *a, int *b, ...) {
// |    int lb1[] = { 0,    0,    0    };
// |    int ub1[] = { up_i, up_j, up_k };
// |    int lb2[] = { 0,    0          };
// |    int ub2[] = { up_i, up_l       };
// |    IterSpace ispace1(1, 2, lb1, ub1);
// |    IterSpace ispace2(1, 1, lb2, ub2);
// |    ArrayIndFunc a1({ a1i, a1j, a1k, a1_ }, &ispace1);
// |    ArrayIndFunc a2({ a2i, a2j, a2k, a2_ }, &ispace1);
// |    ArrayIndFunc b1({ b1i, b1j, b1k, b1_ }, &ispace1);
// |    ArrayIndFunc b2({ b2i, b2j, b2k, b2_ }, &ispace1);
// |    ArrayIndFunc a3({ a3i, a3l, a3_ }, &ispace2);
// |    ArrayIndFunc a4({ a4i, a4l, a4_ }, &ispace2);
// |    ArrayIndFunc b3({ b3i, b3l, b3_ }, &ispace2);
// |    ArrayIndFunc b4({ b4i, b4l, b4_ }, &ispace2);
// |
// |    const int el_size = sizeof(int);
// |    const int loc_arr_align = 8; /*TODO(nios) what ABI says?*/
// |
// |    ArrayAccess acc_a(4, { &a1, &a2, &a3, &a4 }, el_size, loc_arr_align);
// |    ArrayAccess acc_b(4, { &b1, &b2, &b3, &b4 }, el_size, loc_arr_align);
// |
// |    // TODO(nios) for now allocate only in l2
// |    ArrayAccess accs[] = { &acc_a, &acc_b };
// |    const int num_arrs = sizeof(accs) / sizeof(accs[0]);
// |    ParLoopNestDataAccess l2data(num_arrs, accs);
// |    size_t *offsets = (size_t)alloca(num_arrs * sizeof(size_t));
// |    PartitionData1D block(offsets);
// |    size_t l2_mem = nios_get_free_l2_mem_size();
// |
// |    data_block_ParLoopNestDataAccess_partition(&l2data, l2_mem);
// |
// |    size_t a_ws = acc_a.wset.extent;
// |    size_t b_ws = acc_b.wset.extent;
// |    int *loc_a = (int*)alloca_l2(a_ws, loc_arr_align);
// |    int *loc_b = (int*)alloca_l2(b_ws, loc_arr_align);
// |    int **big_arrs = {a,     b    };
// |    int **loc_arrs = {loc_a, loc_b};
// |    data_block_ParLoopNestDataAccess_setArrayMap(&l2data, big_arrs, loc_arrs);
// |
// |    for (int i1 = 0; i1 < n_chunks; i1++) {
// |      data_block_ParLoopNestDataAccess_setupBlockIterationAndCopyIn(
// |        &l2data, i1, &block);
// |      __kmpc_fork_call(omp_outlined, loc_a, loc_b, &block, ...);
// |      data_block_ParLoopNestDataAccess_copyOut(&l2data, i1, block);
// |    }
// |  }
// |
//
// Limitations of current implementation:
// - the number of parallel loops is 1 (1-dimensional partitioning only)
// - for each array, the coefficient at the parallel array index shouls be the
//     same over all accesses to that array
// - only single memory level hierarchy is supported (either L2 or L3)

//===----------------------------------------------------------------------===//

#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include <stddef.h> // size_t
#include <limits.h> // INT_MIN/MAX

#ifndef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif

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
#endif
};

// Loop nest iteration space
struct IterSpace {
  int nPar; // number of parallel loops in the nest (outermost)
  int nSeq; // number of "sequential" loops (executed entirely by 1 thread)
  // The two vectors below have nPar+nSeq components,
  // component 0 corresponds to the outermost loop
  IntVector lb; // upper bounds
  IntVector ub; // lower bounds

#ifdef __cplusplus
  IterSpace(int n_par, int n_seq, int* _lb, int *_ub) :
    nPar(n_par), nSeq(n_seq), lb(_lb), ub(_ub)
  {}
#endif
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
#endif
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
  int getOffset() {
    return coeffs.comps[ispace->nPar + ispace->nSeq];
  }

  // Calculates two points in the sequential part of the iteration space
  // corresponding to minimal and maximal array index function value
  void getSeqMinMaxPoints(IntVector minp, IntVector maxp);

  // Calculates the value of the function on given vector slice
  size_t calc(IntVector ind, int start, int len) {
    return coeffs.dotProduct(ind, start, len);
  }

  size_t calcSeqPart(IntVector ind) {
    return calc(ind, ispace->nPar, ispace->nSeq) + getOffset();
  }

  size_t calcParPart(IntVector ind) {
    return calc(ind, 0, ispace->nPar);
  }
#endif
};

// Array access one-dimensional workset characteristics
struct Workset1D {
  // 1D for now
  int extent;  // the extent of the workset
  int offset;  // the value of the seq part of ind func over minimal point
  int overlap; // overlap between two adjacent parallel iterations
  int align;   // how the local array holding the workset must be aligned

#ifdef __cplusplus
  Workset1D(int _extent, int _offset, int _overlap, int _align) :
    extent(_extent), offset(_offset), overlap(_overlap), align(_align)
  {}
#endif
};

// An aggregate of all accesses to a single array within a parallel loop
struct ArrayAccess {
  int n;                   // the number of accesses
  ArrayIndFunc** indFuncs; // index function for each
  int elemSize;            // array element size
  Workset1D wset;          // workset of all accesses, valid after calcWorkset()

#ifdef __cplusplus
  ArrayAccess(int _n, ArrayIndFunc** ind_funcs, int el_size, int ws_align) :
    n(_n), indFuncs(ind_funcs), elemSize(el_size), wset(0, 0, 0, ws_align)
  {}

  void calcWorkset();

  // Returns the coefficient at the parallel loop index in the index function.
  int getSingleCommonParLoopIndexCoeff();

  size_t getWorksetOverlap()   { return wset.overlap; }
  size_t getWorksetSize()      { return wset.extent;  }
  size_t getWorksetOffset()    { return wset.offset;  }
  size_t getWorksetAlignment() { return wset.align;   }

  // In current implementation the coefficient at the parallel loop index must
  // be the same for all accesses to this array. This function asserts that.
  void checkParCoeffs();
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
  void setArrayMap(void** big_arrs, void** loc_arrs) {
    bigArrs = big_arrs;
    locArrs = loc_arrs;
  }
#endif
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

#endif // DATA_BLOCK_H
