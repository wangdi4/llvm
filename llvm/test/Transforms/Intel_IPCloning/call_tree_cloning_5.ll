; Checks that the Call Tree Cloning transformation clones expected functions;
; Checks that the Multi-Versioning (MV) transformation creates MV function as expected.

; RUN: opt < %s -passes='module(call-tree-clone)' -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s
; RUN: opt < %s -call-tree-clone -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s

; Checks the multi-version (MV) function on pixel_avg() has proper code generation


;*** IR Dump After IP Cloning ***; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@dst = internal unnamed_addr global [1000 x i8] zeroinitializer, align 16
@src1 = internal unnamed_addr constant [1000 x i8] zeroinitializer, align 16
@src2 = internal unnamed_addr constant [1000 x i8] zeroinitializer, align 16
@Width = internal global i32 8, align 4
@Height = internal global i32 8, align 4

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @pixel_avg(i32, i32) unnamed_addr #0 {
  %3 = icmp sgt i32 %1, 0
  br i1 %3, label %4, label %12

; <label>:4:                                      ; preds = %2
  %5 = icmp sgt i32 %0, 0
  %6 = sext i32 %0 to i64
  br label %7

; <label>:7:                                      ; preds = %13, %4
  %8 = phi i32 [ 0, %4 ], [ %17, %13 ]
  %9 = phi i8* [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %4 ], [ %14, %13 ]
  %10 = phi i8* [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @src1, i64 0, i64 0), %4 ], [ %15, %13 ]
  %11 = phi i8* [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @src2, i64 0, i64 0), %4 ], [ %16, %13 ]
  br i1 %5, label %19, label %13

; <label>:12:                                     ; preds = %13, %2
  ret void

; <label>:13:                                     ; preds = %19, %7
  %14 = getelementptr inbounds i8, i8* %9, i64 4
  %15 = getelementptr inbounds i8, i8* %10, i64 2
  %16 = getelementptr inbounds i8, i8* %11, i64 4
  %17 = add nuw nsw i32 %8, 1
  %18 = icmp eq i32 %17, %1
  br i1 %18, label %12, label %7

; <label>:19:                                     ; preds = %19, %7
  %20 = phi i64 [ %32, %19 ], [ 0, %7 ]
  %21 = getelementptr inbounds i8, i8* %10, i64 %20
  %22 = load i8, i8* %21, align 1, !tbaa !3
  %23 = zext i8 %22 to i32
  %24 = getelementptr inbounds i8, i8* %11, i64 %20
  %25 = load i8, i8* %24, align 1, !tbaa !3
  %26 = zext i8 %25 to i32
  %27 = add nuw nsw i32 %23, 1
  %28 = add nuw nsw i32 %27, %26
  %29 = lshr i32 %28, 1
  %30 = trunc i32 %29 to i8
  %31 = getelementptr inbounds i8, i8* %9, i64 %20
  store i8 %30, i8* %31, align 1, !tbaa !3
  %32 = add nuw nsw i64 %20, 1
  %33 = icmp eq i64 %32, %6
  br i1 %33, label %13, label %19
}

; Checks the multi-version (MV) function on pixel_avg() has proper code generation
; CHECK-LABEL:  define internal fastcc void @pixel_avg(i32 %0, i32 %1) unnamed_addr #0 {
;
; checks 4 2-variable clones
;
; CHECK:        %2 = icmp eq i32 %0, 8
; CHECK:        %3 = icmp eq i32 %1, 8
; CHECK:        call fastcc void @"pixel_avg|8.8"()
;
; CHECK:        %5 = icmp eq i32 %0, 16
; CHECK:        %6 = icmp eq i32 %1, 16
; CHECK:        call fastcc void @"pixel_avg|16.16"()
;
; CHECK:        %8 = icmp eq i32 %0, 16
; CHECK:        %9 = icmp eq i32 %1, 8
; CHECK:        call fastcc void @"pixel_avg|16.8"()
;
; CHECK:        %11 = icmp eq i32 %0, 8
; CHECK:        %12 = icmp eq i32 %1, 16
; CHECK:        call fastcc void @"pixel_avg|8.16"()
;
; checks 4 1-variable clones
;
; CHECK:        %14 = icmp eq i32 %0, 20
; CHECK:        call fastcc void @"pixel_avg|20._"(i32 %1)
;
; CHECK:        %15 = icmp eq i32 %0, 16
; CHECK:        call fastcc void @"pixel_avg|16._"(i32 %1)
;
; CHECK:        %16 = icmp eq i32 %0, 12
; CHECK:        call fastcc void @"pixel_avg|12._"(i32 %1)
;
; CHECK:        %17 = icmp eq i32 %0, 8
; CHECK:        call fastcc void @"pixel_avg|8._"(i32 %1)
;
; checks the default-fallthrough path
;
; CHECK:        call fastcc void @"pixel_avg|_._"(i32 %0, i32 %1)
;

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
  tail call fastcc void @pixel_avg(i32 8, i32 8)
  tail call fastcc void @pixel_avg(i32 8, i32 16)
  tail call fastcc void @pixel_avg(i32 16, i32 8)
  tail call fastcc void @pixel_avg(i32 16, i32 16)
  br label %2

; <label>:1:                                      ; preds = %2
  ret i32 0

; <label>:2:                                      ; preds = %2, %0
  %3 = phi i32 [ 0, %0 ], [ %6, %2 ]
  %4 = load volatile i32, i32* @Width, align 4, !tbaa !6
  %5 = load volatile i32, i32* @Height, align 4, !tbaa !6
  tail call fastcc void @pixel_avg(i32 %4, i32 %5)
  %6 = add nuw nsw i32 %3, 1
  %7 = icmp eq i32 %6, 100
  br i1 %7, label %1, label %2
}

; Checks the 2-variable clones are created and called:
; CHECK:  "pixel_avg|8.8"
; CHECK:  "pixel_avg|8.16"
; CHECK:  "pixel_avg|16.8"
; CHECK:  "pixel_avg|16.16"
; CHECK:  "pixel_avg|_._"
;
; Checks the 1-variable clones are created:
; CHECK:  "pixel_avg|20._"
; CHECK:  "pixel_avg|16._"
; CHECK:  "pixel_avg|12._"
; CHECK:  "pixel_avg|8._"
;


attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang fc25755a7dd8cc64339b342d0cba7a81391fbd6e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 94c6faa25b891ab331f9bf9dc396150dfcfa1477)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}

; Original source a.c is below.
; Compile: icx -flto test.c

;Goal: try to build a case that works for cloner-based multi versioniong (MV)
;#include <stdint.h>
;
;volatile int Height = 8;
;volatile int Width = 8;
;
;uint8_t dst[1000];
;uint8_t src1[1000];
;uint8_t src2[1000];
;
;void __attribute__((noinline)) pixel_avg(uint8_t *dst, int i_dst_stride,
;                                         uint8_t *src1, int i_src1_stride,
;                                         uint8_t *src2, int i_src2_stride,
;                                         int i_width, int i_height) {
;  for (int y = 0; y < i_height; y++) {
;    for (int x = 0; x < i_width; x++)
;      dst[x] = (src1[x] + src2[x] + 1) >> 1;
;    dst += i_dst_stride;
;    src1 += i_src1_stride;
;    src2 += i_src2_stride;
;  }
;}
;
;int main(void) {
;  pixel_avg(dst, 4, src1, 2, src2, 4, 8, 8);
;  pixel_avg(dst, 4, src1, 2, src2, 4, 8, 16);
;  pixel_avg(dst, 4, src1, 2, src2, 4, 16, 8);
;  pixel_avg(dst, 4, src1, 2, src2, 4, 16, 16);
;
;  for (int i = 0; i < 100; ++i) {
;    pixel_avg(dst, 4, src1, 2, src2, 4, Width, Height);
;  }
;
;  return 0;
;}
;
