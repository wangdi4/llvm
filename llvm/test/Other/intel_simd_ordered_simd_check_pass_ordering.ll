; RUN: opt -enable-new-pm=0 -O2 -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s --strict-whitespace
; RUN: opt -enable-new-pm=0 -O2 -loopopt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -disable-output -debug-pass=Structure < %s -o /dev/null 2>&1 | FileCheck %s --check-prefix=HIR --strict-whitespace

; #pragma omp ordered simd is processed by running the following passes for
; LLVM-IR and HIR path:
; - VPO CFG Restructuring : It should be called before Function Outlining of
;                           Ordered regions because the directives should be in
;                           their own single-predecessor single-successor basic
;                           blocks.
; - Function Outlining of Ordered Regions: Calls Code Extractor that removes the
;                                          simd ordered region from the code and
;                                          replaces it with a function call.
; - VPO CFG Restructuring : Call this pass again because Code Extractor might
;                           add new instructions in the same blocks as the
;                           directives.
; - Inliner for always_inline functions : Inlines the simd ordered code back to
;                                         the code.
; - A No-Op Barrier Pass : It should be called after the inliner in order to
;                          synchronize the passes.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @var_tripcount(i32* %ip, i32 %n, i32* %x) local_unnamed_addr {
;
; VPO CFG Restructuring should be executed before Function Outlinig of Ordered
; Regions because WRegion Graph might not work correctly.
; CHECK:              VPO CFGRestructuring
; CHECK-NEXT:      Function Outlining of Ordered Regions
;
; The following pass manager is executed because Function Outling of Ordered
; Regions is a module pass and it uses analyses which are function passes.
; CHECK-NEXT:        FunctionPass Manager
; CHECK-NEXT:          Dominator Tree Construction
; CHECK-NEXT:          Natural Loop Information
; CHECK-NEXT:          Scalar Evolution Analysis
; CHECK-NEXT:          Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:          Function Alias Analysis Results
; CHECK-NEXT:          VPO Work-Region Collection
; CHECK-NEXT:          Lazy Branch Probability Analysis
; CHECK-NEXT:          Lazy Block Frequency Analysis
; CHECK-NEXT:          Optimization Remark Emitter
; CHECK-NEXT:          VPO Work-Region Information
; CHECK-NEXT:      FunctionPass Manager
; CHECK-NEXT:        Dominator Tree Construction
; CHECK-NEXT:        Natural Loop Information
;
; Call again VPO CFG Restructuring to move the directives in SESE basic blcoks.
; CHECK-NEXT:        VPO CFGRestructuring
; CHECK-NEXT:        Replace known math operations with optimized library functions
; CHECK-NEXT:        LCSSA Verifier
; CHECK-NEXT:        Loop-Closed SSA Form Pass
; CHECK-NEXT:        Scalar Evolution Analysis
; CHECK-NEXT:        Basic Alias Analysis (stateless AA impl)
; CHECK-NEXT:        Function Alias Analysis Results
; CHECK-NEXT:        VPO Work-Region Collection
; CHECK-NEXT:        Lazy Branch Probability Analysis
; CHECK-NEXT:        Lazy Block Frequency Analysis
; CHECK-NEXT:        Optimization Remark Emitter
; CHECK-NEXT:        VPO Work-Region Information
; CHECK-NEXT:        Demanded bits analysis
; CHECK-NEXT:        Loop Access Analysis
; CHECK-NEXT:        VPlan Vectorizer
; CHECK-NEXT:        Dominator Tree Construction
; CHECK-NEXT:        Replace known math operations with optimized library functions
;
; The inliner puts back to the code the ordered region. Hence, it should be
; executed after VPlan.
; CHECK-NEXT:      CallGraph Construction
; CHECK-NEXT:      Call Graph SCC Pass Manager
; CHECK-NEXT:        Inliner for always_inline functions
;
; Barrier pass should be called after the inliner.
; CHECK-NEXT:      A No-Op Barrier Pass
; CHECK-NEXT:      FunctionPass Manager
;
; VPO Direcetive Cleanup removes the directives of #pragma omp ordered simd.
; CHECK-NEXT:        VPO Directive Cleanup
;
; Start processing the ordered region for HIR path
; HIR:             VPO CFGRestructuring
; HIR-NEXT:      Function Outlining of Ordered Regions
; HIR-NEXT:        FunctionPass Manager
; HIR-NEXT:          Dominator Tree Construction
; HIR-NEXT:          Natural Loop Information
; HIR-NEXT:          Scalar Evolution Analysis
; HIR-NEXT:          Basic Alias Analysis (stateless AA impl)
; HIR-NEXT:          Function Alias Analysis Results
; HIR-NEXT:          VPO Work-Region Collection
; HIR-NEXT:          Lazy Branch Probability Analysis
; HIR-NEXT:          Lazy Block Frequency Analysis
; HIR-NEXT:          Optimization Remark Emitter
; HIR-NEXT:          VPO Work-Region Information
; HIR-NEXT: FunctionPass Manager
; HIR-NOT:  Manager
; HIR:        VPlan HIR Vectorizer
; HIR-NOT: Manager
; HIR:        HIR Code Generation
;
; Call the inliner at the end of HIR path
; HIR:        CallGraph Construction
; HIR-NEXT:      Call Graph SCC Pass Manager
; HIR-NEXT:        Inliner for always_inline functions
;
; Call the barrier pass to synchronize pass manager after calling the inliner
; HIR-NEXT:      A No-Op Barrier Pass
; HIR-NEXT:      FunctionPass Manager
; HIR:             VPlan Vectorizer
; HIR:           CallGraph Construction
; HIR-NEXT:      Call Graph SCC Pass Manager
; HIR-NEXT:        Inliner for always_inline functions
; HIR-NEXT:      A No-Op Barrier Pass
; HIR-NEXT:      FunctionPass Manager
; HIR-NEXT:        VPO Directive Cleanup
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %latch ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %indvars.iv
  br label %ordered.entry

ordered.entry:
  %tok.ordered = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"() ]
  br label %ordered

ordered:
  %val = load i32, i32* %x
  store i32 %val, i32* %arrayidx, align 4
  br label %ordered.exit

ordered.exit:
  call void @llvm.directive.region.exit(token %tok.ordered) [ "DIR.OMP.END.ORDERED"() ]
  br label %latch

latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  br label %for.cond.cleanup

for.cond.cleanup:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
