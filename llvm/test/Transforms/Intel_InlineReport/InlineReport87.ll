; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt -passes='cgscc(inline),function(lcssa,hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-cg,simplifycfg)' -inline-report=0xe807 -S 2>&1 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Check that the inlining report can replicate calls when inlining is run
; after unrolling.

; Notes on the options (in order) for this LIT test
;  -inline: Runs the inliner before Loop Opt
;  -lcssa -hir-ssa-deconstruction: Required before running Loop Opt
;  -hir-post-vec-complete-unroll: Performs the complete unrolling
;  -hir-cg: Needed to dump the LLVM IR after Loop Opt
;  -simplifycfg: Required to clean up the pre-Loop Opt transformation code
;  -inline: Runs the inliner again after Loop Opt. This similates the
;    running of the always inliner pass after Loop Opt in the legacy pass
;    manager
;  -inline-report=7: Produce the inlining report

; CHECK: COMPILE FUNC: foo1
; CHECK: COMPILE FUNC: foo
; CHECK: foo1 {{.*}}Callee has noinline attribute
; CHECK: foo1 {{.*}}Callee has noinline attribute
; CHECK: foo1 {{.*}}Callee has noinline attribute
; CHECK: foo1 {{.*}}Callee has noinline attribute
; CHECK: foo1 {{.*}}Callee has noinline attribute

@A = common global [1000 x [1000 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.6, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.inc.6 ]
  %0 = trunc i64 %indvars.iv20 to i32
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond.1.preheader ], [ %indvars.iv.next, %for.body.3 ]
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @A, i64 0, i64 %indvars.iv, i64 %indvars.iv20
  %1 = load i32, i32* %arrayidx5, align 4
  %add = add nsw i32 %1, %0
  %call = tail call i32 @foo1(i32 %add)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc.6, label %for.body.3

for.inc.6:                                        ; preds = %for.body.3
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 500
  br i1 %exitcond22, label %for.end.8, label %for.cond.1.preheader

for.end.8:                                        ; preds = %for.inc.6
  %add9 = add nsw i32 %add, 5
  store i32 %add9, i32* getelementptr inbounds ([1000 x [1000 x i32]], [1000 x [1000 x i32]]* @A, i64 0, i64 5, i64 5), align 4
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture)

define i32 @foo1(i32 %0) #0 {
  ret i32 %0
}

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture)

attributes #0 = { noinline }

; end INTEL_FEATURE_SW_ADVANCED

