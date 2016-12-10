//===-- CSAStatistics.cpp - CSA Statistics --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file "reexpresses" the code containing traditional control flow
// into a basically data flow representation suitable for the CSA.
//
//===----------------------------------------------------------------------===//

#include <map>
#include "CSA.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "CSAInstrInfo.h"
#include "CSATargetMachine.h"
#include "CSALicAllocation.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SparseSet.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineSSAUpdater.h"
#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "MachineCDG.h"
#include "CSAInstrInfo.h"

using namespace llvm;

#define DEBUG_TYPE "csa-statistics"

STATISTIC(NumPICKS, "Number of PICK instrucitons generated");
STATISTIC(NumSWITCHES, "Number of SWITCH instructions generated");
STATISTIC(NumCOPYS, "Number of COPY instructions generated");
STATISTIC(NumINITS, "Number of LIC INITIALIZE instructions generated");
STATISTIC(NumADDS, "Number of Add instrucitons generated");
STATISTIC(NumSUBS, "Number of Sub instrucitons generated");
STATISTIC(NumFMAS, "Number of FMA instrucitons generated");
STATISTIC(NumMULS, "Number of MUL instrucitons generated");
STATISTIC(NumDIVS, "Number of DIV instrucitons generated");
STATISTIC(NumLOADS, "Number of LOAD instrucitons generated");
STATISTIC(NumSTORES, "Number of STORE instrucitons generated");
STATISTIC(NumCMPS, "Number of COMPARE instrucitons generated");
STATISTIC(NumSHIFTS, "Number of SHIFT instrucitons generated");


static cl::opt<int>
CSAStatisticsPass("csa-statistics", cl::Hidden,
               cl::desc("CSA Specific: Statistics"),
               cl::init(1));



namespace {
  class CSAStatistics : public MachineFunctionPass {
  public:
    static char ID;
    CSAStatistics();

    StringRef getPassName() const override {
      return "CSA Statistics";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;
  private:
    MachineFunction *thisMF;
  };
}

namespace llvm {
    void initializeCSAStatisticsPass(PassRegistry&);
}

//  Because of the namespace-related syntax limitations of gcc, we need
//  To hoist init out of namespace blocks. 
char CSAStatistics::ID = 0;
INITIALIZE_PASS(CSAStatistics, "csa-statistics", "CSA Statistics", true, true)

CSAStatistics::CSAStatistics() : MachineFunctionPass(ID) {
    initializeCSAStatisticsPass(*PassRegistry::getPassRegistry());
}

MachineFunctionPass *llvm::createCSAStatisticsPass() {
  return new CSAStatistics();
}

bool CSAStatistics::runOnMachineFunction(MachineFunction &MF) {

  if (CSAStatisticsPass == 0) return false;
  thisMF = &MF;
  const CSAInstrInfo &TII = *static_cast<const CSAInstrInfo*>(thisMF->getSubtarget().getInstrInfo());
  bool Modified = false;
  
  for (MachineFunction::iterator BB = thisMF->begin(), E = thisMF->end(); BB != E; ++BB) {
	  for (MachineBasicBlock::iterator II = BB->begin(), E = BB->end(); II != E; ++II) {
      MachineInstr *I = &*II;
		  if (TII.isSwitch(I)) {
			  NumSWITCHES++;
		  } else if (TII.isPick(I)) {
			  NumPICKS++;
		  } else if (TII.isInit(I)) {
			  NumINITS++;
		  } else if (TII.isCopy(I)) {
			  NumCOPYS++;
		  }
		  else if (TII.isLoad(I)) {
			  NumLOADS++;
		  }
		  else if (TII.isStore(I)) {
			  NumSTORES++;
		  }
		  else if (TII.isFMA(I)) {
			  NumFMAS++;
		  }
		  else if (TII.isMul(I)) {
			  NumMULS++;
		  }
		  else if (TII.isDiv(I)) {
			  NumDIVS++;
		  }
		  else if (TII.isAdd(I)) {
			  NumADDS++;
		  }
		  else if (TII.isSub(I)) {
			  NumSUBS++;
		  }
		  else if (TII.isShift(I)) {
			  NumSHIFTS++;
		  }
		  else if (TII.isCmp(I)) {
			  NumCMPS++;
		  }
	  }
  }

  std::string Filename = thisMF->getName().str() + "_stats" + ".txt";
  std::error_code EC;
  raw_fd_ostream File1(Filename, EC, sys::fs::F_Text);
  DEBUG(errs() << "Writing '" << Filename << "'...");


  PrintStatistics(File1);

  return Modified;

}

