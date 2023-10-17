; Verify that we skip insertion of auto-vectorization directives when the
; target doesn't support vector registers. (CMPLRLLVM-52630)
; REQUIRES: asserts

; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert -disable-output -print-after=hir-vec-dir-insert -debug-only=parvec-transform < %s 2>&1 | FileCheck %s

; CHECK: Vec Directive Insertion disabled for target lacking vector registers.

; CHECK: *** IR Dump After HIRVecDirInsertPass ***
; CHECK: BEGIN REGION { }
; CHECK-NOT: %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NOT: @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]

; Following is the input HIR.  Test extracted from test.c at CMPLRLLVM-52630.
;
;         BEGIN REGION { }
;               + DO i1 = 0, 11, 1   <DO_MULTI_EXIT_LOOP>
;               |   %1 = (@g.e)[0][%0][i1];
;               |   if (%1 != 0)
;               |   {
;               |      goto for.cond.loopexit;
;               |   }
;               + END LOOP
;         END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@g.e = internal unnamed_addr constant [1 x [3 x i32]] [[3 x i32] [i32 1, i32 0, i32 0]], align 4

define i32 @g() #1 {
entry:
  %0 = load i32, ptr @a, align 4
  %idxprom.i = sext i32 %0 to i64
  br label %for.body.i

for.body.i:
  %indvars.iv.i = phi i64 [ 0, %entry ], [ %indvars.iv.next.i, %for.cond1.i ]
  %arrayidx3.i = getelementptr inbounds [3 x i32], ptr @g.e, i64 %idxprom.i, i64 %indvars.iv.i
  %1 = load i32, ptr %arrayidx3.i, align 4
  %tobool.not.i = icmp eq i32 %1, 0
  br i1 %tobool.not.i, label %for.cond1.i, label %for.cond.loopexit

for.cond1.i:
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, 12
  br i1 %exitcond.not.i, label %for.cond.loopexit, label %for.body.i

for.cond.loopexit:
  ret i32 %1
}

attributes #1 = { "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+x87,-aes,-avx,-avx2,-avx512bf16,-avx512bitalg,-avx512bw,-avx512cd,-avx512dq,-avx512er,-avx512f,-avx512fp16,-avx512ifma,-avx512pf,-avx512vbmi,-avx512vbmi2,-avx512vl,-avx512vnni,-avx512vp2intersect,-avx512vpopcntdq,-avxifma,-avxneconvert,-avxvnni,-avxvnniint16,-avxvnniint8,-f16c,-fma,-fma4,-gfni,-kl,-pclmul,-sha,-sha512,-sm3,-sm4,-sse2,-sse3,-sse4.1,-sse4.2,-sse4a,-ssse3,-vaes,-vpclmulqdq,-widekl,-xop" }
