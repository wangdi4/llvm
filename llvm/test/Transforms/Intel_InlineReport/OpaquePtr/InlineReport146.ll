; RUN: opt -passes='cgscc(inline)' -inline-report=0x1e807 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0x1e886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0x1e886 -S | opt -passes='inlinereportemitter' -inline-report=0x1e886 -S 2>&1 | FileCheck %s  --check-prefixes=CHECK,CHECK-MD

; Check demangling option for Windows style C++ demangling

; CHECK-CL-LABEL: DEAD STATIC FUNC: public: __cdecl two_ints::two_ints(int, int)
; CHECK-LABEL: COMPILE FUNC: void __cdecl init(class two_ints *const, int, class two_ints, class two_ints)
; CHECK: EXTERN: public: class two_ints __cdecl two_ints::operator+(class two_ints)
; CHECK-LABEL: COMPILE FUNC: class two_ints __cdecl mysum(class two_ints *const, int)
; CHECK: INLINE: public: __cdecl two_ints::two_ints(int, int) <<Callee has single callsite and local linkage>>
; CHECK: EXTERN: public: class two_ints __cdecl two_ints::operator+(class two_ints)
; CHECK-MD-LABEL: DEAD STATIC FUNC: public: __cdecl two_ints::two_ints(int, int)

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.29.30133"

%class.two_ints = type { i32, i32 }

$"??0two_ints@@QEAA@HH@Z" = comdat any

; Function Attrs: mustprogress uwtable
define dso_local void @"?init@@YAXQEAVtwo_ints@@HV1@1@Z"(ptr noundef %a, i32 noundef %n, i64 %value_one.coerce, i64 %value_two.coerce) local_unnamed_addr #0 {
entry:
  %value_one = alloca i64, align 8
  %tmpcast = bitcast ptr %value_one to ptr
  %ref.tmp = alloca %class.two_ints, align 4
  store i64 %value_one.coerce, ptr %value_one, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i = bitcast ptr %ref.tmp to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #5
  call void @"??Htwo_ints@@QEAA?AV0@V0@@Z"(ptr noundef nonnull align 4 dereferenceable(8) %tmpcast, ptr nonnull sret(%class.two_ints) align 4 %ref.tmp, i64 %value_two.coerce)
  %idxprom = zext i32 %i.0 to i64
  %i1 = getelementptr inbounds %class.two_ints, ptr %ref.tmp, i64 0, i32 0
  %i2 = getelementptr inbounds %class.two_ints, ptr %a, i64 %idxprom, i32 0
  %i3 = load i32, ptr %i1, align 4, !tbaa !4
  store i32 %i3, ptr %i2, align 4, !tbaa !4
  %i4 = getelementptr inbounds %class.two_ints, ptr %ref.tmp, i64 0, i32 1
  %i5 = getelementptr inbounds %class.two_ints, ptr %a, i64 %idxprom, i32 1
  %i6 = load i32, ptr %i4, align 4, !tbaa !4
  store i32 %i6, ptr %i5, align 4, !tbaa !4
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #5
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  ret void
}

declare dso_local void @"??Htwo_ints@@QEAA?AV0@V0@@Z"(ptr noundef nonnull align 4 dereferenceable(8), ptr sret(%class.two_ints) align 4, i64) local_unnamed_addr #1

; Function Attrs: mustprogress uwtable
define dso_local void @"?mysum@@YA?AVtwo_ints@@QEAV1@H@Z"(ptr noalias sret(%class.two_ints) align 4 %agg.result, ptr noundef %a, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %ref.tmp = alloca %class.two_ints, align 4
  %call = call noundef ptr @"??0two_ints@@QEAA@HH@Z"(ptr noundef nonnull align 4 dereferenceable(8) %agg.result, i32 noundef 0, i32 noundef 0)
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %i = bitcast ptr %ref.tmp to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %i) #5
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds %class.two_ints, ptr %a, i64 %idxprom
  %agg.tmp.sroa.0.0..sroa_cast = bitcast ptr %arrayidx to ptr
  %agg.tmp.sroa.0.0.copyload = load i64, ptr %agg.tmp.sroa.0.0..sroa_cast, align 4, !tbaa.struct !10
  call void @"??Htwo_ints@@QEAA?AV0@V0@@Z"(ptr noundef nonnull align 4 dereferenceable(8) %agg.result, ptr nonnull sret(%class.two_ints) align 4 %ref.tmp, i64 %agg.tmp.sroa.0.0.copyload)
  %i1 = getelementptr inbounds %class.two_ints, ptr %ref.tmp, i64 0, i32 0
  %i2 = getelementptr inbounds %class.two_ints, ptr %agg.result, i64 0, i32 0
  %i3 = load i32, ptr %i1, align 4, !tbaa !4
  store i32 %i3, ptr %i2, align 4, !tbaa !4
  %i4 = getelementptr inbounds %class.two_ints, ptr %ref.tmp, i64 0, i32 1
  %i5 = getelementptr inbounds %class.two_ints, ptr %agg.result, i64 0, i32 1
  %i6 = load i32, ptr %i4, align 4, !tbaa !4
  store i32 %i6, ptr %i5, align 4, !tbaa !4
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %i) #5
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !11

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local noundef ptr @"??0two_ints@@QEAA@HH@Z"(ptr noundef nonnull returned align 4 dereferenceable(8) %this, i32 noundef %int_one, i32 noundef %int_two) unnamed_addr #2 comdat align 2 {
entry:
  %_int_one = getelementptr inbounds %class.two_ints, ptr %this, i64 0, i32 0, !intel-tbaa !12
  store i32 %int_one, ptr %_int_one, align 4, !tbaa !12
  %_int_two = getelementptr inbounds %class.two_ints, ptr %this, i64 0, i32 1, !intel-tbaa !14
  store i32 %int_two, ptr %_int_two, align 4, !tbaa !14
  ret ptr %this
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #4

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = !{i64 0, i64 4, !4, i64 4, i64 4, !4}
!11 = distinct !{!11, !9}
!12 = !{!13, !5, i64 0}
!13 = !{!"struct@?AVtwo_ints@@", !5, i64 0, !5, i64 4}
!14 = !{!13, !5, i64 4}
