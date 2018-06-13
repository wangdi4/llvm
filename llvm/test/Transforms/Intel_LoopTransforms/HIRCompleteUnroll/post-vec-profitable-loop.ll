; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; This loop is extracted from @x264_pixel_satd_8x4 in 525.x264. 
; Check that the loop becomes profitable post-vectorizer.

; CHECK: Function

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %t13 = (%t0)[%t7 * i1];
; CHECK: |   %t15 = (%t2)[%t8 * i1];
; CHECK: |   %t19 = (%t0)[%t7 * i1 + 4];
; CHECK: |   %t22 = (%t2)[%t8 * i1 + 4];
; CHECK: |   %t28 = (%t0)[%t7 * i1 + 1];
; CHECK: |   %t31 = (%t2)[%t8 * i1 + 1];
; CHECK: |   %t35 = (%t0)[%t7 * i1 + 5];
; CHECK: |   %t38 = (%t2)[%t8 * i1 + 5];
; CHECK: |   %t44 = (%t0)[%t7 * i1 + 2];
; CHECK: |   %t47 = (%t2)[%t8 * i1 + 2];
; CHECK: |   %t51 = (%t0)[%t7 * i1 + 6];
; CHECK: |   %t54 = (%t2)[%t8 * i1 + 6];
; CHECK: |   %t60 = (%t0)[%t7 * i1 + 3];
; CHECK: |   %t63 = (%t2)[%t8 * i1 + 3];
; CHECK: |   %t67 = (%t0)[%t7 * i1 + 7];
; CHECK: |   %t70 = (%t2)[%t8 * i1 + 7];
; CHECK: |   (%t5)[0][i1][0] = zext.i8.i32(%t13) + zext.i8.i32(%t28) + zext.i8.i32(%t44) + zext.i8.i32(%t60) + 65536 * zext.i8.i32(%t19) + 65536 * zext.i8.i32(%t35) + 65536 * zext.i8.i32(%t51) + 65536 * zext.i8.i32(%t67) + -65536 * zext.i8.i32(%t22) + -65536 * zext.i8.i32(%t38) + -65536 * zext.i8.i32(%t54) + -65536 * zext.i8.i32(%t70) + -1 * zext.i8.i32(%t15) + -1 * zext.i8.i32(%t31) + -1 * zext.i8.i32(%t47) + -1 * zext.i8.i32(%t63);
; CHECK: |   (%t5)[0][i1][2] = zext.i8.i32(%t13) + zext.i8.i32(%t28) + -1 * zext.i8.i32(%t44) + -1 * zext.i8.i32(%t60) + 65536 * zext.i8.i32(%t19) + 65536 * zext.i8.i32(%t35) + -65536 * zext.i8.i32(%t51) + -65536 * zext.i8.i32(%t67) + -65536 * zext.i8.i32(%t22) + -65536 * zext.i8.i32(%t38) + 65536 * zext.i8.i32(%t54) + 65536 * zext.i8.i32(%t70) + -1 * zext.i8.i32(%t15) + -1 * zext.i8.i32(%t31) + zext.i8.i32(%t47) + zext.i8.i32(%t63);
; CHECK: |   (%t5)[0][i1][1] = zext.i8.i32(%t13) + -1 * zext.i8.i32(%t28) + zext.i8.i32(%t44) + -1 * zext.i8.i32(%t60) + 65536 * zext.i8.i32(%t19) + -65536 * zext.i8.i32(%t35) + 65536 * zext.i8.i32(%t51) + -65536 * zext.i8.i32(%t67) + -65536 * zext.i8.i32(%t22) + 65536 * zext.i8.i32(%t38) + -65536 * zext.i8.i32(%t54) + 65536 * zext.i8.i32(%t70) + -1 * zext.i8.i32(%t15) + zext.i8.i32(%t31) + -1 * zext.i8.i32(%t47) + zext.i8.i32(%t63);
; CHECK: |   (%t5)[0][i1][3] = zext.i8.i32(%t13) + -1 * zext.i8.i32(%t28) + -1 * zext.i8.i32(%t44) + zext.i8.i32(%t60) + 65536 * zext.i8.i32(%t19) + -65536 * zext.i8.i32(%t35) + -65536 * zext.i8.i32(%t51) + 65536 * zext.i8.i32(%t67) + -65536 * zext.i8.i32(%t22) + 65536 * zext.i8.i32(%t38) + 65536 * zext.i8.i32(%t54) + -65536 * zext.i8.i32(%t70) + -1 * zext.i8.i32(%t15) + zext.i8.i32(%t31) + zext.i8.i32(%t47) + -1 * zext.i8.i32(%t63);
; CHECK: + END LOOP


; CHECK: Function

; CHECK-NOT DO i1


define void @foo(i8* %t0, i8* %t2, i64 %t7, i64 %t8) {
entry:
 %t5 = alloca [4 x [4 x i32]], align 16
 br label %loop

loop:                                      ; preds = %loop, %entry
  %t10 = phi i64 [ 0, %entry ], [ %t87, %loop ]
  %t11 = phi i8* [ %t0, %entry ], [ %t88, %loop ]
  %t12 = phi i8* [ %t2, %entry ], [ %t89, %loop ]
  %t13 = load i8, i8* %t11, align 1
  %t14 = zext i8 %t13 to i32
  %t15 = load i8, i8* %t12, align 1
  %t16 = zext i8 %t15 to i32
  %t17 = sub nsw i32 %t14, %t16
  %t18 = getelementptr inbounds i8, i8* %t11, i64 4
  %t19 = load i8, i8* %t18, align 1
  %t20 = zext i8 %t19 to i32
  %t21 = getelementptr inbounds i8, i8* %t12, i64 4
  %t22 = load i8, i8* %t21, align 1
  %t23 = zext i8 %t22 to i32
  %t24 = sub nsw i32 %t20, %t23
  %t25 = shl nsw i32 %t24, 16
  %t26 = add nsw i32 %t25, %t17
  %t27 = getelementptr inbounds i8, i8* %t11, i64 1
  %t28 = load i8, i8* %t27, align 1
  %t29 = zext i8 %t28 to i32
  %t30 = getelementptr inbounds i8, i8* %t12, i64 1
  %t31 = load i8, i8* %t30, align 1
  %t32 = zext i8 %t31 to i32
  %t33 = sub nsw i32 %t29, %t32
  %t34 = getelementptr inbounds i8, i8* %t11, i64 5
  %t35 = load i8, i8* %t34, align 1
  %t36 = zext i8 %t35 to i32
  %t37 = getelementptr inbounds i8, i8* %t12, i64 5
  %t38 = load i8, i8* %t37, align 1
  %t39 = zext i8 %t38 to i32
  %t40 = sub nsw i32 %t36, %t39
  %t41 = shl nsw i32 %t40, 16
  %t42 = add nsw i32 %t41, %t33
  %t43 = getelementptr inbounds i8, i8* %t11, i64 2
  %t44 = load i8, i8* %t43, align 1
  %t45 = zext i8 %t44 to i32
  %t46 = getelementptr inbounds i8, i8* %t12, i64 2
  %t47 = load i8, i8* %t46, align 1
  %t48 = zext i8 %t47 to i32
  %t49 = sub nsw i32 %t45, %t48
  %t50 = getelementptr inbounds i8, i8* %t11, i64 6
  %t51 = load i8, i8* %t50, align 1
  %t52 = zext i8 %t51 to i32
  %t53 = getelementptr inbounds i8, i8* %t12, i64 6
  %t54 = load i8, i8* %t53, align 1
  %t55 = zext i8 %t54 to i32
  %t56 = sub nsw i32 %t52, %t55
  %t57 = shl nsw i32 %t56, 16
  %t58 = add nsw i32 %t57, %t49
  %t59 = getelementptr inbounds i8, i8* %t11, i64 3
  %t60 = load i8, i8* %t59, align 1
  %t61 = zext i8 %t60 to i32
  %t62 = getelementptr inbounds i8, i8* %t12, i64 3
  %t63 = load i8, i8* %t62, align 1
  %t64 = zext i8 %t63 to i32
  %t65 = sub nsw i32 %t61, %t64
  %t66 = getelementptr inbounds i8, i8* %t11, i64 7
  %t67 = load i8, i8* %t66, align 1
  %t68 = zext i8 %t67 to i32
  %t69 = getelementptr inbounds i8, i8* %t12, i64 7
  %t70 = load i8, i8* %t69, align 1
  %t71 = zext i8 %t70 to i32
  %t72 = sub nsw i32 %t68, %t71
  %t73 = shl nsw i32 %t72, 16
  %t74 = add nsw i32 %t73, %t65
  %t75 = add nsw i32 %t42, %t26
  %t76 = sub nsw i32 %t26, %t42
  %t77 = add nsw i32 %t74, %t58
  %t78 = sub nsw i32 %t58, %t74
  %t79 = add nsw i32 %t77, %t75
  %t80 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %t5, i64 0, i64 %t10, i64 0
  store i32 %t79, i32* %t80, align 16
  %t81 = sub nsw i32 %t75, %t77
  %t82 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %t5, i64 0, i64 %t10, i64 2
  store i32 %t81, i32* %t82, align 8
  %t83 = add nsw i32 %t78, %t76
  %t84 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %t5, i64 0, i64 %t10, i64 1
  store i32 %t83, i32* %t84, align 4
  %t85 = sub nsw i32 %t76, %t78
  %t86 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %t5, i64 0, i64 %t10, i64 3
  store i32 %t85, i32* %t86, align 4
  %t87 = add nuw nsw i64 %t10, 1
  %t88 = getelementptr inbounds i8, i8* %t11, i64 %t7
  %t89 = getelementptr inbounds i8, i8* %t12, i64 %t8
  %t90 = icmp eq i64 %t87, 4
  br i1 %t90, label %exit, label %loop

exit:
  ret void
}
