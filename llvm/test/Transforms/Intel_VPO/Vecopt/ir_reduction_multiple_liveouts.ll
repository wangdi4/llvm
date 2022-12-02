; Verify that VPlan bails out from scenarios where reduction is associated
; with multiple liveout instructions.

; RUN: opt -vplan-vec -vplan-force-vf=2 -debug-only=LoopVectorizationPlanner -disable-output < %s 2>&1 | FileCheck %s

; CHECK: LVP: Reduction with multiple liveout instructions is not supported.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo([4 x i16]* %arr) {
entry:
  %red = alloca [4 x i16], align 16
  %elem0 = getelementptr inbounds [4 x i16], [4 x i16]* %red, i64 0, i64 0
  store i16 0, i16* %elem0, align 16
  %elem1 = getelementptr inbounds [4 x i16], [4 x i16]* %red, i64 0, i64 1
  store i16 0, i16* %elem1, align 16
  %elem2 = getelementptr inbounds [4 x i16], [4 x i16]* %red, i64 0, i64 2
  store i16 0, i16* %elem2, align 16
  %elem3 = getelementptr inbounds [4 x i16], [4 x i16]* %red, i64 0, i64 3
  store i16 0, i16* %elem3, align 16
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"([4 x i16]* %red, i16 0, i32 4) ]
  br label %loop.ph

loop.ph:
  %elem1.ld = load i16, i16* %elem1, align 4
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
  store i16 %red.next.lcssa, i16* %elem1, align 4
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  %res = add i16 %red.phi.lcssa, 3
  %st.elem = getelementptr inbounds [4 x i16], [4 x i16]* %arr, i64 0, i64 0
  store i16 %res, i16* %st.elem, align 16
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
