; Checks that the Multi-Versioning (MV) transformation creates MV function as expected for -march=core-avx2 tuning

; RUN: opt < %s -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -march=core-avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s
; RUN: opt < %s -call-tree-clone -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s

; Checks the multi-version (MV) function on get_ref() has proper code generation


;*** IR Dump After IP Cloning ***; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"RV: %p\0A\00", align 1
@dst = internal global [1000 x i8] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  tail call fastcc void @get_ref(i32 16, i32 16)
  tail call fastcc void @get_ref(i32 8, i32 8)
  tail call fastcc void @get_ref(i32 8, i32 16)
  tail call fastcc void @get_ref(i32 16, i32 8)
  br label %1

; <label>:1:                                      ; preds = %1, %0
  %2 = phi i32 [ 0, %0 ], [ %4, %1 ]
  %3 = sub nuw nsw i32 100, %2
  tail call fastcc void @get_ref(i32 %2, i32 %3)
  %4 = add nuw nsw i32 %2, 1
  %5 = icmp eq i32 %4, 100
  br i1 %5, label %6, label %1

; <label>:6:                                      ; preds = %1
  %7 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0))
  ret i32 0
}

; Checks the calls to get_ref are properly generated:
; CHECK:       "get_ref|16.16"
; CHECK-NEXT:  "get_ref|8.8"
; CHECK-NEXT:  "get_ref|8.16"
; CHECK-NEXT:  "get_ref|16.8"
; CHECK:       tail call fastcc void @get_ref(i32 %2, i32 %3)

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @get_ref(i32, i32) unnamed_addr #1 {
  %3 = icmp sgt i32 %1, 0
  br i1 %3, label %4, label %34

; <label>:4:                                      ; preds = %2
  %5 = icmp sgt i32 %0, 0
  %6 = sext i32 1 to i64
  %7 = sext i32 %0 to i64
  br label %8

; <label>:8:                                      ; preds = %13, %4
  %9 = phi i32 [ 0, %4 ], [ %17, %13 ]
  %10 = phi i8* [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %4 ], [ %14, %13 ]
  %11 = phi i8* [ null, %4 ], [ %15, %13 ]
  %12 = phi i8* [ null, %4 ], [ %16, %13 ]
  br i1 %5, label %19, label %13

; <label>:13:                                     ; preds = %19, %8
  %14 = getelementptr inbounds i8, i8* %10, i64 %6, !intel-tbaa !3
  %15 = getelementptr inbounds i8, i8* %11, i64 2, !intel-tbaa !3
  %16 = getelementptr inbounds i8, i8* %12, i64 2, !intel-tbaa !3
  %17 = add nuw nsw i32 %9, 1
  %18 = icmp eq i32 %17, %1
  br i1 %18, label %34, label %8

; <label>:19:                                     ; preds = %19, %8
  %20 = phi i64 [ %32, %19 ], [ 0, %8 ]
  %21 = getelementptr inbounds i8, i8* %11, i64 %20
  %22 = load i8, i8* %21, align 1, !tbaa !3
  %23 = zext i8 %22 to i32
  %24 = getelementptr inbounds i8, i8* %12, i64 %20
  %25 = load i8, i8* %24, align 1, !tbaa !3
  %26 = zext i8 %25 to i32
  %27 = add nuw nsw i32 %23, 1
  %28 = add nuw nsw i32 %27, %26
  %29 = lshr i32 %28, 1
  %30 = trunc i32 %29 to i8
  %31 = getelementptr inbounds i8, i8* %10, i64 %20
  store i8 %30, i8* %31, align 1, !tbaa !3
  %32 = add nuw nsw i64 %20, 1
  %33 = icmp eq i64 %32, %7
  br i1 %33, label %13, label %19

; <label>:34:                                     ; preds = %13, %2
  br label %35

; <label>:35:                                     ; preds = %34
  ret void
}

; Checks the multi-version (MV) function on get_ref() has proper code generation
; CHECK-LABEL:  define internal fastcc void @get_ref(i32 %0, i32 %1) unnamed_addr #1 {
;
; checks 4 2-variable clones
;
; CHECK:        %2 = icmp eq i32 %0, 8
; CHECK:        %3 = icmp eq i32 %1, 8
; CHECK:        all fastcc void @"get_ref|8.8"()

; CHECK:        %5 = icmp eq i32 %0, 16
; CHECK:        %6 = icmp eq i32 %1, 16
; CHECK:        call fastcc void @"get_ref|16.16"()
;
; CHECK:        %8 = icmp eq i32 %0, 16
; CHECK:        %9 = icmp eq i32 %1, 8
; CHECK:        call fastcc void @"get_ref|16.8"()
;
; CHECK:        %11 = icmp eq i32 %0, 8
; CHECK:        %12 = icmp eq i32 %1, 16
; CHECK:        call fastcc void @"get_ref|8.16"()
;
; checks 4 1-variable clones
;
; CHECK:        %14 = icmp eq i32 %0, 20
; CHECK:        call fastcc void @"get_ref|20._"(i32 %1)
;
; CHECK:        %15 = icmp eq i32 %0, 16
; CHECK:        call fastcc void @"get_ref|16._"(i32 %1)
;
; CHECK:        %16 = icmp eq i32 %0, 12
; CHECK:        call fastcc void @"get_ref|12._"(i32 %1)
;
; CHECK:        %17 = icmp eq i32 %0, 8
; CHECK:        call fastcc void @"get_ref|8._"(i32 %1)
;
; checks the default-fallthrough path
;
; CHECK:        call fastcc void @"get_ref|_._"(i32 %0, i32 %1)
;
;


; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #2

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


