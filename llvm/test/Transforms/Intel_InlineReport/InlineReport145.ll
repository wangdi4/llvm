; RUN: opt -passes='cgscc(inline)' -inline-report=0x1e807 -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -passes='inlinereportsetup' -inline-report=0x1e886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0x1e886 -S | opt -passes='inlinereportemitter' -inline-report=0x1e886 -S 2>&1 | FileCheck %s  --check-prefixes=CHECK,CHECK-MD

; Check demangling option for Linux style C++ demangling

; CHECK-CL-LABEL: DEAD STATIC FUNC: two_ints::two_ints(int, int)
; CHECK-LABEL: COMPILE FUNC: init(two_ints*, int, two_ints, two_ints)
; CHECK: EXTERN: two_ints::operator+(two_ints)
; CHECK-LABEL: COMPILE FUNC: mysum(two_ints*, int)
; CHECK: INLINE: two_ints::two_ints(int, int) <<Callee has single callsite and local linkage>>
; CHECK: EXTERN: two_ints::operator+(two_ints)
; CHECK-MD-LABEL: DEAD STATIC FUNC: two_ints::two_ints(int, int)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.two_ints = type { i32, i32 }

$_ZN8two_intsC2Eii = comdat any

; Function Attrs: mustprogress uwtable
define dso_local void @_Z4initP8two_intsiS_S_(%class.two_ints* noundef %a, i32 noundef %n, i64 %value_one.coerce, i64 %value_two.coerce) local_unnamed_addr #0 {
entry:
  %value_one = alloca i64, align 8
  %tmpcast = bitcast i64* %value_one to %class.two_ints*
  store i64 %value_one.coerce, i64* %value_one, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call = call i64 @_ZN8two_intsplES_(%class.two_ints* noundef nonnull align 4 dereferenceable(8) %tmpcast, i64 %value_two.coerce)
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds %class.two_ints, %class.two_ints* %a, i64 %idxprom
  %ref.tmp.sroa.0.0..sroa_cast1 = bitcast %class.two_ints* %arrayidx to i64*
  store i64 %call, i64* %ref.tmp.sroa.0.0..sroa_cast1, align 4, !tbaa.struct !3
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  ret void
}

declare dso_local i64 @_ZN8two_intsplES_(%class.two_ints* noundef nonnull align 4 dereferenceable(8), i64) local_unnamed_addr #1

; Function Attrs: mustprogress uwtable
define dso_local i64 @_Z5mysumP8two_intsi(%class.two_ints* noundef %a, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %retval = alloca i64, align 8
  %tmpcast = bitcast i64* %retval to %class.two_ints*
  call void @_ZN8two_intsC2Eii(%class.two_ints* noundef nonnull align 4 dereferenceable(8) %tmpcast, i32 noundef 0, i32 noundef 0)
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds %class.two_ints, %class.two_ints* %a, i64 %idxprom
  %agg.tmp.sroa.0.0..sroa_cast = bitcast %class.two_ints* %arrayidx to i64*
  %agg.tmp.sroa.0.0.copyload = load i64, i64* %agg.tmp.sroa.0.0..sroa_cast, align 4, !tbaa.struct !3
  %call = call i64 @_ZN8two_intsplES_(%class.two_ints* noundef nonnull align 4 dereferenceable(8) %tmpcast, i64 %agg.tmp.sroa.0.0.copyload)
  store i64 %call, i64* %retval, align 8, !tbaa.struct !3
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !10

for.end:                                          ; preds = %for.cond
  %0 = load i64, i64* %retval, align 8
  ret i64 %0
}

; Function Attrs: nounwind uwtable
define linkonce_odr dso_local void @_ZN8two_intsC2Eii(%class.two_ints* noundef nonnull align 4 dereferenceable(8) %this, i32 noundef %int_one, i32 noundef %int_two) unnamed_addr #2 comdat align 2 {
entry:
  %_int_one = getelementptr inbounds %class.two_ints, %class.two_ints* %this, i64 0, i32 0, !intel-tbaa !11
  store i32 %int_one, i32* %_int_one, align 4, !tbaa !11
  %_int_two = getelementptr inbounds %class.two_ints, %class.two_ints* %this, i64 0, i32 1, !intel-tbaa !13
  store i32 %int_two, i32* %_int_two, align 4, !tbaa !13
  ret void
}

attributes #0 = { mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{i64 0, i64 4, !4, i64 4, i64 4, !4}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = !{!12, !5, i64 0}
!12 = !{!"struct@_ZTS8two_ints", !5, i64 0, !5, i64 4}
!13 = !{!12, !5, i64 4}
