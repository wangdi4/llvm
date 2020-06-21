;RUN: llc -mtriple=x86_64-pc-linux -mattr=+avx512f -mcpu=skylake-avx512  --enable-intel-advanced-opts -O3  < %s | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.B = type { i8 }

@b = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: uwtable
define dso_local void @test_align_non_fall_through_bb() local_unnamed_addr #0 {
; CHECK-LABEL: test_align_non_fall_through_bb:
; CHECK:       .p2align 5, 0x90
; CHECK:       .p2align 5, 0x90
; CHECK:       .p2align 5, 0x90

entry:
  %c = alloca %class.B, align 1
  %call = tail call i32 @_Z15number_of_testsv()
  %0 = getelementptr inbounds %class.B, %class.B* %c, i64 0, i32 0
  call void @llvm.lifetime.start.p0i8(i64 1, i8* nonnull %0) #5
  %call1 = call nonnull align 8 dereferenceable(8) double* @_ZN1BixEi(%class.B* nonnull %c, i32 0)
  %1 = load i32, i32* @b, align 4, !tbaa !2
  %call2 = call nonnull align 8 dereferenceable(8) double* @_ZN1BixEi(%class.B* nonnull %c, i32 %1)
  %cmp39 = icmp sgt i32 %call, 0
  br i1 %cmp39, label %while.body.lr.ph, label %while.cond13.preheader

while.body.lr.ph:                                 ; preds = %entry
  %sub.ptr.lhs.cast = ptrtoint double* %call2 to i64
  %sub.ptr.rhs.cast = ptrtoint double* %call1 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast
  %sub.ptr.div = ashr exact i64 %sub.ptr.sub, 3
  %2 = call { i64, i1 } @llvm.umul.with.overflow.i64(i64 %sub.ptr.div, i64 8)
  %3 = extractvalue { i64, i1 } %2, 1
  %4 = extractvalue { i64, i1 } %2, 0
  %5 = select i1 %3, i64 -1, i64 %4
  br label %while.body

while.cond13.preheader:                           ; preds = %if.end12, %entry
  %tobool36 = icmp eq i32 %call, 0
  br i1 %tobool36, label %while.end16, label %while.body15

while.body:                                       ; preds = %while.body.lr.ph, %if.end12
  %dec40.in = phi i32 [ %call, %while.body.lr.ph ], [ %dec40, %if.end12 ]
  %dec40 = add nsw i32 %dec40.in, -1
  %call3 = call noalias nonnull i8* @_Znam(i64 %5) #6
  %6 = bitcast i8* %call3 to double*
  %7 = load i32, i32* @b, align 4, !tbaa !2
  %cmp4 = icmp sgt i32 %7, 0
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  call void @_Z4copyIPdS0_EvT_S1_T0_(double* nonnull %call1, double* nonnull %call2, double* nonnull %6)
  br label %if.end12

if.else:                                          ; preds = %while.body
  %cmp5 = icmp eq i32 %7, 0
  br i1 %cmp5, label %if.then6, label %if.else11

if.then6:                                         ; preds = %if.else
  %add.ptr = getelementptr inbounds double, double* %6, i64 %sub.ptr.div
  call void @_Z6uniqueIPdEvT_S1_(double* nonnull %6, double* nonnull %add.ptr)
  br label %if.end12

if.else11:                                        ; preds = %if.else
  call void @_Z13multiset_testPdS_i(double* nonnull %call1, double* nonnull %call2, i32 %dec40)
  br label %if.end12

if.end12:                                         ; preds = %if.then6, %if.else11, %if.then
  %cmp = icmp sgt i32 %dec40.in, 1
  br i1 %cmp, label %while.body, label %while.cond13.preheader

while.body15:                                     ; preds = %while.cond13.preheader, %while.body15
  %dec1437.in = phi i32 [ %dec1437, %while.body15 ], [ %call, %while.cond13.preheader ]
  %dec1437 = add nsw i32 %dec1437.in, -1
  call void @_Z13multiset_testPdS_i(double* nonnull %call1, double* nonnull %call2, i32 %dec1437)
  %tobool = icmp eq i32 %dec1437, 0
  br i1 %tobool, label %while.end16, label %while.body15

while.end16:                                      ; preds = %while.body15, %while.cond13.preheader
  call void @llvm.lifetime.end.p0i8(i64 1, i8* nonnull %0) #5
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i32 @_Z15number_of_testsv() local_unnamed_addr #2

declare dso_local nonnull align 8 dereferenceable(8) double* @_ZN1BixEi(%class.B*, i32) local_unnamed_addr #2

; Function Attrs: nounwind readnone speculatable willreturn
declare { i64, i1 } @llvm.umul.with.overflow.i64(i64, i64) #3

; Function Attrs: nobuiltin nofree allocsize(0)
declare dso_local noalias nonnull i8* @_Znam(i64) local_unnamed_addr #4

declare dso_local void @_Z4copyIPdS0_EvT_S1_T0_(double*, double*, double*) local_unnamed_addr #2

declare dso_local void @_Z6uniqueIPdEvT_S1_(double*, double*) local_unnamed_addr #2

declare dso_local void @_Z13multiset_testPdS_i(double*, double*, i32) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone speculatable willreturn }
attributes #4 = { nobuiltin nofree allocsize(0) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }
attributes #6 = { builtin allocsize(0) }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
