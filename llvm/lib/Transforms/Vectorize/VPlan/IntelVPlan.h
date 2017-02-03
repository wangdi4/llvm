#ifndef LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTELVPLAN_H 
#define LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTELVPLAN_H 

#include "../VPlan.h" // FIXME
#include "llvm/Support/GenericDomTreeConstruction.h"

namespace llvm {

// TODO: VPLoopBase, VPLoop, VPLoopHIR  
class VPLoop : public VPRegionBlock { 
  
//private: 
//  const unsigned char VLID;   // Subclass identifier (for isa/dyn_cast) 
  
public: 
//  typedef enum { 
//    VPLoopHIRSC,   
//  } VPLoopTy; 
  
//  VPLoop(const std::string &Name, VPLoopTy SC) 
//      : VPRegion(Name, VPLoopSC), VLID(SC) {}

  VPLoop(const std::string &Name)
      : VPRegionBlock(VPBlockBase::VPLoopRegionSC, Name) {}

  /// Method to support type inquiry through isa, cast, and dyn_cast.
  static inline bool classof(const VPRegionBlock *R) {
    return R->getVPBlockID() == VPBlockBase::VPLoopRegionSC;
  }
}; 

/// The IntelVPlanUtils class provides interfaces for the construction and
/// manipulation of a VPlan.
class IntelVPlanUtils : public VPlanUtils {

public:
  IntelVPlanUtils(VPlan *Plan) : VPlanUtils(Plan) {}

  /// Creates a new VPScalarizeOneByOneRecipe or VPVectorizeOneByOneRecipe based
  /// on the isScalarizing parameter respectively.
  // TODO: VPlan is passed in the original interface. Why is this necessary if
  // VPlanUtils already has a copy?
  VPOneByOneRecipeBase *createOneByOneRecipe(const BasicBlock::iterator B,
                                             const BasicBlock::iterator E,
                                             // VPlan *Plan,
                                             bool isScalarizing);

  /// Create a new, empty VPLoop, with no blocks.
  VPLoop *createLoop() {
    VPLoop *Loop = new VPLoop(createUniqueName("loop"));
    setReplicator(Loop, false /*IsReplicator*/);
    return Loop;
  }
};

// TODO: We may need this in VPlan.h/cpp eventually
/// \brief Template specialization of the standard LLVM dominator tree utility
/// for VPBlocks.
class VPDominatorTree : public DominatorTreeBase<VPBlockBase> {

public:
  VPDominatorTree(bool isPostDom) : DominatorTreeBase<VPBlockBase>(isPostDom) {}

  virtual ~VPDominatorTree() {}
};

// From HIR POC
   
// #include "/users/grapapor/LLVM/VPlan/llvm/lib/Transforms/Vectorize/VPlan.h" 
// #include "llvm/IR/Intel_LoopIR/HLLoop.h" 
// #include "llvm/IR/Intel_LoopIR/HLInst.h" 
//   
// using namespace loopopt; 
//   
// namespace llvm { // LLVM Namespace 
// namespace vpo {  // VPO Vectorizer Namespace 
//   
//   
// // VPLoop Classes 
//   
// class VPLoop : public VPRegion { 
//   friend class VPlanHIR; 
//   
// private: 
//   const unsigned char VLID;   // Subclass identifier (for isa/dyn_cast) 
//   
// public: 
//   typedef enum { 
//     VPLoopHIRSC,   
//   } VPLoopTy; 
//   
//   VPLoop(const std::string &Name, VPLoopTy SC) 
//       : VPRegion(Name, VPLoopSC), VLID(SC) {} 
// }; 
//   
// class VPLoopHIR : public VPLoop { 
//   friend class VPlanHIR; 
//   
// private: 
//   /// Pointer to HIR loop node. 
//   HLLoop *HIRLoop; 
//   
// public: 
//   // Interface to create VPLoop from HIR loop. 
//   VPLoopHIR(const std::string &Name, HLLoop *HLoop) 
//       : VPLoop(Name, VPLoopHIRSC), HIRLoop(HLoop) {} 
//   
//   // VPLoop copy constructor. 
//   //VPLoopHIR(VPLoopHIR &VPOrigLoop); 
//   
//   virtual ~VPLoopHIR() override {} 
// }; 
//   
//   
// // VPIf Classes 
//   
// class VPIf : public VPRegion { 
//   friend class VPlanHIR; 
//   
// private: 
//   const unsigned char VIID;   // Subclass identifier (for isa/dyn_cast) 
//   
//   /// 'Then' branch chain 
//   VPChain *ThenChain = nullptr; 
//   
//   /// 'Else' branch chain. 
//   VPChain *ElseChain = nullptr; 
//   
// public: 
//   typedef enum { 
//     VPIfHIRSC,   
//   } VPIfTy; 
//   
//   VPIf(const std::string &Name, VPIfTy SC) 
//       : VPRegion(Name, VPIfSC), VIID(SC) {} 
//   
//   VPBlock *getThenChain() { return ThenChain; } 
//   VPBlock *getElseChain() { return ElseChain; } 
// }; 
//   
// class VPIfHIR : public VPIf { 
//   friend class VPlanHIR; 
//   
// private: 
//    /// Pointer to HIR If node. 
//   HLIf *HIRIf; 
//   
// public: 
//   // Interface to create AVRLoop from LLVM Loop. 
//   VPIfHIR(const std::string &Name, HLIf *HLIf) 
//       : VPIf(Name, VPIfHIRSC), HIRIf(HLIf) {} 
//   
//   // VPIf copy constructor. 
//   //VPIfHIR(VPIfHIR &VPOrigLoop); 
//   
//   virtual ~VPIfHIR() override {} 
// }; 
//   
//   
// // VPRecipes 
//   
// // Test 
// //class VPOneByOneRecipeHIR : public VPRecipe { 
// //private: 
// // 
// ////  bool IsScalarizing = false; 
// // 
// //protected: 
// //  //VPOneByOneRecipeHIR(bool IsScalarizing) : IsScalarizing(IsScalarizing) {} 
// //  VPOneByOneRecipeHIR() {} 
// // 
// //  // Do the actual vectorization work for a single instruction 
// //  // TODO 
// //  void vectorizeInstruction(HLInst *I, VPContext& Context) {}; 
// //}; 
// // 
// //typedef ilist<HLInst> HLInstList; 
// // 
// //class VPOneByOneShadowRecipeHIR : public VPOneByOneRecipeHIR { 
// //private: 
// //  HLInstList::iterator Begin; 
// //  HLInstList::iterator End; 
// // 
// //public: 
// //  VPOneByOneShadowRecipeHIR(const HLInstList::iterator &B, 
// //                            const HLInstList::iterator &E) 
// //      // const HLInstList::iterator& E, bool IsScalarizing) 
// //      : Begin(B), 
// //        End(E) {} 
// //  //: VPOneByOneRecipe(IsScalarizing), Begin(B), End(E) {} 
// // 
// //  void vectorize(VPContext& Context) override { 
// //    for (auto It = Begin; It != End; ++It) 
// //      vectorizeInstruction(&*It, Context); 
// //  } 
// // 
// //  void print(raw_ostream &O) const override { 
// //    bool First = true; 
// //    for (auto It = Begin; It != End; ++It) { 
// //      if (!First) 
// //        O << "\n"; 
// //      formatted_raw_ostream FO(O); 
// //      It->print(FO, 1);//->printAsOperand(O, true); 
// //      First = false; 
// //    } 
// //  } 
// //}; 
//   
//   
// // VPlan 
//   
// class VPlanHIR : public VPlan { 
//   
// public:  
//   
//   //TODO: Move to utils 
//   
//   VPLoopHIR *createLoopHIR(HLLoop *HLoop, bool IsReplicator = false) { 
//     VPLoopHIR *Loop = new VPLoopHIR(createUniqueName("loop"), HLoop); 
//     if (IsReplicator) 
//       setReplicator(Loop); 
//     return Loop; 
//   } 
//   
//   VPIfHIR *createIfHIR(HLIf *HIf, bool IsReplicator = false) { 
//     VPIfHIR *If = new VPIfHIR(createUniqueName("if"), HIf); 
//     if (IsReplicator) 
//       setReplicator(If); 
//     return If; 
//   } 
//   
//   void setIfThenChain(VPIf *VIf, VPChain *ThenChain) { 
//     assert(!VIf->getThenChain() && "ThenChain already set in this VPIf"); 
//     VIf->ThenChain = ThenChain; 
//   } 
//   
//   void setIfElseChain(VPIf *VIf, VPChain *ElseChain) { 
//     assert(!VIf->getElseChain() && "ElseChain already set in this VPIf"); 
//     VIf->ElseChain = ElseChain; 
//   } 
// }; 
//   
// } // End VPO Vectorizer Namespace 

}  // namespace llvm

  
#endif // LLVM_TRANSFORMS_VECTORIZE_VPLAN_INTELVPLAN_H 

