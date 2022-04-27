; RUN: opt -passes="dpcpp-kernel-analysis,dpcpp-kernel-wg-loop-bound" %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="dpcpp-kernel-analysis,dpcpp-kernel-wg-loop-bound" %s -debug -disable-output 2>&1 | FileCheck %s
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound %s -debug -disable-output 2>&1 | FileCheck %s

; The test cheks that load instruction doesn't block wg boundaries adjustment
;
; Original source:
;
;struct IdxStT {
;  unsigned int Idx;
;};
;
;__kernel void load_before_early_exit(__global int *Buf, struct IdxStT Idx) {
;  __global int *AdjustedPtr = Buf + (size_t)Idx.Idx;
;  if (get_local_id(0) > 42)
;    *AdjustedPtr = 42;
;}

; CHECK: WGLoopBoundaries
; CHECK: found 1 early exit boundaries
; CHECK: Dim=0, Contains=F, IsGID=F, IsSigned=F, IsUpperBound=F
; CHECK-SAME: i64 42

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.IdxStT = type { i32 }

; Function Attrs: convergent nounwind uwtable
define dso_local void @load_before_early_exit(i32*, %struct.IdxStT* byval(%struct.IdxStT) align 4) #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 {
  %3 = getelementptr inbounds %struct.IdxStT, %struct.IdxStT* %1, i32 0, i32 0
  %4 = load i32, i32* %3, align 4, !tbaa !8
  %5 = zext i32 %4 to i64
  %6 = getelementptr inbounds i32, i32* %0, i64 %5
  %7 = call i64 @_Z12get_local_idj(i32 0) #3
  %8 = icmp ugt i64 %7, 42
  br i1 %8, label %9, label %10

9:                                                ; preds = %2
  store i32 42, i32* %6, align 4, !tbaa !13
  br label %10

10:                                               ; preds = %9, %2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent nounwind readnone
declare dso_local i64 @_Z12get_local_idj(i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { convergent nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone }

!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{!"clang version 9.0.0 "}
!3 = !{void (i32*, %struct.IdxStT*)* @load_before_early_exit}
!4 = !{i32 1, i32 0}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"struct IdxStT"}
!7 = !{!"", !""}
!8 = !{!9, !10, i64 0}
!9 = !{!"IdxStT", !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!10, !10, i64 0}

; DEBUGIFY-COUNT-25: Instruction with empty DebugLoc in function WG.boundaries.
; DEBUGIFY-COUNT-1: Missing line
; DEBUGIFY-NOT: WARNING
