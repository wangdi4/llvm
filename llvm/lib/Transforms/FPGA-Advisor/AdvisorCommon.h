//===- FPGA-Advisor-Analysis.h - Main FPGA-Advisor pass definition -------*- C++
//-*-===//
//
// Copyright (c) 2016, Intel Corporation
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//===----------------------------------------------------------------------===//
//
// This file contains the class declarations for all the analysis
// that are useful for the FPGA-Advisor-Analysis.
//===----------------------------------------------------------------------===//
// Author: chenyuti
//===----------------------------------------------------------------------===//
//
// This file contains the shared typedefs for FPGA analyses

#ifndef LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_COMMON_H
#define LLVM_LIB_TRANSFORMS_FPGA_ADVISOR_COMMON_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/IR/Instruction.def"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/stack.hpp>

#include <algorithm>

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <dlfcn.h>

#define SINGLE_THREAD_TID 0

using namespace llvm;

namespace fpga {

typedef struct {
  std::vector<Loop *> subloops;
  uint64_t maxIter;
  uint64_t parIter;
} LoopIterInfo;

typedef struct {
  Function *function;
  LoopInfo *loopInfo;
  std::vector<BasicBlock *> bbList;
  std::vector<Instruction *> instList;
  std::vector<LoopIterInfo> loopList;
  std::vector<LoadInst *> loadList;
  std::vector<StoreInst *> storeList;
} FunctionInfo;

typedef struct {
  int acceleratorLatency;
  int acceleratorII;
  int cpuLatency;
} LatencyStruct;

// Dependence Graph type:
// STL list container for OutEdge list
// STL vector container for vertices
// Use directed edges
// the boolean is an edge property which is true when the dependence edge exists
// due to a true dependence
// it may also have memory dependences
// typedef boost::property <edge_custom_t, bool> TrueDependence;
struct true_dependence_t {
  typedef boost::edge_property_tag kind;
};
typedef boost::property<true_dependence_t, bool> TrueDependence;
// typedef boost::property<boost::edge_index_t, bool> TrueDependence;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS,
                              BasicBlock *, TrueDependence>
    DepGraph;

} // end fpga namespace

#endif
