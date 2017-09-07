#include "ModuleScheduler.h"

namespace fpga {

ModuleScheduler::ModuleScheduler() : ModulePass(ID) {
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
    assert(sizeof(getBlockLatency) == sizeof(analyzerLibHandle));
    memcpy(&getBlockLatency, &analyzerLibHandle, sizeof(analyzerLibHandle));

    if (getBlockLatency == NULL) {
      printf("failed to load getBlockLatency\n");
      exit(1);
    }

    assert(sizeof(getBlockII) == sizeof(analyzerLibHandle));
    memcpy(&getBlockII, &analyzerLibHandle, sizeof(analyzerLibHandle));

    if (getBlockII == NULL) {
      printf("failed to load getBlockII\n");
      exit(1);
    }
  }
}

bool ModuleScheduler::runOnModule(Module &M) override {
  std::cerr << "ModuleScheduler:" << __func__ << "\n";
  for (auto &F : M) {
    visit(F);
  }
  return true;
}

ModuleScheduler::visitBasicBlock(BasicBlock &BB) {
  LatencyStruct latencyStruct;
  latencyStruct.cpuLatency = 0;

  for (Instruction &I : BB) {
    latencyStruct.cpuLatency += get_instruction_latency(&I);
  }

  if (useDefault) {
    // approximate latency of basic block as number of instructions
    latencyStruct.acceleratorLatency = latencyStruct.cpuLatency;
    latencyStruct.acceleratorII = latencyStruct.cpuLatency;
  } else {
    latencyStruct.acceleratorLatency = getBlockLatency(&BB);
    latencyStruct.acceleratorII = getBlockII(&BB);
  }

  latencyTableFPGA.insert(
      std::make_pair(BB.getTerminator()->getParent(), latencyStruct));
}

int ModuleScheduler::getInstructionLatency(Instruction *I) {
  int latency = 0;
  switch (I->getOpcode()) {
  // simple binary and logical operations
  case Instruction::Add:
  case Instruction::Sub:
  case Instruction::Shl:
  case Instruction::LShr:
  case Instruction::AShr:
  case Instruction::And:
  case Instruction::Or:
  case Instruction::Xor:
    latency = 1;
    break;

  // complicated binary operations
  case Instruction::Mul:
  case Instruction::UDiv:
  case Instruction::SDiv:
  case Instruction::URem:
  case Instruction::SRem:
    latency = 10;
    break;

  // FP operations
  case Instruction::FAdd:
  case Instruction::FSub:
  case Instruction::FMul:
  case Instruction::FDiv:
  case Instruction::FRem:
    latency = 15;
    break;

  // memory operations
  case Instruction::Alloca:
    latency = 0;
    break;
  case Instruction::GetElementPtr:
    latency = 1;
    break;
  case Instruction::Load:
  case Instruction::Store:
  case Instruction::Fence:
  case Instruction::AtomicCmpXchg:
  case Instruction::AtomicRMW:
    latency = 5;
    break;

  // cast operations
  // these shouldn't take any cycles
  case Instruction::Trunc:
  case Instruction::ZExt:
  case Instruction::SExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::BitCast:
    latency = 0;
    break;

  // more complicated cast operations
  case Instruction::FPToUI:
  case Instruction::FPToSI:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::AddrSpaceCast:
    latency = 5;
    break;

  // other
  case Instruction::ICmp:
  case Instruction::FCmp:
  case Instruction::PHI:
  case Instruction::Select:
  case Instruction::UserOp1:
  case Instruction::UserOp2:
  case Instruction::VAArg:
  case Instruction::ExtractElement:
  case Instruction::InsertElement:
  case Instruction::ShuffleVector:
  case Instruction::ExtractValue:
  case Instruction::InsertValue:
  case Instruction::LandingPad:
    latency = 5;
    break;

  case Instruction::Call:
    latency = 100;
    break; // can be more sophisticated!!!

  case Instruction::Ret:
  case Instruction::Br:
  case Instruction::Switch:
  case Instruction::Resume:
  case Instruction::Unreachable:
    latency = 0;
    break;
  case Instruction::Invoke:
    latency = 100;
    break; // can be more sophisticated!!!
  case Instruction::IndirectBr:
    latency = 10;
    break;

  default:
    latency = 1;
    std::cerr << "Warning: unknown operation " << I->getOpcodeName() << "\n";
    break;
  }

  return latency;
}
}
