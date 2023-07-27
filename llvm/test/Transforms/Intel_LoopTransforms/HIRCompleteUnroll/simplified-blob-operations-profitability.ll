; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that the appearence of simplified temp blob %3 (due to constant array access of @input_buf) in sext blobs makes this loop profitable.

; CHECK: Function

; CHECK: + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK: |   %3 = (@input_buf)[0][i1];
; CHECK: |   %18 = 0;
; CHECK: |   if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) >= 0)
; CHECK: |   {
; CHECK: |      %11 = 12;
; CHECK: |      if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) <= 32767)
; CHECK: |      {
; CHECK: |         %11 = (@alloc_map_buf)[0][(sext.i16.i64(%3) + -1 * sext.i16.i64(%add))/u64];
; CHECK: |      }
; CHECK: |      %17 = (zext.i16.i32(%11) + zext.i16.i32(%1) >u %add1) ? -1 * %1 + %val : %11;
; CHECK: |      %18 = %17;
; CHECK: |   }
; CHECK: |   (%ptr)[i1] = %18;
; CHECK: |   %1 = %18  +  %1;
; CHECK: + END LOOP

; CHECK: Function

; CHECK-NOT: DO i1

@input_buf = internal unnamed_addr constant [20 x i16] [i16 -5120, i16 -5120, i16 -5120, i16 -5120, i16 -5120, i16 15360, i16 15360, i16 15360, i16 15360, i16 15360, i16 15360, i16 15360, i16 15360, i16 15360, i16 15360, i16 -5120, i16 -5120, i16 -5120, i16 -5120, i16 -5120], align 16
@alloc_map_buf = internal unnamed_addr global [512 x i16] [i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 5, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 6, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 7, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 9, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 10, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 11, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 13, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 14, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 15, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 17, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 18, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 19, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 20, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21, i16 21], align 16

define void @foo(ptr %ptr, i16 %add, i32 %add1, i16 %val) {
entry:
  %add.ext = sext i16 %add to i64
  br label %loop

loop:                                     ; preds = %backedge, %entry
  %0 = phi i64 [ %21, %backedge ], [ 0, %entry ]
  %1 = phi i16 [ 0, %entry ], [ %20, %backedge ]
  %2 = getelementptr inbounds [20 x i16], ptr @input_buf, i64 0, i64 %0
  %3 = load i16, ptr %2, align 2
  %4 = sext i16 %3 to i64
  %5 = sub nsw i64 %4, %add.ext
  %6 = icmp slt i64 %5, 0
  br i1 %6, label %backedge, label %if.then

if.then:                                     ; preds = %loop
  %7 = icmp sgt i64 %5, 32767
  br i1 %7, label %if.inner, label %if.else

if.else:                                     ; preds = %if.then
  %8 = lshr i64 %5, 6
  %9 = getelementptr inbounds [512 x i16], ptr @alloc_map_buf, i64 0, i64 %8
  %10 = load i16, ptr %9, align 2
  br label %if.inner

if.inner:                                     ; preds = %if.else, %if.then
  %11 = phi i16 [ %10, %if.else ], [ 12, %if.then ]
  %12 = zext i16 %11 to i32
  %13 = zext i16 %1 to i32
  %14 = add nuw nsw i32 %12, %13
  %15 = icmp ugt i32 %14, %add1
  %16 = sub i16 %val, %1
  %17 = select i1 %15, i16 %16, i16 %11
  br label %backedge

backedge:                                     ; preds = %if.inner, %loop
  %18 = phi i16 [ 0, %loop ], [ %17, %if.inner ]
  %19 = getelementptr inbounds i16, ptr %ptr, i64 %0
  store i16 %18, ptr %19, align 2
  %20 = add i16 %18, %1
  %21 = add nuw nsw i64 %0, 1
  %22 = icmp ne i64 %21, 20
  br i1 %22, label %loop, label %exit

exit:                                     ; preds = %backedge
  %23 = phi i16 [ %20, %backedge ]
  ret void
}


