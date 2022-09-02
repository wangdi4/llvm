; INTEL_CUSTOMIZATION
;RUN: opt -disable-verify -enable-new-pm=0 -O3 -paropt=31 -debug-pass=Structure -S -disable-output %s 2>&1 |  FileCheck %s

;CHECK:      Pass Arguments:
;            Target Transform Information
;            Xmain opt level pass                    ;INTEL
;CHECK:      Assumption Cache Tracker
;CHECK-NEXT: Target Library Information
;CHECK-NEXT: Profile summary info
;CHECK-NEXT: VPO paropt config pass
;CHECK-NEXT: Optimization report options pass        ;INTEL
;            Type-Based Alias Analysis
;            Scoped NoAlias Alias Analysis
;            Std Container Alias Analysis            ;INTEL
;              FunctionPass Manager
;                Subscript Intrinsic Lowering        ;INTEL
;                Instrument function entry/exit with calls to e.g. mcount() (pre inlining) ;INTEL
;CHECK:    Dominator Tree Construction
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
;                Lower 'expect' Intrinsics
;                Simplify the CFG
;                Dominator Tree Construction
;                SROA
;                Early CSE
; CHECK: Pass Arguments:
;            Target Library Information
;            Target Transform Information
;            Xmain opt level pass                    ;INTEL
;            Type-Based Alias Analysis
;            Scoped NoAlias Alias Analysis
;            Std Container Alias Analysis            ;INTEL
;            Assumption Cache Tracker
;            Profile summary info
;            Optimization report options pass        ;INTEL
;CHECK:      VPO paropt config pass                  ;INTEL
;              ModulePass Manager
;                Annotation2Metadata
;                Force set function attributes
;                Infer set function attributes
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Call-site splitting
;                Interprocedural Sparse Conditional Constant Propagation
;                  FunctionPass Manager
;                    Dominator Tree Construction
;                Called Value Propagation
;                Global Variable Optimizer
;                  FunctionPass Manager
;                    Dominator Tree Construction
;                    Natural Loop Information
;                    Post-Dominator Tree Construction
;                    Branch Probability Analysis
;                    Block Frequency Analysis
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Promote Memory to Register
;                Dead Argument Elimination
;                FunctionPass Manager
;                  Dominator Tree Construction
;CHECK:            Natural Loop Information
;CHECK:            VPO CFGRestructuring
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  Combine redundant instructions
;                  Lazy Value Information Analysis                ;INTEL
;                  Post-Dominator Tree Construction               ;INTEL
;                  Jump Threading                                 ;INTEL
;                  Simplify the CFG
;                  Dominator Tree Construction                    ;INTEL
;                  Natural Loop Information                       ;INTEL
;                  Scalar Evolution Analysis                      ;INTEL
;                  Handle Pragma Vector Aligned                   ;INTEL
;                CallGraph Construction
;                Globals Alias Analysis
;                Setup inlining report
;                Set attributes for callsites in [no]inline list  ;INTEL
;                Call Graph SCC Pass Manager
;                  Remove unused exception handling info
;                  Function Integration/Inlining
;CHECK:            FunctionPass Manager
;CHECK-NEXT:         Dominator Tree Construction
;CHECK-NEXT:         SROA
;CHECK-NEXT:         Simplify the CFG
;CHECK-NEXT:     A No-Op Barrier Pass
;CHECK-NEXT:     FunctionPass Manager
;CHECK-NEXT:       VPO Restore Operands
; INTEL_CUSTOMIZATION
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
;CHECK-NEXT:       VPO Paropt Shared Privatization Pass
;CHECK-NEXT:       Dominator Tree Construction
;CHECK-NEXT:       Natural Loop Information
;CHECK-NEXT:       Scalar Evolution Analysis
;CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:       Function Alias Analysis Results
;CHECK-NEXT:       VPO Work-Region Collection
;CHECK-NEXT:       Lazy Branch Probability Analysis
;CHECK-NEXT:       Lazy Block Frequency Analysis
;CHECK-NEXT:       Optimization Remark Emitter
;CHECK-NEXT:       VPO Work-Region Information
;CHECK-NEXT:       VPO Paropt Optimize Data Sharing
; end INTEL_CUSTOMIZATION
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
;CHECK-NEXT:       VPO CFG simplification
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
;CHECK-NEXT:       VPO Paropt Guard Memory Motion
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
;CHECK-NEXT:       VPO Rename Operands
;CHECK-NEXT:     CallGraph Construction
;CHECK-NEXT:     Call Graph SCC Pass Manager
;CHECK-NEXT:       Inliner for always_inline functions
;CHECK-NEXT:     Dead Global Elimination
;CHECK-NEXT:     CallGraph Construction
;CHECK-NEXT:     Call Graph SCC Pass Manager
;                  Promote 'by reference' arguments to scalars ;INTEL
;                  CallGraphSCC adaptor for SROA               ;INTEL
;                  OpenMP specific optimizations
;                  Deduce function attributes
;                  FunctionPass Manager
;                    TBAAPROP                                ;INTEL
;                    Dominator Tree Construction
;                    SROA
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Memory SSA
;                    Early CSE w/ MemorySSA
;                    Speculatively execute instructions if target has divergent branches
;                    Function Alias Analysis Results
;                    Lazy Value Information Analysis
;                    Post-Dominator Tree Construction        ;INTEL
;                    Jump Threading
;                    Value Propagation
;                    Simplify the CFG
;                    Dominator Tree Construction
;                    Combine pattern based expressions
;CHECK:              Natural Loop Information
;CHECK:              VPO CFGRestructuring
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    Combine redundant instructions
;                    Conditionally eliminate dead library calls
;                    Natural Loop Information
;                    Post-Dominator Tree Construction
;                    Branch Probability Analysis
;                    Block Frequency Analysis
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    PGOMemOPSize
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Natural Loop Information
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    Tail Call Elimination
;                    Simplify the CFG
;                    Reassociate expressions
;                    Dominator Tree Construction
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Memory SSA
;                    Natural Loop Information
;                    Canonicalize natural loops
;                    LCSSA Verifier
;                    Loop-Closed SSA Form Pass
;                    Scalar Evolution Analysis
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Loop Pass Manager
;                      Loop Invariant Code Motion
;                      Rotate Loops
;                      Loop Invariant Code Motion
;                      Unswitch loops
;                    Simplify the CFG
;                    Dominator Tree Construction
;CHECK:              Natural Loop Information
;CHECK:              VPO CFGRestructuring
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    Combine redundant instructions
;                    Canonicalize natural loops
;                    LCSSA Verifier
;                    Loop-Closed SSA Form Pass
;                    Scalar Evolution Analysis
;                    Loop Pass Manager
;                      Recognize loop idioms
;                      Induction Variable Simplification
;                      Delete dead loops
;                      Unroll loops
;                    SROA
;                    Function Alias Analysis Results
;                    MergedLoadStoreMotion
;                    Phi Values Analysis
;                    Function Alias Analysis Results
;                    Memory Dependence Analysis
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    Global Value Numbering
;                    Sparse Conditional Constant Propagation
;                    Demanded bits analysis
;                    Bit-Tracking Dead Code Elimination
;CHECK:              VPO CFGRestructuring
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    Combine redundant instructions
;                    Lazy Value Information Analysis
;                    Post-Dominator Tree Construction        ;INTEL
;                    Jump Threading
;                    Value Propagation
;                    Post-Dominator Tree Construction
;                    Aggressive Dead Code Elimination
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Memory SSA
;                    MemCpy Optimization
;                    Natural Loop Information
;                    Dead Store Elimination
;                    Canonicalize natural loops
;                    LCSSA Verifier
;                    Loop-Closed SSA Form Pass
;                    Function Alias Analysis Results
;                    Scalar Evolution Analysis
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Loop Pass Manager
;                      Loop Invariant Code Motion
;                    Simplify the CFG
;                    Dominator Tree Construction
;CHECK:              Natural Loop Information
;CHECK:              VPO CFGRestructuring
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    Combine redundant instructions
;                    Transform sin and cos calls             ;INTEL
;                Interprocedural Sparse Conditional Constant Propagation ;INTEL
;                  FunctionPass Manager                      ;INTEL
;                    Dominator Tree Construction             ;INTEL
;                CallGraph Construction                      ;INTEL
;                Propagate noalias to function arguments     ;INTEL
;                  FunctionPass Manager                      ;INTEL
;                    Dominator Tree Construction             ;INTEL
;                A No-Op Barrier Pass
;                FunctionPass Manager                        ;INTEL
;                  Dominator Tree Construction               ;INTEL
;                  Basic Alias Analysis (stateless AA impl)  ;INTEL
;                  Function Alias Analysis Results           ;INTEL
;                  StdContainerOpt                           ;INTEL
;                  Cleanup fake loads                        ;INTEL
;                Eliminate Available Externally Globals
;                CallGraph Construction
;                Deduce function attributes in RPO
;                Global Variable Optimizer
;                  FunctionPass Manager
;                    Dominator Tree Construction
;                    Natural Loop Information
;                    Post-Dominator Tree Construction
;                    Branch Probability Analysis
;                    Block Frequency Analysis
;                Dead Global Elimination
;                CallGraph Construction
; INTEL_CUSTOMIZATION
;                Andersen Interprocedural AA
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Global Variable Optimizer under -O2 and above
;                  Promote Memory to Register
;                  Post-Dominator Tree Construction
;                  Aggressive Dead Code Elimination
;                CallGraph Construction
; end INTEL_CUSTOMIZATION
;                Globals Alias Analysis
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Float to int
;                  Lower constant intrinsics
;                  Natural Loop Information
;                  Canonicalize natural loops
;                  LCSSA Verifier
;                  Loop-Closed SSA Form Pass
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Scalar Evolution Analysis
;                  Loop Pass Manager
;                    Rotate Loops
; INTEL_CUSTOMIZATION
;                VecClone
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Early CSE
;                  Natural Loop Information
;                  Canonicalize natural loops
;                  Lazy Value Information Analysis
;                  Lower SwitchInst's to branches
;                  Natural Loop Information
;                  LCSSA Verifier
;                  Loop-Closed SSA Form Pass
;                  VPO CFGRestructuring
;                Function Outlining of Ordered Regions
;                  FunctionPass Manager
;                    Dominator Tree Construction
;                    Natural Loop Information
;                    Scalar Evolution Analysis
;                    Basic Alias Analysis (stateless AA impl)
;                    Function Alias Analysis Results
;                    VPO Work-Region Collection
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                    Optimization Remark Emitter
;                    VPO Work-Region Information
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Natural Loop Information
;                  VPO CFGRestructuring
;                  Replace known math operations with optimized library functions
;                  LCSSA Verifier
;                  Loop-Closed SSA Form Pass
;                  Scalar Evolution Analysis
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  VPO Work-Region Collection
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  VPO Work-Region Information
;                  Demanded bits analysis
;                  Loop Access Analysis
;                  VPlan Vectorizer
;                  Dominator Tree Construction
;                  Replace known math operations with optimized library functions
;                CallGraph Construction
;                Call Graph SCC Pass Manager
;                  Inliner for always_inline functions
;                A No-Op Barrier Pass
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Natural Loop Information
;                  VPO CFGRestructuring
;                  VPO Directive Cleanup
; end INTEL_CUSTOMIZATION
;CHECK:            VPO Restore Operands
; INTEL_CUSTOMIZATION
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
;CHECK-NEXT:       VPO Paropt Shared Privatization Pass
;CHECK-NEXT:       Dominator Tree Construction
;CHECK-NEXT:       Natural Loop Information
;CHECK-NEXT:       Scalar Evolution Analysis
;CHECK-NEXT:       Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:       Function Alias Analysis Results
;CHECK-NEXT:       VPO Work-Region Collection
;CHECK-NEXT:       Lazy Branch Probability Analysis
;CHECK-NEXT:       Lazy Block Frequency Analysis
;CHECK-NEXT:       Optimization Remark Emitter
;CHECK-NEXT:       VPO Work-Region Information
;CHECK-NEXT:       VPO Paropt Optimize Data Sharing
; end INTEL_CUSTOMIZATION
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
;CHECK-NEXT:       VPO Directive Cleanup
;CHECK-NEXT:       VPO CFG simplification
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
;CHECK-NEXT:       VPO Paropt Guard Memory Motion
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
;CHECK-NEXT:       VPO Rename Operands
;CHECK-NEXT:     CallGraph Construction
;CHECK-NEXT:     Call Graph SCC Pass Manager
;CHECK-NEXT:       Inliner for always_inline functions
;CHECK-NEXT:     Dead Global Elimination
;CHECK-NEXT:     FunctionPass Manager
;                  Dominator Tree Construction
;                  Natural Loop Information
;                  Scalar Evolution Analysis
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Loop Access Analysis
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  Loop Distribution                         ;INTEL
;                  Canonicalize natural loops
;                  Scalar Evolution Analysis
;                  Basic Alias Analysis (stateless AA impl)  ;INTEL
;                  Function Alias Analysis Results
;                  Loop Access Analysis
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Loop Load Elimination
;CHECK:            VPO CFGRestructuring
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  Combine redundant instructions
;                  Simplify the CFG
;                  Dominator Tree Construction
;                  Natural Loop Information
;                  Scalar Evolution Analysis
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Demanded bits analysis
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  Inject TLI Mappings
;                  SLP Vectorizer
;                  Scalar Evolution Analysis                 ;INTEL
;                  Load Coalescing                           ;INTEL
;                  SROA                                      ;INTEL
;                  Function Alias Analysis Results           ;INTEL
;                  Optimize scalar/vector ops
;                  Early CSE                                 ;INTEL
;CHECK:            VPO CFGRestructuring
;                  Optimization Remark Emitter
;                  Combine redundant instructions
;                  Canonicalize natural loops
;                  LCSSA Verifier
;                  Loop-Closed SSA Form Pass
;                  Scalar Evolution Analysis
;                  Loop Pass Manager
;                    Unroll loops
;                  Unaligned Nontemporal Store Conversion    ;INTEL
;CHECK:            VPO CFGRestructuring
;                  Basic Alias Analysis (stateless AA impl)  ;INTEL
;                  Function Alias Analysis Results           ;INTEL
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  Combine redundant instructions
;                  Memory SSA
;                  Canonicalize natural loops
;                  LCSSA Verifier
;                  Loop-Closed SSA Form Pass
;                  Scalar Evolution Analysis
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Loop Pass Manager
;                    Loop Invariant Code Motion
;                  Optimization Remark Emitter
;                  Warn about non-applied transformations
;                  Alignment from assumptions
;                Strip Unused Function Prototypes
;                Dead Global Elimination
;                Merge Duplicate Global Constants
;                Call Graph Profile
;                  FunctionPass Manager
;                    Dominator Tree Construction
;                    Natural Loop Information
;                    Lazy Branch Probability Analysis
;                    Lazy Block Frequency Analysis
;                FunctionPass Manager
;                  Dominator Tree Construction
;                  Natural Loop Information
;                  Post-Dominator Tree Construction
;                  Branch Probability Analysis
;                  Block Frequency Analysis
;                  Canonicalize natural loops
;                  LCSSA Verifier
;                  Loop-Closed SSA Form Pass
;                  Basic Alias Analysis (stateless AA impl)
;                  Function Alias Analysis Results
;                  Scalar Evolution Analysis
;                  Block Frequency Analysis
;                  Loop Pass Manager
;                    Loop Sink
;                  Lazy Branch Probability Analysis
;                  Lazy Block Frequency Analysis
;                  Optimization Remark Emitter
;                  Remove redundant instructions
;                  Hoist/decompose integer division and remainder
;                  Simplify the CFG
;                  Annotation Remarks
;                Emit inlining report           ;INTEL
;            Pass Arguments:
;              FunctionPass Manager
;                Dominator Tree Construction
;            Pass Arguments:
;            Target Library Information
;            Target Transform Information       ;INTEL
;              FunctionPass Manager
;                Dominator Tree Construction
;                Natural Loop Information
;                Post-Dominator Tree Construction
;                Branch Probability Analysis
;                Block Frequency Analysis
;CHECK:      Pass Arguments:
;CHECK: Assumption Cache Tracker
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
; INTEL_CUSTOMIZATION
;            Pass Arguments:
;              FunctionPass Manager
;                Dominator Tree Construction
;            Pass Arguments:
;              FunctionPass Manager
;                Dominator Tree Construction
; end INTEL_CUSTOMIZATION
;            Pass Arguments:
;            Target Library Information
;            Target Transform Information      ;INTEL
;              FunctionPass Manager
;                Dominator Tree Construction
;                Natural Loop Information
;                Post-Dominator Tree Construction
;                Branch Probability Analysis
;                Block Frequency Analysis
; INTEL_CUSTOMIZATION
;            Pass Arguments:
;            Assumption Cache Tracker
;            Target Library Information
;            Xmain opt level pass
;            Profile summary info
;              FunctionPass Manager
;                Dominator Tree Construction
;                Natural Loop Information
;                Scalar Evolution Analysis
;                Basic Alias Analysis (stateless AA impl)
;                Function Alias Analysis Results
;                VPO Work-Region Collection
;                Lazy Branch Probability Analysis
;                Lazy Block Frequency Analysis
;                Optimization Remark Emitter
;                VPO Work-Region Information
; end INTEL_CUSTOMIZATION
;            Pass Arguments:
;CHECK:      Assumption Cache Tracker
;CHECK-NEXT: Target Library Information
;CHECK-NEXT: Xmain opt level pass                    ;INTEL
;CHECK-NEXT: Profile summary info
;CHECK-NEXT:   FunctionPass Manager
;CHECK-NEXT:     Dominator Tree Construction
;CHECK-NEXT:     Natural Loop Information
;CHECK:          Canonicalize natural loops
;CHECK-NEXT:     Scalar Evolution Analysis
;CHECK-NEXT:     Basic Alias Analysis (stateless AA impl)
;CHECK-NEXT:     Function Alias Analysis Results
;CHECK-NEXT:     VPO Work-Region Collection
;CHECK-NEXT:     Lazy Branch Probability Analysis
;CHECK-NEXT:     Lazy Block Frequency Analysis
;CHECK-NEXT:     Optimization Remark Emitter
;CHECK-NEXT:     VPO Work-Region Information
;            Pass Arguments:
;            Target Library Information
;              FunctionPass Manager
;                Dominator Tree Construction
;                Natural Loop Information
;                Lazy Branch Probability Analysis
;                Lazy Block Frequency Analysis

define void @foo() {
  ret void
}
; end INTEL_CUSTOMIZATION
