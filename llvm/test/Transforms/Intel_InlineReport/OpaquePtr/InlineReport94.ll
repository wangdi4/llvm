; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that a call with a byval argument without alloca address space
; is not inlined.

; CHECK-CL-LABEL: define dso_local void @foo()
; CHECK-CL: tail call void @set_value
; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: set_value{{.*}}Call has byval argument without alloca address space
; CHECK-MD-LABEL: define dso_local void @foo()
; CHECK-MD: tail call void @set_value

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._my_struct = type { i32, [10 x <4 x float>], i32, [12 x i8] }

@s_struct = external dso_local local_unnamed_addr global %struct._my_struct, align 16

; Function Attrs: nofree noinline norecurse nosync nounwind uwtable
define dso_local void @set_value(ptr addrspace(4) nocapture readonly byval(%struct._my_struct) align 16 %p1) local_unnamed_addr #0 {
entry:
  %p = addrspacecast ptr addrspace(4) %p1 to ptr
  %scevgep = getelementptr %struct._my_struct, ptr %p, i64 0, i32 1, i64 0
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 16 dereferenceable(160) getelementptr inbounds (%struct._my_struct, ptr @s_struct, i64 0, i32 1, i64 0), ptr noundef nonnull align 16 dereferenceable(160) %scevgep, i64 160, i1 false)
  ret void
}

; Function Attrs: nofree nosync nounwind uwtable
define dso_local void @foo() local_unnamed_addr #1 {
entry:
  %s = alloca %struct._my_struct, align 16
  %s1 = addrspacecast ptr %s to ptr addrspace(4)
  call void @llvm.lifetime.start.p0(i64 192, ptr nonnull %s) #4
  %arrayidx = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 0
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx, align 16, !tbaa !3
  %arrayidx.1 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 1
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.1, align 16, !tbaa !3
  %arrayidx.2 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 2
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.2, align 16, !tbaa !3
  %arrayidx.3 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 3
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.3, align 16, !tbaa !3
  %arrayidx.4 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 4
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.4, align 16, !tbaa !3
  %arrayidx.5 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 5
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.5, align 16, !tbaa !3
  %arrayidx.6 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 6
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.6, align 16, !tbaa !3
  %arrayidx.7 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 7
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.7, align 16, !tbaa !3
  %arrayidx.8 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 8
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.8, align 16, !tbaa !3
  %arrayidx.9 = getelementptr inbounds %struct._my_struct, ptr %s, i64 0, i32 1, i64 9
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, ptr %arrayidx.9, align 16, !tbaa !3
  tail call void @set_value(ptr addrspace(4) nonnull byval(%struct._my_struct) align 16 %s1)
  call void @llvm.lifetime.end.p0(i64 192, ptr nonnull %s) #4
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nofree noinline norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="128" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
