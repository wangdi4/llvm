; RUN: opt -opaque-pointers < %s -passes='module(call-tree-clone),print<inline-report>' -enable-intel-advanced-opts=1 -mtriple=i686-- -march=core-avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -inline-report=0xe807 -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -march=core-avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -inline-report=0xe886 | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

; Check that multiversioned callsites appear in the inlining report, when
; inlining precedes call tree cloning.

; CHECK-CL-LABEL: COMPILE FUNC: get_ref{{ *$}}
; CHECK-CL: get_ref|8.8 {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|16.16 {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|16.8 {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|8.16 {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|20._ {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|16._ {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|12._ {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|8._ {{.*}}Multiversioned callsite
; CHECK-CL: get_ref|_._ {{.*}}Multiversioned callsite

; CHECK-CL-LABEL: COMPILE FUNC: main{{ *$}}
; CHECK-CL: get_ref|16.16 {{.*}}Newly created callsite
; CHECK-CL: get_ref|8.8 {{.*}}Newly created callsite
; CHECK-CL: get_ref|8.16 {{.*}}Newly created callsite
; CHECK-CL: get_ref|16.8 {{.*}}Newly created callsite
; CHECK-CL: get_ref {{.*}}Newly created callsite
; CHECK-CL: EXTERN: printf{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: main{{ *$}}
; CHECK-MD: get_ref|16.16 {{.*}}Not tested for inlining
; CHECK-MD: get_ref|8.8 {{.*}}Not tested for inlining
; CHECK-MD: get_ref|8.16 {{.*}}Not tested for inlining
; CHECK-MD: get_ref|16.8 {{.*}}Not tested for inlining
; CHECK-MD: get_ref {{.*}}Not tested for inlining
; CHECK-MD: EXTERN: printf{{ *$}}

; CHECK-MD-LABEL: COMPILE FUNC: get_ref{{ *$}}
; CHECK-MD: get_ref|8.8 {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|16.16 {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|16.8 {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|8.16 {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|20._ {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|16._ {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|12._ {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|8._ {{.*}}Multiversioned callsite
; CHECK-MD: get_ref|_._ {{.*}}Multiversioned callsite

; CHECK-LABEL: COMPILE FUNC: get_ref|16.16{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|8.8{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|8.16{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|16.8{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|_._{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|20._{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|16._{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|12._{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|8._{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|_.20{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|_.16{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|_.12{{ *$}}

; CHECK-LABEL: COMPILE FUNC: get_ref|_.8{{ *$}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"RV: %p\0A\00", align 1
@dst = internal global [1000 x i8] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
bb:
  tail call fastcc void @get_ref(i32 16, i32 16)
  tail call fastcc void @get_ref(i32 8, i32 8)
  tail call fastcc void @get_ref(i32 8, i32 16)
  tail call fastcc void @get_ref(i32 16, i32 8)
  br label %bb1

bb1:                                              ; preds = %bb1, %bb
  %i = phi i32 [ 0, %bb ], [ %i3, %bb1 ]
  %i2 = sub nuw nsw i32 100, %i
  tail call fastcc void @get_ref(i32 %i, i32 %i2)
  %i3 = add nuw nsw i32 %i, 1
  %i4 = icmp eq i32 %i3, 100
  br i1 %i4, label %bb5, label %bb1

bb5:                                              ; preds = %bb1
  %i6 = tail call i32 (ptr, ...) @printf(ptr @.str, ptr @dst)
  ret i32 0
}

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @get_ref(i32 %arg, i32 %arg1) unnamed_addr #1 {
bb:
  %i = icmp sgt i32 %arg1, 0
  br i1 %i, label %bb2, label %bb32

bb2:                                              ; preds = %bb
  %i3 = icmp sgt i32 %arg, 0
  %i4 = sext i32 1 to i64
  %i5 = sext i32 %arg to i64
  br label %bb6

bb6:                                              ; preds = %bb11, %bb2
  %i7 = phi i32 [ 0, %bb2 ], [ %i15, %bb11 ]
  %i8 = phi ptr [ @dst, %bb2 ], [ %i12, %bb11 ]
  %i9 = phi ptr [ null, %bb2 ], [ %i13, %bb11 ]
  %i10 = phi ptr [ null, %bb2 ], [ %i14, %bb11 ]
  br i1 %i3, label %bb17, label %bb11

bb11:                                             ; preds = %bb17, %bb6
  %i12 = getelementptr inbounds i8, ptr %i8, i64 %i4, !intel-tbaa !3
  %i13 = getelementptr inbounds i8, ptr %i9, i64 2, !intel-tbaa !3
  %i14 = getelementptr inbounds i8, ptr %i10, i64 2, !intel-tbaa !3
  %i15 = add nuw nsw i32 %i7, 1
  %i16 = icmp eq i32 %i15, %arg1
  br i1 %i16, label %bb32, label %bb6

bb17:                                             ; preds = %bb17, %bb6
  %i18 = phi i64 [ %i30, %bb17 ], [ 0, %bb6 ]
  %i19 = getelementptr inbounds i8, ptr %i9, i64 %i18
  %i20 = load i8, ptr %i19, align 1, !tbaa !3
  %i21 = zext i8 %i20 to i32
  %i22 = getelementptr inbounds i8, ptr %i10, i64 %i18
  %i23 = load i8, ptr %i22, align 1, !tbaa !3
  %i24 = zext i8 %i23 to i32
  %i25 = add nuw nsw i32 %i21, 1
  %i26 = add nuw nsw i32 %i25, %i24
  %i27 = lshr i32 %i26, 1
  %i28 = trunc i32 %i27 to i8
  %i29 = getelementptr inbounds i8, ptr %i8, i64 %i18
  store i8 %i28, ptr %i29, align 1, !tbaa !3
  %i30 = add nuw nsw i64 %i18, 1
  %i31 = icmp eq i64 %i30, %i5
  br i1 %i31, label %bb11, label %bb17

bb32:                                             ; preds = %bb11, %bb
  br label %bb33

bb33:                                             ; preds = %bb32
  ret void
}

; Function Attrs: nounwind
declare dso_local i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #2

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e5f4f662867240aabc27bc9491b73d220049214d) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 55a65d6b450a8fef4eabd8f7a3a3850d67223e64)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
