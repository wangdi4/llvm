; REQUIRES: asserts
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange" -debug-only=hir-loop-interchange < %s 2>&1 | FileCheck %s

; CHECK: Loopnest Interchanged: ( 1 2 ) --> ( 2 1 )

; Source code
; #define T 1000
; int a[T];
; int b[T][T];
; int c[T][T];
;
; void foo(int N) {
;   int i, j ;
;   for (i = 0; i < N - 1; i++) {
;     int zero = a[0];             // This is OK. Not preventing a perfect loop nest.
;     int sum = a[i + 1];
;     for (j = 0; j < N; j++) {
;       sum = sum + zero + b[j][i];
;     }
;     a[i + 1] = sum;
;   }
; }

; *** IR Dump Before HIR Loop Interchange ***
; Function: _Z3fooi
;
; <0>       BEGIN REGION { }
; <27>         + DO i1 = 0, sext.i32.i64((-1 + %N)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
; <28>         |   + DO i2 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; <4>          |   |   %sum.029 = (@a)[0][i1 + 1];
; <13>         |   |   %sum.029 = %sum.029 + %0  +  (@b)[0][i2][i1];
; <21>         |   |   (@a)[0][i1 + 1] = %sum.029;
; <28>         |   + END LOOP
; <27>         + END LOOP
; <0>       END REGION

; DDG's==
; 4:13 %sum.029 --> %sum.029 OUTPUT (*) (?)
; 4:13 %sum.029 --> %sum.029 FLOW (=) (0)
; 4:21 %sum.029 --> %sum.029 FLOW (=) (0)
; 13:13 %sum.029 --> %sum.029 FLOW (<= *) (? ?)
; 13:21 %sum.029 --> %sum.029 FLOW (*) (?)
; 13:13 %sum.029 --> %sum.029 ANTI (= =) (0 0)
; 21:13 %sum.029 --> %sum.029 ANTI (*) (?)
; 4:21 (@a)[0][i1 + 1] --> (@a)[0][i1 + 1] ANTI (=) (0)
;
; *** IR Dump After HIR Loop Interchange ***
; Function: _Z3fooi
;
; <0>       BEGIN REGION { modified }
; <27>            + DO i1 = 0, sext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1000>
; <28>            |   + DO i2 = 0, sext.i32.i64((-1 + %N)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 999>
; <4>             |   |   %sum.029 = (@a)[0][i2 + 1];
; <13>            |   |   %sum.029 = %sum.029 + %0  +  (@b)[0][i1][i2];
; <21>            |   |   (@a)[0][i2 + 1] = %sum.029;
; <28>            |   + END LOOP
; <27>            + END LOOP
; <0>       END REGION

;Module Before HIR; ModuleID = 'valid-sink-2.cpp'
source_filename = "valid-sink-2.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@a = dso_local local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_valid_sink_2.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z3fooi(i32 %N) local_unnamed_addr #3 {
entry:
  %cmp30 = icmp sgt i32 %N, 1
  br i1 %cmp30, label %for.body.lr.ph, label %for.end15

for.body.lr.ph:                                   ; preds = %entry
  %0 = load i32, ptr @a, align 16, !tbaa !2
  %1 = add i32 %N, -1
  %wide.trip.count = sext i32 %N to i64
  %wide.trip.count34 = sext i32 %1 to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body.lr.ph, %for.end
  %indvars.iv32 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next33, %for.end ]
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @a, i64 0, i64 %indvars.iv.next33
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !2
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.body3 ]
  %sum.029 = phi i32 [ %2, %for.body3.lr.ph ], [ %add9, %for.body3 ]
  %add4 = add nsw i32 %sum.029, %0
  %arrayidx8 = getelementptr inbounds [1000 x [1000 x i32]], ptr @b, i64 0, i64 %indvars.iv, i64 %indvars.iv32
  %3 = load i32, ptr %arrayidx8, align 4, !tbaa !7
  %add9 = add nsw i32 %add4, %3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add9.lcssa = phi i32 [ %add9, %for.body3 ]
  store i32 %add9.lcssa, ptr %arrayidx, align 4, !tbaa !2
  %exitcond35 = icmp eq i64 %indvars.iv.next33, %wide.trip.count34
  br i1 %exitcond35, label %for.end15.loopexit, label %for.body3.lr.ph

for.end15.loopexit:                               ; preds = %for.end
  br label %for.end15

for.end15:                                        ; preds = %for.end15.loopexit, %entry
  ret void
}

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_valid_sink_2.cpp() #4 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZSt8__ioinit)
  %0 = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZSt8__ioinit, ptr nonnull @__dso_handle) #2
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ee02616b2c2a59ccaa0f22d40bfa39737ab8afa7) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm a9e599c763e0d8713d293019a2501a0be3175608)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA1000_A1000_i", !3, i64 0}
