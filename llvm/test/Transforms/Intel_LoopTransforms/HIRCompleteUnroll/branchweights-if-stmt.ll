; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,hir-cg" -S 2>&1 < %s | FileCheck %s

; Verify that i1 loop is completely unrolled by 2 and branch_weights of if-stmt, and select insts are
; adjusted accordingly (divided by 2).
; The test case is modified from simplified-blob-operations-profitability.ll

; Before complete unroll
;  + DO i1 = 0, 1, 1   <DO_LOOP> // will be completely unrolled by 2
;  |   %3 = (@input_buf)[0][i1];
;  |   %18 = 0;
;  |   if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) >= 0)
;  |   {
;  |      %11 = 12;
;  |      if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) <= 32767)
;  |      {
;  |         %11 = (@alloc_map_buf)[0][(sext.i16.i64(%3) + -1 * sext.i16.i64(%add))/u64];
;  |      }
;  |      %17 = (zext.i16.i32(%11) + zext.i16.i32(%1) >u %add1) ? -1 * %1 + %val : %11;
;  |      %18 = %17;
;  |   }
;  |   (%ptr)[i1] = %18;
;  |   %1 = %18  +  %1;
;  + END LOOP

; After complete unroll
;         BEGIN REGION { modified }
;               %3 = (@input_buf)[0][0];
;               %18 = 0;
;               if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) >= 0)
;               {
;                  %11 = 12;
;                  if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) <= 32767)
;                  {
;                     %11 = (@alloc_map_buf)[0][(sext.i16.i64(%3) + -1 * sext.i16.i64(%add))/u64];
;                  }
;                  %17 = (zext.i16.i32(%11) + zext.i16.i32(%1) >u %add1) ? -1 * %1 + %val : %11;
;                  %18 = %17;
;               }
;               (%ptr)[0] = %18;
;               %1 = %18  +  %1;
;               %3 = (@input_buf)[0][1];
;               %18 = 0;
;               if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) >= 0)
;               {
;                  %11 = 12;
;                  if (sext.i16.i64(%3) + -1 * sext.i16.i64(%add) <= 32767)
;                  {
;                     %11 = (@alloc_map_buf)[0][(sext.i16.i64(%3) + -1 * sext.i16.i64(%add))/u64];
;                  }
;                  %17 = (zext.i16.i32(%11) + zext.i16.i32(%1) >u %add1) ? -1 * %1 + %val : %11;
;                  %18 = %17;
;               }
;               (%ptr)[1] = %18;
;               %1 = %18  +  %1;
;         END REGION

;CHECK: region.0:
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_FIRST_IF:[0-9]+]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_SECOND_IF:[0-9]+]]
;CHECK: select i1
;CHECK-SAME: !prof ![[PROF_SELECT:[0-9]+]]

;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_FIRST_IF]]
;CHECK: br i1
;CHECK-SAME: !prof ![[PROF_SECOND_IF]]
;CHECK: select i1
;CHECK-SAME: !prof ![[PROF_SELECT]]

;CHECK-DAG: ![[PROF_FIRST_IF]] = !{!"branch_weights", i32 300, i32 200}
;CHECK-DAG: ![[PROF_SECOND_IF]] = !{!"branch_weights", i32 75, i32 25}
;CHECK-DAG: ![[PROF_SELECT]] = !{!"branch_weights", i32 49, i32 30}

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
  %6 = icmp slt i64 %5, 100
  br i1 %6, label %backedge, label %if.then, !prof !1

if.then:                                     ; preds = %loop
  %7 = icmp sgt i64 %5, 200
  br i1 %7, label %if.inner, label %if.else, !prof !2

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
  %17 = select i1 %15, i16 %16, i16 %11, !prof !3
  br label %backedge

backedge:                                     ; preds = %if.inner, %loop
  %18 = phi i16 [ 0, %loop ], [ %17, %if.inner ]
  %19 = getelementptr inbounds i16, ptr %ptr, i64 %0
  store i16 %18, ptr %19, align 2
  %20 = add i16 %18, %1
  %21 = add nuw nsw i64 %0, 1
  %22 = icmp ne i64 %21, 2
  br i1 %22, label %loop, label %exit, !prof !4

exit:                                     ; preds = %backedge
  %23 = phi i16 [ %20, %backedge ]
  ret void
}

!1 = !{!"branch_weights", i32 400, i32 600} ;outer if
!2 = !{!"branch_weights", i32 50, i32 150}  ;inner if-invert
!3 = !{!"branch_weights", i32 99, i32 61}   ;select
!4 = !{!"branch_weights", i32 1000, i32 10} ;loop backedge
