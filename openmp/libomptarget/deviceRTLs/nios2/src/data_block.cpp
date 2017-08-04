//= deviceRTLs/nios2/data_block.cpp - Automatic data partitioning -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Device library to support automatic data partitioning (blocking).
///
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h> // INT_MIN/MAX
#include "data_block.h"
#ifdef X86_TARGET
#include <memory.h>
#else
#include "fpga_mc_dispatcher.h"
#endif // X86_TARGET

#ifdef X86_TARGET
#define PRN_PTR "%p"
#define PRN_SIZE_T "%zu"
#else
#define PRN_PTR "%x"
#define PRN_SIZE_T "%u"
#endif // X86_TARGET

#define ALIGN_UP(val, al) ( (val+al-1) & ~(al-1) )
#define MIN(x1,x2) (x1 > x2  ? x2 : x1)
#define MAX(x1,x2) (x1 >= x2 ? x1 : x2)
#define ABS(x) ((x) > 0 ? (x) : -(x))

enum {
  ERR_NO_ERROR = 0,
  ERR_UNSPECIFIED,
  ERR_LOOP_TREE_TOO_DEEP,
  ERR_DIFF_COEFFS,
  ERR_OUT_OF_MEMORY
};

typedef unsigned char byte;

static void* memcpy_l4_to_device(void* dst, void *src, size_t n) {
#ifdef DATABLOCK_DEBUG
  printf("fpga_mc_memcpy_l4_to_device 0x" PRN_PTR " -> 0x" PRN_PTR ", " PRN_SIZE_T " bytes\n", src, dst, n);
#endif // DATABLOCK_DEBUG
#ifdef X86_TARGET
  memcpy(dst, src, n);
#else
  fpga_mc_memcpy_l4_to_device(dst, (l4_virtual_ptr_t)src, n);
#endif // X86_TARGET
  return dst;
}

static void* memcpy_device_to_l4(void* dst, void *src, size_t n) {
#ifdef DATABLOCK_DEBUG
  printf("fpga_mc_memcpy_device_to_l4 0x" PRN_PTR " -> 0x" PRN_PTR ", " PRN_SIZE_T " bytes\n", src, dst, n);
#endif // DATABLOCK_DEBUG
#ifdef X86_TARGET
  memcpy(dst, src, n);
#else
  fpga_mc_memcpy_device_to_l4((l4_virtual_ptr_t)dst, src, n);
#endif // X86_TARGET
  return dst;
}

static void abort(int err_code, const char *msg) {
#ifdef X86_TARGET
  fprintf(stderr, "### DATA BLOCKING FATAL RUNTIME ERROR: %s\n", msg);
  exit(err_code);
#else
  fpga_mc_abort(err_code, "### DATA BLOCKING FATAL RUNTIME ERROR");
#endif // X86_TARGET
}

int IntVector::dotProduct(IntVector v, int start, int len) {
  int* comps1 = v.comps;
  int res = 0;

  for (int i = start; i < start + len; i++) {
    res += comps[i] * comps1[i];
  }
  return res;
}

void IntVector::sub(IntVector v, int start, int len) {
  for (int i = start; i < start + len; i++) {
    comps[i] -= v.comps[i];
  }
}

void ArrayIndFunc::adjIspaceMinMaxPoints(
  IntVector minp, IntVector maxp, int start, int len)
{
  for (int i = start; i < start + len; i++) {
    int lb = minp.comps[i];
    int ub = maxp.comps[i];

    if ((coeffs.comps[i] > 0) != (lb <= ub)) {
      minp.comps[i] = ub;
      maxp.comps[i] = lb;
    }
  }
}

void ArrayIndFunc::getIspaceMinMaxPoints(
  IntVector minp, IntVector maxp, int start, int len)
{
  for (int i = start; i < start+len; i++) {
    int lb = ispace->lb.comps[i];
    int ub = ispace->ub.comps[i];
    bool inc_range = (coeffs.comps[i] > 0) == (lb <= ub);
    minp.comps[i] = inc_range ? lb : ub;
    maxp.comps[i] = inc_range ? ub : lb;
  }
}

void ArrayAccess::calcSingleParIterWorkset() {
  int min_arr_seq_ind = INT_MAX;
  int max_arr_seq_ind = INT_MIN;

  int minp[DATABLOCK_MAX_NEST_DEPTH];
  int maxp[DATABLOCK_MAX_NEST_DEPTH];

  // 1. loop over all index functions for current array and calculate the
  // minimum and maximum array index parts added by the sequential part of the
  // array index function

  for (int i = 0; i < n; i++) {
    ArrayIndFunc *f = indFuncs[i];

    if (f->getDepth() >= DATABLOCK_MAX_NEST_DEPTH) {
      abort(ERR_LOOP_TREE_TOO_DEEP, "loop tree too deep");
    }
    IntVector minv(minp), maxv(maxp);
    int n_par = f->ispace->nPar;
    int n_seq = f->ispace->nSeq;
    f->getIspaceMinMaxPoints(minv, maxv, n_par, n_seq);
    int off = f->getAddend();
    int cur_seq_min = f->calc(minv, n_par, n_seq) + off;
    int cur_seq_max = f->calc(maxv, n_par, n_seq) + off;
    min_arr_seq_ind = MIN(min_arr_seq_ind, cur_seq_min);
    max_arr_seq_ind = MAX(max_arr_seq_ind, cur_seq_max);
  }
  // 2. calculate the workset of a single || iteration, offset and overlap
  wset.extent1 = (max_arr_seq_ind - min_arr_seq_ind + 1)*elemSize;
  wset.offset = min_arr_seq_ind;
  int c0 = getSingleCommonParLoopIndexCoeff();
  wset.overlap = ABS(c0)*elemSize - wset.extent1;

  // workset function depending on the numnber of parallel iterations n:
  // ws(n) = c0*(n-1)*elem_size + wset.extent1
}

void ArrayAccess::updateWorkset(int n_par_iters) {
  int c0 = getSingleCommonParLoopIndexCoeff();
  wset.nParIters = n_par_iters;
  wset.extentN = wset.extent1 + (n_par_iters-1)*ABS(c0)*elemSize;
}

size_t ArrayAccess::getWorksetSize(int n_par_iters) {
  if (n_par_iters != wset.nParIters) {
    updateWorkset(n_par_iters);
  }
  return wset.extentN;
}

void ArrayAccess::checkParCoeffs() {
#if defined(_DEBUG) || defined(DEBUG)
  int c0 = indFuncs[0]->coeffs.comps[0];

  for (int i = 0; i < n; i++) {
    if (indFuncs[i]->coeffs.comps[0] != c0) {
      abort(
        ERR_DIFF_COEFFS,
        "different coeffs at parallel indices not supported");
    }
  }
#endif
}

int ArrayAccess::getSingleCommonParLoopIndexCoeff() {
  checkParCoeffs();
  return indFuncs[0]->coeffs.comps[0];
}

int ParLoopNestDataAccess::getParLoopUpperBound(int loop_num) {
  // TODO(nios) assert all parallel ispaces are the same in all index functions
  // for all arrays, and that loop_num is 0 (since we are 1D)
  return accs[0]->indFuncs[0]->ispace->ub.comps[loop_num];
}

int ParLoopNestDataAccess::getParLoopLowerBound(int loop_num) {
  // TODO(nios) assert all parallel ispaces are the same in all index functions
  // for all arrays, and that loop_num is 0 (since we are 1D)
  return accs[0]->indFuncs[0]->ispace->lb.comps[loop_num];
}

void ParLoopNestDataAccess::partition(size_t mem_size) {
  // 1. calculate the sum function of the worksets of each individual array:
  int c0 = 0;
  size_t ws = 0;

  for (int i = 0; i < n; i++) {
    ArrayAccess *acc = accs[i];
    acc->calcSingleParIterWorkset();
    int c00 = acc->getSingleCommonParLoopIndexCoeff()*acc->elemSize;
    c0 += ABS(c00);
    int align = acc->getWorksetAlignment();
    ws = ALIGN_UP(ws, align);
    ws += acc->getWorksetSize();
  }
  // full workset function is: full_ws(n) = c0*(n-1) + ws
  // max number of iterations fitting into mem: n = (mem - ws) / c0 + 1;

  if (mem_size < ws) {
    printf(
      "not enough memory (" PRN_SIZE_T " bytes) - workset too big (" PRN_SIZE_T " bytes)\n",
      mem_size, ws);
    abort(ERR_OUT_OF_MEMORY, "not enough memory");
  }

  // 2. find the blocking parameters using the full workset function
  iChunk = (mem_size - ws) / c0 + 1;
  int up_i = accs[0]->indFuncs[0]->ispace->ub.comps[0];
  int lo_i = accs[0]->indFuncs[0]->ispace->lb.comps[0];

  if (up_i < lo_i) {
    int tmp = up_i;
    up_i = lo_i;
    lo_i = tmp;
  }
  nChunks = ((up_i-lo_i+1) + iChunk - 1) / iChunk;

  // 3. now update workset for each array - from single parallel iteration
  // to iChunk parallel iterations
  for (int i = 0; i < n; i++) {
    accs[i]->updateWorkset(iChunk);
  }

#ifdef DATABLOCK_DEBUG
  printf("Data blocking for %d bytes, workset " PRN_SIZE_T " bytes:\n", mem_size, ws);
  print(0);
#endif // DATABLOCK_DEBUG
}

void ParLoopNestDataAccess::setupBlockIterationAndCopyIn(
  int iter_num, PartitionData1D *block)
{
#ifdef DATABLOCK_DEBUG
  if (getParLoopUpperBound(0) < getParLoopLowerBound(0) ||
      getParLoopLowerBound(0 < 0))
  {
    abort(ERR_UNSPECIFIED, "unexpected bounds in the parallel loop");
  }
#endif // DATABLOCK_DEBUG
  int global_up = getParLoopUpperBound(0);
  block->parIndLo = iter_num * iChunk;
  int temp_up = block->parIndLo + iChunk - 1;
  block->parIndUp = MIN(global_up, temp_up);
  ArrayIndFunc *f = getArrayIndFunc(0, 0);
  // for now minp and maxp could be just scalars, but in anticipation of
  // future multi-dimensional ||-ion, they are declared vectors
  int minp[DATABLOCK_MAX_PAR_DEPTH];
  int maxp[DATABLOCK_MAX_PAR_DEPTH];
  minp[0] = block->parIndLo;
  maxp[0] = block->parIndUp;
  IntVector minv(minp);
  IntVector maxv(maxp);
  int n_par = f->ispace->nPar;
  f->adjIspaceMinMaxPoints(minv, maxv, 0, n_par);
  int min_par_ind = f->calc(minv, 0, n_par);

  for (int i = 0; i < n; i++) {
    block->arrIndOffsets[i] = min_par_ind + accs[i]->getWorksetOffset();
  }
#ifdef DATABLOCK_DEBUG
  printf("\n");
  printf("block calculated for iteration %d:\n", iter_num);
  block->print(n, 1);
#endif // DATABLOCK_DEBUG
  copyIn(block);
}

void ParLoopNestDataAccess::copyImpl(PartitionData1D *block, bool in) {
  for (int i = 0; i < n; i++) {
    ArrayAccess *acc = accs[i];
    void* loc_arr = locArrs[i];
    byte* big_arr = (byte*)bigArrs[i];
    size_t byte_off = block->arrIndOffsets[i] * acc->elemSize;
    byte* arr_ptr = big_arr + byte_off;
    int n_par_iters = block->getNumParIters();
    size_t ws = acc->getWorksetSize(n_par_iters);

#ifdef DATABLOCK_DEBUG
    printf(
      "Copy (%d iters) %s: big=0x" PRN_PTR " loc=0x" PRN_PTR
      " byte_off=" PRN_SIZE_T " ws size=" PRN_SIZE_T "\n",
      n_par_iters, in ? "in" : "out", big_arr, loc_arr, byte_off, ws);

#endif // DATABLOCK_DEBUG

    if (in) {
      memcpy_l4_to_device(loc_arr, arr_ptr, ws);
    }
    else {
      memcpy_device_to_l4(arr_ptr, loc_arr, ws);
    }
  }
}

void ParLoopNestDataAccess::setArrayMap(void** big_arrs, void** loc_arrs) {
  bigArrs = big_arrs;
  locArrs = loc_arrs;

#ifdef DATABLOCK_DEBUG
  printf("setArrayMap (big->loc):\n");

  for (int i = 0; i < this->n; i++) {
    printf("  0x" PRN_PTR " -> 0x" PRN_PTR "\n", bigArrs[i], locArrs[i]);
  }
#endif // DATABLOCK_DEBUG
}


//-----------------------------------------------------------------------------
// C interface functions implementations
//-----------------------------------------------------------------------------

EXPORT void data_block_ParLoopNestDataAccess_partition(
  struct ParLoopNestDataAccess* obj, size_t mem_size)
{
  obj->partition(mem_size);
}

EXPORT void data_block_ParLoopNestDataAccess_setArrayMap(
  struct ParLoopNestDataAccess* obj, void** big, void **loc)
{
  obj->setArrayMap(big, loc);
}

EXPORT void data_block_ParLoopNestDataAccess_setupBlockIterationAndCopyIn(
  struct ParLoopNestDataAccess* obj, int iter, PartitionData1D *block)
{
  obj->setupBlockIterationAndCopyIn(iter, block);
}

EXPORT void data_block_ParLoopNestDataAccess_copyOut(
  struct ParLoopNestDataAccess* obj, PartitionData1D *block)
{
  obj->copyOut(block);
}

EXPORT unsigned char* data_block_device_alloc_l2(size_t n, int align) {
#ifdef X86_TARGET
  unsigned char* res = (unsigned char*)_aligned_malloc(n, align);
#else
  unsigned char* res = (unsigned char*)fpga_mc_unsafe_malloc_l2(n, align);
#endif // X86_TARGET
#ifdef DATABLOCK_DEBUG
  printf("data_block_device_alloc_l2(" PRN_SIZE_T ", %d) -> 0x" PRN_PTR "\n", n, align, res);
#endif // DATABLOCK_DEBUG
  return res;
}

//-----------------------------------------------------------------------------
// Debug printing
//-----------------------------------------------------------------------------

#ifdef DATABLOCK_DEBUG

#define TAB_STR "  "

static void tb(int tab) {
  for (int i = 0; i < tab; i++) {
    printf("%s", TAB_STR);
  }
}

static void nl() {
  printf("\n");
}

void IntVector::print(int n, int tab) {
  tb(tab);
  for (int i = 0; i < n; i++) {
    printf("%d", comps[i]);

    if (i < n - 1) {
      printf(" ");
    }
  }
}

void IterSpace::print(int tab) {
  tb(tab); printf("IterSpace 0x" PRN_PTR " {\n", this);
  tab++;
  tb(tab); printf("nPar:%d nSeq:%d\n", nPar, nSeq);
  int n_comps = nPar + nSeq;
  tb(tab);
  printf("lb: {"); lb.print(n_comps, 0);
  printf("} ub: {"); ub.print(n_comps, 0);
  printf("}\n");
  tab--;
  tb(tab); printf("}\n");
}

void PartitionData1D::print(int n_arrs, int tab) {
  tb(tab); printf("PartitionData1D 0x" PRN_PTR " {\n", this);
  tab++;
  tb(tab); printf("parIndLo:%d parIndUp:%d\n", parIndLo, parIndUp);
  tb(tab); printf("offsets [%d] (per array): {", n_arrs);
  for (int i = 0; i < n_arrs; i++) {
    printf("%d:" PRN_SIZE_T "", i, arrIndOffsets[i]);

    if (i < n_arrs - 1) {
      printf(" ");
    }
  }
  printf("}\n");
  tab--;
  tb(tab); printf("}\n");
}

void ArrayIndFunc::print(int tab) {
  int n = ispace->nPar + ispace->nSeq;
  tb(tab); printf("ArrayIndFunc 0x" PRN_PTR " {\n", this);
  tab++;
  tb(tab); printf("coeffs [%d]: {", n); coeffs.print(n, 0);
  printf("} addend:%d\n", coeffs.comps[n]);
  tb(tab); printf("iter space: 0x" PRN_PTR "\n", ispace);
  tab--;
  tb(tab); printf("}\n");
}

void Workset1D::print(int tab) {
  tb(tab);
  printf(
    "Workset1D { extent(1):" PRN_SIZE_T " extent(n): " PRN_SIZE_T
    " n:%d offset:%d overlap:%d align:%d }\n",
    extent1, extentN, nParIters, offset, overlap, align);
}

void ArrayAccess::print(int tab) {
  tb(tab); printf("ArrayAccess [%d] 0x" PRN_PTR " {\n", n, this);
  tab++;
  for (int i = 0; i < n; i++) {
    tb(tab); printf("access %d {\n", i);
    tab++;
    tb(tab); printf("elemSize:%d ", elemSize); wset.print(0);
    tb(tab); printf("index function:\n");
    tab++;
    indFuncs[i]->print(tab);
    tab--;
    tab--;
  }
  tab--;
  tb(tab); printf("}\n");
}

static int find_ispace(IterSpace* ispace_arr[], int ispace_arr_len, IterSpace* ispace) {
  for (int i = 0; i < ispace_arr_len; i++) {
    if (ispace_arr[i] == ispace) {
      return i;
    }
  }
  return -1;
}

void ParLoopNestDataAccess::print(int tab) {
  tb(tab); printf("parallel iteration space chunk : %d\n", iChunk);
  tb(tab); printf("number of chunks               : %d\n", nChunks);
  tb(tab); printf("iteration spaces: \n");
  tab++;
  const int MAX_ITER_SPACES = 32;
  int n_ispaces = 0;
  IterSpace* ispaces[MAX_ITER_SPACES];

  for (int i = 0; i < n; i++) {
    ArrayAccess* aa = accs[i];

    for (int j = 0; j < aa->n; j++) {
      ArrayIndFunc* ind_f = aa->indFuncs[j];
      IterSpace* ispace = ind_f->ispace;

      if (find_ispace(ispaces, n_ispaces, ispace) < 0) {
        n_ispaces++;

        if (n_ispaces > MAX_ITER_SPACES) {
          tb(tab); printf("... some iter spaces skipped\n");
          goto break_out;
        }
        ispaces[n_ispaces - 1] = ispace;
        ispace->print(tab);
      }
    }
  }

break_out:
  tab--;
  tb(tab); printf("array accesses (%d arrays):\n", n);
  tab++;

  for (int i = 0; i < n; i++) {
    accs[i]->print(tab);
  }
}
#endif // DATABLOCK_DEBUG
