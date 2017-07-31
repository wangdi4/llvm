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

#include "data_block.h"

#define ALIGN_UP(val, al) ( (val+al-1) & ~(al-1) )
#define MIN(x1,x2) (x1 > x2  ? x2 : x1)
#define MAX(x1,x2) (x1 >= x2 ? x1 : x2)


typedef unsigned char byte;


// TODO(nios) temp dummy implementations
size_t nios_get_free_l2_mem_size() { return 0; }
size_t nios_get_free_l3_mem_size() { return 0; }

void* nios_memcpy_l4_to_device(void* dst, void *src, size_t n) { return dst; }
void* nios_memcpy_device_to_l4(void* dst, void *src, size_t n) { return dst; }
void  nios_abort(const char *msg) {}


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

void ArrayIndFunc::getSeqMinMaxPoints(IntVector minp, IntVector maxp) {
  int start = ispace->nPar, end = start + ispace->nSeq;

  for (int i = start; i < end; i++) {
    bool gt = coeffs.comps[i] > 0;
    minp.comps[i] = gt ? ispace->lb.comps[i] : ispace->ub.comps[i];
    maxp.comps[i] = gt ? ispace->ub.comps[i] : ispace->lb.comps[i];
  }
}

void ArrayAccess::calcWorkset() {
  int min_arr_ind = INT_MAX;
  int max_arr_ind = INT_MIN;
  const int max_seq_depth = 8;

  if (max_seq_depth < n) {
    nios_abort("loop tree too deep");
  }
  int minp[max_seq_depth];
  int maxp[max_seq_depth];
  int c0 = getSingleCommonParLoopIndexCoeff();

  // 1. loop over all index functions for current array and calculate the
  // minimum and maximum array index parts added by the sequential part of the
  // array index function

  for (int i = 0; i < n; i++) {
    ArrayIndFunc *f = indFuncs[i];
    IntVector minv(minp), maxv(maxp);
    f->getSeqMinMaxPoints(minv, maxv);
    int n_par = f->ispace->nPar;
    int n_seq = f->ispace->nSeq;
    int cur_min = f->calc(minv, n_par, n_seq) + f->getOffset();
    int cur_max = f->calc(maxv, n_par, n_seq) + f->getOffset();
    min_arr_ind = MIN(min_arr_ind, cur_min);
    max_arr_ind = MAX(max_arr_ind, cur_max);
  }
  // 2. calculate the workset of a single || iteration, offset and overlap
  wset.extent = (max_arr_ind - min_arr_ind)*elemSize;
  wset.offset = min_arr_ind*elemSize;
  wset.overlap = c0*elemSize - wset.extent;

  // workset function depending on the numnber of parallel iterations n:
  // ws(n) = c0*n + wset.extent
}

void ArrayAccess::checkParCoeffs() {
#if defined(_DEBUG) || defined(DEBUG)
  int c0 = indFuncs[0]->coeffs.comps[0];

  for (int i = 0; i < n; i++) {
    if (indFuncs[i]->coeffs.comps[0] != c0) {
      nios_abort("different coeffs at parallel indices not supported");
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

void ParLoopNestDataAccess::partition(size_t mem_size) {
  // 1. calculate the sum function of the worksets of each individual array:
  int c0 = 0;
  size_t ws = 0;

  for (int i = 0; i < n; i++) {
    ArrayAccess *acc = accs[i];
    acc->calcWorkset();
    c0 += acc->getSingleCommonParLoopIndexCoeff();
    int align = acc->getWorksetAlignment();
    ws = ALIGN_UP(ws, align);
    ws += acc->getWorksetSize();
  }
  // full workset function is: full_ws(n) = c0*n + ws
  // max number of iterations fitting into mem: n = (mem - ws) / c0;

  // 2. find the blocking parameters using the full workset function
  iChunk = (mem_size - ws) / c0;
  int up_i = accs[0]->indFuncs[0]->ispace->ub.comps[0];
  nChunks = (up_i + iChunk - 1) / iChunk;
}

void ParLoopNestDataAccess::setupBlockIterationAndCopyIn(
  int iter_num, PartitionData1D *block)
{
  int global_up = getParLoopUpperBound(0);
  block->parIndLo = iter_num * iChunk;
  int temp_up = block->parIndLo + iChunk;
  block->parIndUp = MIN(global_up, temp_up);
  ArrayIndFunc *f = getArrayIndFunc(0, 0);
  IntVector par_ind(&block->parIndLo);
  size_t par_off = f->calcParPart(par_ind);

  for (int i = 0; i < n; i++) {
    block->arrIndOffsets[i] = par_off + accs[i]->getWorksetOffset();
  }
  copyIn(block);
}


void ParLoopNestDataAccess::copyImpl(PartitionData1D *block, bool in) {
  for (int i = 0; i < n; i++) {
    ArrayAccess *acc = accs[i];
    void* loc_arr = locArrs[i];
    byte* big_arr = (byte*)bigArrs[i];
    byte* arr_ptr = big_arr + block->arrIndOffsets[i];
    size_t ws = acc->getWorksetSize();

    if (in) {
      nios_memcpy_l4_to_device(loc_arr, arr_ptr, ws);
    }
    else {
      nios_memcpy_device_to_l4(arr_ptr, loc_arr, ws);
    }
  }
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
