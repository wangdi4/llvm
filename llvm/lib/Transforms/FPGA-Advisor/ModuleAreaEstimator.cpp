#include "ModuleAreaEstimator.h"

// std
#include <cstdio>
#include <cstring>
#include <iostream>
#include <dlfcn.h>

using namespace std;

namespace fpga {
ModuleAreaEstimator::ModuleAreaEstimator() : ModulePass(ID) {
  char *analyzerLib;

  if ((analyzerLib = getenv("FPGA_ADVISOR_USE_DYNAMIC_ANALYZER"))) {
    useDefault = false;
    // Load up the library
    analyzerLibHandle = dlopen(analyzerLib, RTLD_GLOBAL | RTLD_LAZY);

    if (analyzerLibHandle == NULL) {
      printf("failed to load %s\n", analyzerLib);
      std::cerr << dlerror() << std::endl;
      exit(1);
    }

    // pull objects out of the library
    assert(sizeof(getBlockArea) == sizeof(analyzerLibHandle));
    memcpy(&getBlockArea, &analyzerLibHandle, sizeof(analyzerLibHandle));

    if (getBlockArea == NULL) {
      printf("failed to load getBlockArea\n");
      exit(1);
    }
  }
}

bool ModuleAreaEstimator::runOnModule(Module &M) {
  std::cerr << "ModuleAreaEstimator:" << __func__ << "\n";
  for (auto &F : M) {
    visit(F);
  }
  return true;
}

void ModuleAreaEstimator::visitBasicBlock(BasicBlock &BB) {
  int area = 0;

  if (useDefault) {
    // use some fallback estimator code
    // approximate area of basic block as a weighted sum
    // the weight is the complexity of the instruction
    // the sum is over all compute instructions
    // W = 1 + x1y1 + x2y2 + ... + xnyn
    // x1 is the complexity of the operation
    // y1 is the number of this operation existing in the basic block

    for (Instruction &I : BB) {
      area += instructionAreaComplexity(&I);
    }
  } else {
    area = getBlockArea(&BB);
  }

  areaTable.insert(std::make_pair(BB.getTerminator()->getParent(), area));
}

int ModuleAreaEstimator::instructionAreaComplexity(Instruction *I) {
  // basic complexity of instruction is 1
  int complexity = 1;
  if (instructionNeedsFp(I)) {
    complexity += getFpAreaCost();
  }
  if (instructionNeedsGlobalMemory(I)) {
    complexity += getGlobalMemoryAreaCost();
  }
  if (instructionNeedsMuxes(I)) {
    complexity += getMuxAreaCost(I);
  }
  return complexity;
}

bool ModuleAreaEstimator::instructionNeedsFp(Instruction *I) {
  switch (I->getOpcode()) {
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::FCmp:
    return true;
    break;
  default:
    return false;
    break;
  }
  // here we don't consider call instructions that may
  // possibly return a float
}

bool ModuleAreaEstimator::instructionNeedsGlobalMemory(Instruction *I) {
  // check that instruction is memory instruction
  if (!I->mayReadOrWriteMemory()) {
    return false;
  }
  // reads/writes memory, check the location of access
  // this may either be a load/store or function call
  // look into memory dependence analysis and memory location
  // which gives info about the size and starting location of
  // the location pointed to by a pointer...
  // TODO FIXME need to finish this function
  return true;
}

bool ModuleAreaEstimator::instructionNeedsMuxes(Instruction *I) {
  // instructions that need muxing are:
  // switch instructions and phi nodes
  if (isa<SwitchInst>(I)) {
    return true;
  } else if (isa<PHINode>(I)) {
    return true;
  }
  return false;
}

int ModuleAreaEstimator::getMuxAreaCost(Instruction *I) {
  // only incur cost to large muxes
  if (SwitchInst *SwI = dyn_cast<SwitchInst>(I)) {
    // proportional to the size
    return (int)SwI->getNumCases() / 16;
  } else if (PHINode *PN = dyn_cast<PHINode>(I)) {
    // proportional to the size
    return (int)PN->getNumIncomingValues() / 16;
  }
  return 0;
}
}
