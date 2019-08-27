; Checks that the Call Tree Cloning transformation clones expected functions;
; Checks that the Multi-Versioning (MV) transformation creates MV function as expected.

; RUN: opt < %s -passes='module(call-tree-clone)'  -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s
; RUN: opt < %s -call-tree-clone  -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -call-tree-clone-mv-bypass-coll-for-littest=1 -S | FileCheck %s

; Checks the multi-version (MV) function on mc_chroma() has proper code generation


;*** IR Dump After IP Cloning ***; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@dst = internal unnamed_addr global [1000 x i8] zeroinitializer, align 16
@src1 = internal unnamed_addr constant [1000 x i8] zeroinitializer, align 16
@src2 = internal global [1000 x i8] zeroinitializer, align 16
@Width = internal global i32 8, align 4
@Height = internal global i32 8, align 4

; Function Attrs: noinline norecurse nounwind uwtable
define internal fastcc void @mc_chroma(i32, i32) unnamed_addr #0 {
  %3 = icmp sgt i32 %1, 0
  br i1 %3, label %4, label %12

; <label>:4:                                      ; preds = %2
  %5 = icmp sgt i32 %0, 0
  %6 = sext i32 %0 to i64
  br label %7

; <label>:7:                                      ; preds = %13, %4
  %8 = phi i8* [ getelementptr (i8, i8* getelementptr inbounds ([1000 x i8], [1000 x i8]* @src1, i64 0, i64 0), i64 sext (i32 ashr (i32 ptrtoint ([1000 x i8]* @src2 to i32), i32 3) to i64)), %4 ], [ %11, %13 ]
  %9 = phi i32 [ 0, %4 ], [ %15, %13 ]
  %10 = phi i8* [ getelementptr inbounds ([1000 x i8], [1000 x i8]* @dst, i64 0, i64 0), %4 ], [ %14, %13 ]
  %11 = getelementptr inbounds i8, i8* %8, i64 2
  br i1 %5, label %17, label %13

; <label>:12:                                     ; preds = %13, %2
  ret void

; <label>:13:                                     ; preds = %17, %7
  %14 = getelementptr inbounds i8, i8* %10, i64 4
  %15 = add nuw nsw i32 %9, 1
  %16 = icmp eq i32 %15, %1
  br i1 %16, label %12, label %7

; <label>:17:                                     ; preds = %17, %7
  %18 = phi i64 [ %22, %17 ], [ 0, %7 ]
  %19 = getelementptr inbounds i8, i8* %8, i64 %18
  %20 = load i8, i8* %19, align 1, !tbaa !3
  %21 = zext i8 %20 to i32
  %22 = add nuw nsw i64 %18, 1
  %23 = getelementptr inbounds i8, i8* %11, i64 %18
  %24 = load i8, i8* %23, align 1, !tbaa !3
  %25 = zext i8 %24 to i32
  %26 = shl nuw nsw i32 %25, 5
  %27 = shl nuw nsw i32 %21, 5
  %28 = add nuw nsw i32 %27, 32
  %29 = add nuw nsw i32 %28, %26
  %30 = lshr i32 %29, 6
  %31 = trunc i32 %30 to i8
  %32 = getelementptr inbounds i8, i8* %10, i64 %18
  store i8 %31, i8* %32, align 1, !tbaa !3
  %33 = icmp eq i64 %22, %6
  br i1 %33, label %13, label %17
}

; Checks the multi-version (MV) function on mc_chroma() has proper code generation
; CHECK-LABEL:  define internal fastcc void @mc_chroma(i32 %0, i32 %1) unnamed_addr #0 {
;
; checks 4 2-variable clones
;
; CHECK:        %2 = icmp eq i32 %0, 8
; CHECK:        %3 = icmp eq i32 %1, 8
; CHECK:        call fastcc void @"mc_chroma|8.8"()

; CHECK:        %5 = icmp eq i32 %0, 16
; CHECK:        %6 = icmp eq i32 %1, 16
; CHECK:        call fastcc void @"mc_chroma|16.16"()
;
; CHECK:        %8 = icmp eq i32 %0, 16
; CHECK:        %9 = icmp eq i32 %1, 8
; CHECK:        call fastcc void @"mc_chroma|16.8"()
;
; CHECK:        %11 = icmp eq i32 %0, 8
; CHECK:        %12 = icmp eq i32 %1, 16
; CHECK:        call fastcc void @"mc_chroma|8.16"()
;
; checks the default-fallthrough path
;
; CHECK:        call fastcc void @"mc_chroma|_._"(i32 %0, i32 %1)
;

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
  tail call fastcc void @mc_chroma(i32 8, i32 8)
  tail call fastcc void @mc_chroma(i32 8, i32 16)
  tail call fastcc void @mc_chroma(i32 16, i32 8)
  tail call fastcc void @mc_chroma(i32 16, i32 16)
  br label %2

; <label>:1:                                      ; preds = %2
  ret i32 0

; <label>:2:                                      ; preds = %2, %0
  %3 = phi i32 [ 0, %0 ], [ %6, %2 ]
  %4 = load volatile i32, i32* @Width, align 4, !tbaa !6
  %5 = load volatile i32, i32* @Height, align 4, !tbaa !6
  tail call fastcc void @mc_chroma(i32 %4, i32 %5)
  %6 = add nuw nsw i32 %3, 1
  %7 = icmp eq i32 %6, 100
  br i1 %7, label %1, label %2
}

; Checks the 2-variable clones are created and called:
; CHECK:  call fastcc void @"mc_chroma|8.8"()
; CHECK:  call fastcc void @"mc_chroma|8.16"()
; CHECK:  call fastcc void @"mc_chroma|16.8"()
; CHECK:  call fastcc void @"mc_chroma|16.16"()
; CHECK:  tail call fastcc void @mc_chroma(i32 %4, i32 %5)
;
; Checks the 1-variable clones are NOT created:
; CHECK-NOT:  call fastcc void @"mc_chroma|20._"()
; CHECK-NOT:  call fastcc void @"mc_chroma|16._"()
; CHECK-NOT:  call fastcc void @"mc_chroma|12._"()
; CHECK-NOT:  call fastcc void @"mc_chroma|8._"()
;

; After CMPLRLLVM-454, we try to maintain a stable function ordering.
;
; CHECK: define internal fastcc void @"mc_chroma|8.8"()
; CHECK: define internal fastcc void @"mc_chroma|8.16"()
; CHECK: define internal fastcc void @"mc_chroma|16.8"()
; CHECK: define internal fastcc void @"mc_chroma|16.16"()
; CHECK: define internal fastcc void @"mc_chroma|_._"(i32 %0, i32 %1)
; CHECK: define internal fastcc void @"mc_chroma|20._"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|16._"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|12._"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|8._"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|_.20"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|_.16"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|_.12"(i32 %0)
; CHECK: define internal fastcc void @"mc_chroma|_.8"(i32 %0)

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ff38d5989c66cc12167cbe397bfb5d6915c4838f) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm b4619eab7f28ef121aff57eaeb223ab6d5182f19)"}
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
;void __attribute__((noinline)) mc_chroma( uint8_t *dst, int i_dst_stride,
;                       uint8_t *src, int i_src_stride,
;                       int mvx, int mvy,
;                       int i_width, int i_height )
;{
;    uint8_t *srcp;
;
;    int d8x = mvx&0x07;
;    int d8y = mvy&0x07;
;    int cA = (8-d8x)*(8-d8y);
;    int cB = d8x    *(8-d8y);
;    int cC = (8-d8x)*d8y;
;    int cD = d8x    *d8y;
;
;    src += (mvy >> 3) * i_src_stride + (mvx >> 3);
;    srcp = &src[i_src_stride];
;
;    for( int y = 0; y < i_height; y++ )
;    {
;        for( int x = 0; x < i_width; x++ )
;            dst[x] = ( cA*src[x]  + cB*src[x+1] + cC*srcp[x] + cD*srcp[x+1] + 32 ) >> 6;
;        dst  += i_dst_stride;
;        src   = srcp;
;        srcp += i_src_stride;
;    }
;}
;
;
;int main(void) {
;  mc_chroma(dst, 4, src1, 2, src2, 4, 8, 8);
;  mc_chroma(dst, 4, src1, 2, src2, 4, 8, 16);
;  mc_chroma(dst, 4, src1, 2, src2, 4, 16, 8);
;  mc_chroma(dst, 4, src1, 2, src2, 4, 16, 16);
;
;  for (int i = 0; i < 100; ++i) {
;	mc_chroma(dst, 4, src1, 2, src2, 4, Width, Height);
;  }
;
;  return 0;
;}
;
