; Test case to verify that we bail out if we encounter an unexpected fence
; in LLVM IR.  The loopopt framework catches this earlier and will not
; present such a loop to the vectorizer for HIR.
;
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s
;
; CHECK: LOOP BEGIN
; CHECK-NEXT: remark #15593: simd loop was not vectorized: fence encountered
; CHECK: LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i64 @foo2(i64 %N, ptr %a) {
entry:
  %k = alloca i64, align 4
  store i64 0, i64* %k, align 4
  br label %reg.entry

reg.entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %k, i64 0, i64 1, i64 1) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 1, %reg.entry ], [ %indvars.iv.next, %latch ]
  %k.red = phi i64 [ 0, %reg.entry ], [ %k.next, %latch ]
  %g = getelementptr i64, ptr %a, i64 %indvars.iv
  %ld = load i64, ptr %g, align 8
  fence seq_cst
  %k.next = add nuw nsw i64 %k.red, %ld
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %latch

latch:
  %exitcond = icmp eq i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body

for.cond.cleanup.loopexit:
  %lcssa.k = phi i64 [%k.next, %latch]
  %lcssa.i = phi i64 [%indvars.iv.next, %latch]
  br label %for.cond.cleanup

for.cond.cleanup:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  %ret = add nuw nsw i64 %lcssa.k, %lcssa.i
  ret i64 %ret
}
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
