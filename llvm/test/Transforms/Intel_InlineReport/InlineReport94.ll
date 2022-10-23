; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that a call with a byval argument without alloca address space
; is not inlined.

; CHECK-CL-LABEL: define dso_local void @foo()
; CHECK-CL: tail call void @set_value
; CHECK-LABEL: COMPILE FUNC: foo
; CHECK: set_value{{.*}}Call has byval argument without alloca address space
; CHECK-MD-LABEL: define dso_local void @foo()
; CHECK-MD: tail call void @set_value

source_filename = "inline.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._my_struct = type { i32, [10 x <4 x float>], i32, [12 x i8] }

@s_struct = external dso_local local_unnamed_addr global %struct._my_struct, align 16

; Function Attrs: nofree noinline norecurse nosync nounwind uwtable
define dso_local void @set_value(%struct._my_struct addrspace(4)* nocapture readonly byval(%struct._my_struct) align 16 %p1) local_unnamed_addr #0 {
entry:
  %p = addrspacecast %struct._my_struct addrspace(4)* %p1 to %struct._my_struct *
  %scevgep = getelementptr %struct._my_struct, %struct._my_struct* %p, i64 0, i32 1, i64 0
  %scevgep8 = bitcast <4 x float>* %scevgep to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(160) bitcast (<4 x float>* getelementptr inbounds (%struct._my_struct, %struct._my_struct* @s_struct, i64 0, i32 1, i64 0) to i8*), i8* noundef nonnull align 16 dereferenceable(160) %scevgep8, i64 160, i1 false)
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nosync nounwind uwtable
define dso_local void @foo() local_unnamed_addr #2 {
entry:
  %s = alloca %struct._my_struct, align 16
  %0 = bitcast %struct._my_struct* %s to i8*
  %s1 = addrspacecast %struct._my_struct* %s to %struct._my_struct addrspace(4)*
  call void @llvm.lifetime.start.p0i8(i64 192, i8* nonnull %0) #4
  %arrayidx = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 0
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx, align 16, !tbaa !3
  %arrayidx.1 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 1
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.1, align 16, !tbaa !3
  %arrayidx.2 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 2
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.2, align 16, !tbaa !3
  %arrayidx.3 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 3
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.3, align 16, !tbaa !3
  %arrayidx.4 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 4
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.4, align 16, !tbaa !3
  %arrayidx.5 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 5
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.5, align 16, !tbaa !3
  %arrayidx.6 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 6
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.6, align 16, !tbaa !3
  %arrayidx.7 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 7
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.7, align 16, !tbaa !3
  %arrayidx.8 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 8
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.8, align 16, !tbaa !3
  %arrayidx.9 = getelementptr inbounds %struct._my_struct, %struct._my_struct* %s, i64 0, i32 1, i64 9
  store <4 x float> <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>, <4 x float>* %arrayidx.9, align 16, !tbaa !3
  tail call void @set_value(%struct._my_struct addrspace(4)* nonnull byval(%struct._my_struct) align 16 %s1)
  call void @llvm.lifetime.end.p0i8(i64 192, i8* nonnull %0) #4
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nofree noinline norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="128" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
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
