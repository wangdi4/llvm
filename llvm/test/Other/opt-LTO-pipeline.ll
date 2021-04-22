; RUN: opt -enable-new-pm=0 -mtriple=x86_64-- -std-link-opts -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck --check-prefix=CHECK %s

; REQUIRES: asserts

; INTEL_CUSTOMIZATION
; CHECK-LABEL: Pass Arguments:
; CHECK-NEXT: Target Library Information
; CHECK-NEXT: Target Transform Information
;           : Target Pass Configuration
; CHECK:      Type-Based Alias Analysis
; CHECK-NEXT: Scoped NoAlias Alias Analysis
; CHECK-NEXT: Std Container Alias Analysis
; CHECK-NEXT: Profile summary info
; CHECK-NEXT: Assumption Cache Tracker
; CHECK-NEXT: Optimization report options pass
; CHECK-NEXT:   ModulePass Manager
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Module Verifier
; CHECK-NEXT:     Dead Global Elimination
; CHECK-NEXT:     Whole program analysis
; CHECK-NEXT:     Intel IPO Prefetch
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Post-Dominator Tree Construction
; CHECK-NEXT:     Whole program analysis
; CHECK-NEXT:     Intel fold WP intrinsic
; CHECK-NEXT:     IP Cloning
; CHECK-NEXT:     Dynamic Casts Optimization Pass
; CHECK-NEXT:     Force set function attributes
; CHECK-NEXT:     Infer set function attributes
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Call-site splitting
; CHECK-NEXT:     PGOIndirectCallPromotion
; CHECK-NEXT:     Interprocedural Sparse Conditional Constant Propagation
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:     Called Value Propagation
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     Call Graph SCC Pass Manager
; CHECK-NEXT:       Deduce function attributes
; CHECK-NEXT:     Deduce function attributes in RPO
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Remove redundant instructions
; CHECK-NEXT:       Simplify the CFG
; CHECK-NEXT:     Global splitter
; CHECK-NEXT:     Whole program devirtualization
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:     DopeVectorConstProp
; CHECK-NEXT:     Global Variable Optimizer
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Post-Dominator Tree Construction
; CHECK-NEXT:         Branch Probability Analysis
; CHECK-NEXT:         Block Frequency Analysis
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Promote Memory to Register
; CHECK-NEXT:     Merge Duplicate Global Constants
; CHECK-NEXT:     Dead Argument Elimination
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Combine redundant instructions
; CHECK-NEXT:     Setup inlining report
; CHECK-NEXT:     Set attributes for callsites in [no]inline list
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     Andersen Interprocedural AA
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Indirect Call Conv
; CHECK-NEXT:     AggInliner
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     Call Graph SCC Pass Manager
; CHECK-NEXT:       Function Integration/Inlining
; CHECK-NEXT:       Remove unused exception handling info
; CHECK-NEXT:       OpenMP specific optimizations
; CHECK-NEXT:     Global Variable Optimizer
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Post-Dominator Tree Construction
; CHECK-NEXT:         Branch Probability Analysis
; CHECK-NEXT:         Block Frequency Analysis
; CHECK-NEXT:     Partial Inliner
; CHECK-NEXT:     IP Cloning
; CHECK-NEXT:     Call-Tree Cloning (with MultiVersioning)
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:     Interprocedural Sparse Conditional Constant Propagation
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:     Dead Global Elimination
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     Call Graph SCC Pass Manager
; CHECK-NEXT:       Promote 'by reference' arguments to scalars
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:         Function Alias Analysis Results
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Lazy Branch Probability Analysis
; CHECK-NEXT:         Lazy Block Frequency Analysis
; CHECK-NEXT:         Optimization Remark Emitter
; CHECK-NEXT:         Combine redundant instructions
; CHECK-NEXT:         Lazy Value Information Analysis
; CHECK-NEXT:         Post-Dominator Tree Construction
; CHECK-NEXT:         Jump Threading
; CHECK-NEXT:         SROA
; CHECK-NEXT:     LTO Array Transpose
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Scalar Evolution Analysis
; CHECK-NEXT:     DeadArrayOpsElimination
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Scalar Evolution Analysis
; CHECK-NEXT:         Array Use Analysis
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Lazy Value Information Analysis
; CHECK-NEXT:       Value Propagation
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Tail Call Elimination
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     Call Graph SCC Pass Manager
; CHECK-NEXT:       Deduce function attributes
; CHECK-NEXT:     Globals Alias Analysis
; CHECK-NEXT:     Andersen Interprocedural AA
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Memory SSA
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Canonicalize natural loops
; CHECK-NEXT:       LCSSA Verifier
; CHECK-NEXT:       Loop-Closed SSA Form Pass
; CHECK-NEXT:       Scalar Evolution Analysis
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Loop Pass Manager
; CHECK-NEXT:         Loop Invariant Code Motion
; CHECK-NEXT:       Phi Values Analysis
; CHECK-NEXT:       Memory Dependence Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Global Value Numbering
; CHECK-NEXT:       DopeVector Hoist
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       MemCpy Optimization
; CHECK-NEXT:       Post-Dominator Tree Construction
; CHECK-NEXT:       Dead Store Elimination
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       MergedLoadStoreMotion
; CHECK-NEXT:       Canonicalize natural loops
; CHECK-NEXT:       LCSSA Verifier
; CHECK-NEXT:       Loop-Closed SSA Form Pass
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Scalar Evolution Analysis
; CHECK-NEXT:       Loop Pass Manager
; CHECK-NEXT:         Induction Variable Simplification
; CHECK-NEXT:         Delete dead loops
; CHECK-NEXT:         Unroll loops
; CHECK-NEXT:     VecClone
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Early CSE
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Canonicalize natural loops
; CHECK-NEXT:       Lazy Value Information Analysis
; CHECK-NEXT:       Lower SwitchInst's to branches
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       LCSSA Verifier
; CHECK-NEXT:       Loop-Closed SSA Form Pass
; CHECK-NEXT:       VPO CFGRestructuring
; CHECK-NEXT:     Function Outlining of Ordered Regions
; CHECK-NEXT:       FunctionPass Manager
; CHECK-NEXT:         Dominator Tree Construction
; CHECK-NEXT:         Natural Loop Information
; CHECK-NEXT:         Scalar Evolution Analysis
; CHECK-NEXT:         Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:         Function Alias Analysis Results
; CHECK-NEXT:         VPO Work-Region Collection
; CHECK-NEXT:         Lazy Branch Probability Analysis
; CHECK-NEXT:         Lazy Block Frequency Analysis
; CHECK-NEXT:         Optimization Remark Emitter
; CHECK-NEXT:         VPO Work-Region Information
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       VPO CFGRestructuring
; CHECK-NEXT:       Replace known math operations with optimized library functions
; CHECK-NEXT:       LCSSA Verifier
; CHECK-NEXT:       Loop-Closed SSA Form Pass
; CHECK-NEXT:       Scalar Evolution Analysis
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       VPO Work-Region Collection
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       VPO Work-Region Information
; CHECK-NEXT:       Demanded bits analysis
; CHECK-NEXT:       Loop Access Analysis
; CHECK-NEXT:       VPlan Vectorization Driver
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Replace known math operations with optimized library functions
; CHECK-NEXT:     CallGraph Construction
; CHECK-NEXT:     Call Graph SCC Pass Manager
; CHECK-NEXT:       Inliner for always_inline functions
; CHECK-NEXT:     A No-Op Barrier Pass
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       VPO Directive Cleanup
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Scalar Evolution Analysis
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Loop Access Analysis
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Loop Distribution
; CHECK-NEXT:       Canonicalize natural loops
; CHECK-NEXT:       LCSSA Verifier
; CHECK-NEXT:       Loop-Closed SSA Form Pass
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Scalar Evolution Analysis
; CHECK-NEXT:       Loop Pass Manager
; CHECK-NEXT:         Unroll loops
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Warn about non-applied transformations
; CHECK-NEXT:       Combine redundant instructions
; CHECK-NEXT:       Simplify the CFG
; CHECK-NEXT:       Sparse Conditional Constant Propagation
; CHECK-NEXT:       Dominator Tree Construction
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Natural Loop Information
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Combine redundant instructions
; CHECK-NEXT:       Demanded bits analysis
; CHECK-NEXT:       Bit-Tracking Dead Code Elimination
; CHECK-NEXT:       Optimize scalar/vector ops
; CHECK-NEXT:       Scalar Evolution Analysis
; CHECK-NEXT:       Alignment from assumptions
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Unaligned Nontemporal Store Conversion
; CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:       Function Alias Analysis Results
; CHECK-NEXT:       Lazy Branch Probability Analysis
; CHECK-NEXT:       Lazy Block Frequency Analysis
; CHECK-NEXT:       Optimization Remark Emitter
; CHECK-NEXT:       Combine redundant instructions
; CHECK-NEXT:       Lazy Value Information Analysis
; CHECK-NEXT:       Post-Dominator Tree Construction
; CHECK-NEXT:       Jump Threading
; CHECK-NEXT:       Forced CMOV generation
; CHECK-NEXT:     Emit inlining report
; CHECK-NEXT:     Cross-DSO CFI
; CHECK-NEXT:     Lower type metadata
; CHECK-NEXT:     Lower type metadata
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Simplify the CFG
; CHECK-NEXT:     Eliminate Available Externally Globals
; CHECK-NEXT:     Dead Global Elimination
; CHECK-NEXT:     FunctionPass Manager
; CHECK-NEXT:       Annotation Remarks
; CHECK-NEXT:       Module Verifier
; CHECK-NEXT:     Bitcode Writer
; CHECK-NEXT: Pass Arguments:  -domtree -postdomtree
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Dominator Tree Construction
; CHECK-NEXT:     Post-Dominator Tree Construction
; CHECK-NEXT: Pass Arguments:  -domtree
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Dominator Tree Construction
; CHECK-NEXT: Pass Arguments:  -domtree
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Dominator Tree Construction
; CHECK-NEXT: Pass Arguments:  -targetlibinfo -tti -domtree -loops -postdomtree -branch-prob -block-freq
; CHECK-NEXT: Target Library Information
; CHECK-NEXT: Target Transform Information
; CHECK-NEXT:   FunctionPass Manager
; CHECK-NEXT:     Dominator Tree Construction
; CHECK-NEXT:     Natural Loop Information
; CHECK-NEXT:     Post-Dominator Tree Construction
; CHECK-NEXT:     Branch Probability Analysis
; CHECK-NEXT:     Block Frequency Analysis
; END INTEL_CUSTOMIZATION

define void @f() {
  ret void
}
