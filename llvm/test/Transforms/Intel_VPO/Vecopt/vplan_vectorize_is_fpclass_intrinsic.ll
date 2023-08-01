; Test to verify that VPlan vectorizers correctly handle llvm.is.fpclass intrinsic.

; RUN: opt < %s -passes=vplan-vec -vplan-force-vf=2 -S | FileCheck %s --check-prefix=IR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -vplan-force-vf=2 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

define void @foo(ptr %d) {
entry:
  br label %loop.ph

loop.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop

loop:
  %i = phi i64 [ 0, %loop.ph ], [ %i7, %loop ]
  %i3 = load float, ptr null, align 4
  %i4 = getelementptr float, ptr %d, i64 %i
  %i5 = tail call i1 @llvm.is.fpclass.f32(float 0.0, i32 0)
; IR: call <2 x i1> @llvm.is.fpclass.v2f32(<2 x float> zeroinitializer, i32 0)
; HIR: %llvm.is.fpclass.v2f32 = @llvm.is.fpclass.v2f32(0.000000e+00,  0);
  %i6 = select i1 %i5, float 0.0, float 0.0
  store float %i6, ptr %i4, align 4
  %i7 = add i64 %i, 1
  %i8 = icmp eq i64 %i7, 100
  br i1 %i8, label %exit, label %loop

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %end

end:
  ret void
}

declare i1 @llvm.is.fpclass.f32(float, i32 immarg)
declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
