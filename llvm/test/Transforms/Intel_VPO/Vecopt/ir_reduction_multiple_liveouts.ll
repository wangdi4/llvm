; Verify that VPlan bails out from scenarios where reduction is associated
; with multiple liveout instructions.

; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -debug-only=LoopVectorizationPlanner -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED

; CHECK: A reduction with more than one live-out instruction is not supported.
; OPTRPTMED: remark #15436: loop was not vectorized: A reduction with more than one live-out instruction is not supported.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %arr) {
entry:
  %red = alloca [4 x i16], align 16
  store i16 0, ptr %red, align 16
  %elem1 = getelementptr inbounds [4 x i16], ptr %red, i64 0, i64 1
  store i16 0, ptr %elem1, align 16
  %elem2 = getelementptr inbounds [4 x i16], ptr %red, i64 0, i64 2
  store i16 0, ptr %elem2, align 16
  %elem3 = getelementptr inbounds [4 x i16], ptr %red, i64 0, i64 3
  store i16 0, ptr %elem3, align 16
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %red, i16 0, i32 4) ]
  br label %loop.ph

loop.ph:
  %elem1.ld = load i16, ptr %elem1, align 4
  br label %loop

loop:
  %red.phi = phi i16 [ %elem1.ld, %loop.ph ], [ %red.next, %loop ]
  %iv = phi i32 [ 0, %loop.ph ], [ %iv.next, %loop ]
  %red.next = add i16 %red.phi, 42
  %iv.next = add nuw nsw i32 %iv, 1
  %iv.cond.not = icmp eq i32 %iv.next, 1024
  br i1 %iv.cond.not, label %loop.exit, label %loop

loop.exit:
  %red.phi.lcssa = phi i16 [ %red.phi, %loop ]
  %red.next.lcssa = phi i16 [ %red.next, %loop ]
  store i16 %red.next.lcssa, ptr %elem1, align 4
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  %res = add i16 %red.phi.lcssa, 3
  store i16 %res, ptr %arr, align 16
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
