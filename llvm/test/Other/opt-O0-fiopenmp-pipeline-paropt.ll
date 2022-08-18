; INTEL_CUSTOMIZATION
; RUN: opt -disable-verify -enable-new-pm=0 -O0 -paropt=31 -vecopt=true -debug-pass=Structure -S -disable-output %s 2>&1 |  FileCheck %s
;CHECK:      Pass Arguments:
;            Target Transform Information
;            Xmain opt level pass
;CHECK:      Assumption Cache Tracker
;CHECK-NEXT: Target Library Information
;CHECK-NEXT: Profile summary info
;CHECK:      Optimization report options pass
;CHECK:        FunctionPass Manager
;                Module Verifier
;                Subscript Intrinsic Lowering
;                Instrument function entry/exit with calls to e.g. mcount() (pre inlining)
;CHECK:          Dominator Tree Construction
;CHECK-NEXT:     Natural Loop Information
;CHECK-NEXT:     VPO CFGRestructuring
;CHECK-NEXT:     Scalar Evolution Analysis
;CHECK-NEXT:     Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:     Function Alias Analysis Results
;CHECK-NEXT:     VPO Work-Region Collection
;CHECK-NEXT:     Lazy Branch Probability Analysis
;CHECK-NEXT:     Lazy Block Frequency Analysis
;CHECK-NEXT:     Optimization Remark Emitter
;CHECK-NEXT:     VPO Work-Region Information
;CHECK-NEXT:     VPO Paropt Apply Config
;CHECK-NEXT:     Dominator Tree Construction
;CHECK-NEXT:     Natural Loop Information
;CHECK-NEXT:     Scalar Evolution Analysis
;CHECK-NEXT:     Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:     Function Alias Analysis Results
;CHECK-NEXT:     VPO Work-Region Collection
;CHECK-NEXT:     Lazy Branch Probability Analysis
;CHECK-NEXT:     Lazy Block Frequency Analysis
;CHECK-NEXT:     Optimization Remark Emitter
;CHECK-NEXT:     VPO Work-Region Information
;CHECK-NEXT:     VPO Paropt Loop Transform Function Pass
;CHECK-NEXT:     Dominator Tree Construction
;CHECK-NEXT:     Natural Loop Information
;CHECK-NEXT:     VPO CFGRestructuring
;CHECK-NEXT:     Scalar Evolution Analysis
;CHECK-NEXT:     Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:     Function Alias Analysis Results
;CHECK-NEXT:     VPO Work-Region Collection
;CHECK-NEXT:     Lazy Branch Probability Analysis
;CHECK-NEXT:     Lazy Block Frequency Analysis
;CHECK-NEXT:     Optimization Remark Emitter
;CHECK-NEXT:     VPO Work-Region Information
;CHECK-NEXT:     VPO Paropt Loop Collapse Function Pass
;CHECK-NEXT:     Dominator Tree Construction
;CHECK-NEXT:     Natural Loop Information
;CHECK-NEXT:     VPO CFGRestructuring
;CHECK-NEXT:     Canonicalize natural loops
;CHECK-NEXT:     Scalar Evolution Analysis
;CHECK-NEXT:     Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:     Function Alias Analysis Results
;CHECK-NEXT:     VPO Work-Region Collection
;CHECK-NEXT:     Lazy Branch Probability Analysis
;CHECK-NEXT:     Lazy Block Frequency Analysis
;CHECK-NEXT:     Optimization Remark Emitter
;CHECK-NEXT:     VPO Work-Region Information
;CHECK-NEXT:     VPO Paropt Prepare
;CHECK-NEXT: Pass Arguments:
;                Target Library Information
;                Target Transform Information
;                Target Pass Configuration
;                Xmain opt level pass
;                Assumption Cache Tracker
;                Profile summary info
;CHECK:      Optimization report options pass
;CHECK:      VPO paropt config pass
;              ModulePass Manager
;                Annotation2Metadata
;                Force set function attributes
;                Setup inlining report
;                Set attributes for callsites in [no]inline list
;                CallGraph Construction
;                Call Graph SCC Pass Manager
;                  Inliner for always_inline functions
;                VecClone
;CHECK:          FunctionPass Manager
;CHECK-NEXT:       VPO Restore Operands
;CHECK-NEXT:       Dominator Tree Construction
;CHECK-NEXT:       Natural Loop Information
;CHECK-NEXT:       VPO CFGRestructuring
;CHECK-NEXT:       Scalar Evolution Analysis
;CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:       Function Alias Analysis Results
;CHECK-NEXT:       VPO Work-Region Collection
;CHECK-NEXT:       Lazy Branch Probability Analysis
;CHECK-NEXT:       Lazy Block Frequency Analysis
;CHECK-NEXT:       Optimization Remark Emitter
;CHECK-NEXT:       VPO Work-Region Information
;CHECK-NEXT:       VPO Paropt Optimize Data Sharing
;CHECK-NEXT:     VPO Paropt Pass
;CHECK-NEXT:       FunctionPass Manager
;CHECK-NEXT:         Dominator Tree Construction
;CHECK-NEXT:         Natural Loop Information
;CHECK-NEXT:         Canonicalize natural loops
;CHECK-NEXT:         Scalar Evolution Analysis
;CHECK-NEXT:         Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:         Function Alias Analysis Results
;CHECK-NEXT:         VPO Work-Region Collection
;CHECK-NEXT:         Lazy Branch Probability Analysis
;CHECK-NEXT:         Lazy Block Frequency Analysis
;CHECK-NEXT:         Optimization Remark Emitter
;CHECK-NEXT:         VPO Work-Region Information
;CHECK-NEXT:     FunctionPass Manager
;CHECK-NEXT:       Dominator Tree Construction
;CHECK-NEXT:       Natural Loop Information
;CHECK-NEXT:       VPO CFGRestructuring
;CHECK-NEXT:     Function Outlining of Ordered Regions
;CHECK-NEXT:       FunctionPass Manager
;CHECK-NEXT:         Dominator Tree Construction
;CHECK-NEXT:         Natural Loop Information
;CHECK-NEXT:         Scalar Evolution Analysis
;CHECK-NEXT:         Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:         Function Alias Analysis Results
;CHECK-NEXT:         VPO Work-Region Collection
;CHECK-NEXT:         Lazy Branch Probability Analysis
;CHECK-NEXT:         Lazy Block Frequency Analysis
;CHECK-NEXT:         Optimization Remark Emitter
;CHECK-NEXT:         VPO Work-Region Information
;CHECK-NEXT:     FunctionPass Manager
;CHECK-NEXT:       Dominator Tree Construction
;CHECK-NEXT:       Natural Loop Information
;CHECK-NEXT:       Pragma omp simd if clause reduction to simdlen(1)
;CHECK-NEXT:       VPO CFGRestructuring
;CHECK-NEXT:       LCSSA Verifier
;CHECK-NEXT:       Loop-Closed SSA Form Pass
;CHECK-NEXT:       Scalar Evolution Analysis
;CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:       Function Alias Analysis Results
;CHECK-NEXT:       VPO Work-Region Collection
;CHECK-NEXT:       Lazy Branch Probability Analysis
;CHECK-NEXT:       Lazy Block Frequency Analysis
;CHECK-NEXT:       Optimization Remark Emitter
;CHECK-NEXT:       VPO Work-Region Information
;CHECK-NEXT:       Demanded bits analysis
;CHECK-NEXT:       Loop Access Analysis
;CHECK-NEXT:       Post-Dominator Tree Construction
;CHECK-NEXT:       Branch Probability Analysis
;CHECK-NEXT:       Block Frequency Analysis
;CHECK-NEXT:       VPlan Vectorizer
;CHECK-NEXT:     CallGraph Construction
;CHECK-NEXT:     Call Graph SCC Pass Manager
;CHECK-NEXT:       Inliner for always_inline functions
;CHECK-NEXT:     A No-Op Barrier Pass
;CHECK-NEXT:     FunctionPass Manager
;CHECK-NEXT:      VPO Directive Cleanup
;CHECK-NEXT:      VPO CFG simplification
;CHECK-NEXT:      Dominator Tree Construction
;CHECK-NEXT:      Natural Loop Information
;CHECK-NEXT:      VPO CFGRestructuring
;CHECK-NEXT:      Scalar Evolution Analysis
;CHECK-NEXT:      Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:      Function Alias Analysis Results
;CHECK-NEXT:      VPO Work-Region Collection
;CHECK-NEXT:      Lazy Branch Probability Analysis
;CHECK-NEXT:      Lazy Block Frequency Analysis
;CHECK-NEXT:      Optimization Remark Emitter
;CHECK-NEXT:      VPO Work-Region Information
;CHECK-NEXT:      VPO Paropt Guard Memory Motion
;CHECK-NEXT:      Dominator Tree Construction
;CHECK-NEXT:      Natural Loop Information
;CHECK-NEXT:      VPO CFGRestructuring
;CHECK-NEXT:      Scalar Evolution Analysis
;CHECK-NEXT:      Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:      Function Alias Analysis Results
;CHECK-NEXT:      VPO Work-Region Collection
;CHECK-NEXT:      Lazy Branch Probability Analysis
;CHECK-NEXT:      Lazy Block Frequency Analysis
;CHECK-NEXT:      Optimization Remark Emitter
;CHECK-NEXT:      VPO Work-Region Information
;CHECK-NEXT:      VPO Rename Operands
;CHECK-NEXT:     CallGraph Construction
;CHECK-NEXT:     Call Graph SCC Pass Manager
;CHECK-NEXT:       Inliner for always_inline functions
;CHECK-NEXT:       FunctionPass Manager
;                    Annotation Remarks
;                    Module Verifier
;CHECK:     Pass Arguments:
;CHECK-NEXT: Assumption Cache Tracker
;CHECK-NEXT: Target Library Information
;CHECK-NEXT: Xmain opt level pass
;CHECK-NEXT: Profile summary info
;CHECK-NEXT:   FunctionPass Manager
;CHECK-NEXT:     Dominator Tree Construction
;CHECK-NEXT:     Natural Loop Information
;CHECK-NEXT:     Canonicalize natural loops
;CHECK-NEXT:     Scalar Evolution Analysis
;CHECK-NEXT:     Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:     Function Alias Analysis Results
;CHECK-NEXT:     VPO Work-Region Collection
;CHECK-NEXT:     Lazy Branch Probability Analysis
;CHECK-NEXT:     Lazy Block Frequency Analysis
;CHECK-NEXT:     Optimization Remark Emitter
;CHECK-NEXT:     VPO Work-Region Information

define void @foo() {
  ret void
}
; end INTEL_CUSTOMIZATION
