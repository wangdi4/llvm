; RUN: opt -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-opt-predicate -print-after=hir-opt-predicate -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that we do not perform illegal partial unswitch for (%t74 == 0) after unrolling i2 loop

; *** IR Dump Before HIR PreVec Complete Unroll ***
; BEGIN REGION { }
;       + DO i1 = 0, 126, 1   <DO_LOOP>
;       |   %t62 = (@reg_class_contents)[0][-1 * i1 + 126];
;       |   %t63 = %t62  &&  %t58;
;       |   (@ira_class_hard_reg_index)[0][-1 * i1 + 126][0] = -1;
;       |   %t67 = 0;
;       |   %t68 = 0;
;       |
;       |   + DO i2 = 0, 1, 1   <DO_LOOP>
;       |   |   %t68.out2 = %t68;
;       |   |   %t70 = (@reg_alloc_order)[0][i2];
;       |   |   %t73 = 1  <<  trunc.i32.i6(%t70);
;       |   |   %t74 = %t73  &&  %t67;
;       |   |   if (%t74 == 0)
;       |   |   {
;       |   |      %t67 = %t73  ||  %t67;
;       |   |      %t78 = %t73  &&  %t63;
;       |   |      if (%t78 == 0)
;       |   |      {
;       |   |         (@ira_class_hard_reg_index)[0][-1 * i1 + 126][%t70] = -1;
;       |   |      }
;       |   |      else
;       |   |      {
;       |   |         (@ira_class_hard_reg_index)[0][-1 * i1 + 126][%t70] = %t68.out2;
;       |   |         %t68 = %t68  +  1;
;       |   |         (@ira_class_hard_regs)[0][-1 * i1 + 126][%t68.out2] = %t70;
;       |   |      }
;       |   |   }
;       |   |   %t68.out = %t68;
;       |   + END LOOP
;       |
;       |   (@ira_class_hard_regs_num)[0][-1 * i1 + 126] = %t68.out;
;       + END LOOP
; END REGION

; *** IR Dump After HIR PreVec Complete Unroll ***
; BEGIN REGION { modified }
;       + DO i1 = 0, 126, 1   <DO_LOOP>
;       |   %t62 = (@reg_class_contents)[0][-1 * i1 + 126];
;       |   %t63 = %t62  &&  %t58;
;       |   (@ira_class_hard_reg_index)[0][-1 * i1 + 126][0] = -1;
;       |   %t67 = 0;
;       |   %t68 = 0;
;       |   %t68.out2 = %t68;
;       |   %t70 = (@reg_alloc_order)[0][0];
;       |   %t73 = 1  <<  trunc.i32.i6(%t70);
;       |   %t74 = %t73  &&  %t67;
;       |   if (%t74 == 0)
;       |   {
;       |     ...
;       |   }
;       |   %t68.out = %t68;
;       |   %t68.out2 = %t68;
;       |   %t70 = (@reg_alloc_order)[0][1];
;       |   %t73 = 1  <<  trunc.i32.i6(%t70);
;       |   %t74 = %t73  &&  %t67;
;       |   if (%t74 == 0)
;       |   {
;       |     ...
;       |   }
;       |   %t68.out = %t68;
;       |   (@ira_class_hard_regs_num)[0][-1 * i1 + 126] = %t68.out;
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK: DO i1 = 0, 126, 1   <DO_LOOP>
; CHECK: END LOOP
; CHECK-NOT: DO i1 = 0, 126, 1   <DO_LOOP>
; CHECK: END REGION

; ModuleID = 'func.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ira_class_hard_reg_index = external hidden unnamed_addr global [27 x [53 x i16]], align 16
@ira_class_hard_regs = external hidden unnamed_addr global [27 x [53 x i16]], align 16
@ira_class_hard_regs_num = external hidden unnamed_addr global [27 x i32], align 16
@reg_alloc_order = external hidden unnamed_addr constant [53 x i32], align 16
@reg_class_contents = external hidden unnamed_addr global [27 x i64], align 16

; Function Attrs: nounwind uwtable
define hidden fastcc void @ira_init(i64 %t58) unnamed_addr #2 {
header:
  %t0 = alloca [27 x i8], align 16
  br label %t59

t59:
  %t60 = phi i64 [ 126, %header ], [ %t98, %t95 ] ; preds = %t95, header
  %t61 = getelementptr inbounds [27 x i64], [27 x i64]* @reg_class_contents, i64 0, i64 %t60
  %t62 = load i64, i64* %t61, align 8, !tbaa !18
  %t63 = and i64 %t62, %t58
  %t64 = getelementptr inbounds [27 x [53 x i16]], [27 x [53 x i16]]* @ira_class_hard_reg_index, i64 0, i64 %t60, i64 0
  store i16 -1, i16* %t64, align 2, !tbaa !20
  br label %t65

t65:                                     ; preds = %t90, %t59
  %t66 = phi i64 [ 0, %t59 ], [ %t93, %t90 ]
  %t67 = phi i64 [ 0, %t59 ], [ %t92, %t90 ]
  %t68 = phi i32 [ 0, %t59 ], [ %t91, %t90 ]
  %t69 = getelementptr inbounds [53 x i32], [53 x i32]* @reg_alloc_order, i64 0, i64 %t66
  %t70 = load i32, i32* %t69, align 4, !tbaa !24
  %t71 = and i32 %t70, 63
  %t72 = zext i32 %t71 to i64
  %t73 = shl i64 1, %t72
  %t74 = and i64 %t73, %t67
  %t75 = icmp eq i64 %t74, 0
  br i1 %t75, label %t76, label %t90

t76:                                     ; preds = %t65
  %t77 = or i64 %t73, %t67
  %t78 = and i64 %t73, %t63
  %t79 = icmp eq i64 %t78, 0
  %t80 = sext i32 %t70 to i64
  br i1 %t79, label %t81, label %t83

t81:                                     ; preds = %t76
  %t82 = getelementptr inbounds [27 x [53 x i16]], [27 x [53 x i16]]* @ira_class_hard_reg_index, i64 0, i64 %t60, i64 %t80
  store i16 -1, i16* %t82, align 2, !tbaa !20
  br label %t90

t83:                                     ; preds = %t76
  %t84 = trunc i32 %t68 to i16
  %t85 = getelementptr inbounds [27 x [53 x i16]], [27 x [53 x i16]]* @ira_class_hard_reg_index, i64 0, i64 %t60, i64 %t80
  store i16 %t84, i16* %t85, align 2, !tbaa !20
  %t86 = trunc i32 %t70 to i16
  %t87 = add nsw i32 %t68, 1
  %t88 = sext i32 %t68 to i64
  %t89 = getelementptr inbounds [27 x [53 x i16]], [27 x [53 x i16]]* @ira_class_hard_regs, i64 0, i64 %t60, i64 %t88
  store i16 %t86, i16* %t89, align 2, !tbaa !20
  br label %t90

t90:                                     ; preds = %t83, %t81, %t65
  %t91 = phi i32 [ %t68, %t65 ], [ %t87, %t83 ], [ %t68, %t81 ]
  %t92 = phi i64 [ %t67, %t65 ], [ %t77, %t83 ], [ %t77, %t81 ]
  %t93 = add nuw nsw i64 %t66, 1
  %t94 = icmp eq i64 %t93, 2
  br i1 %t94, label %t95, label %t65

t95:                                     ; preds = %t90
  %t96 = phi i32 [ %t91, %t90 ]
  %t97 = getelementptr inbounds [27 x i32], [27 x i32]* @ira_class_hard_regs_num, i64 0, i64 %t60
  store i32 %t96, i32* %t97, align 4, !tbaa !26
  %t98 = add nsw i64 %t60, -1
  %t99 = icmp eq i64 %t60, 0
  br i1 %t99, label %exit, label %t59

exit:                                   ; preds = %t96
  ret void
}

!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!11 = !{!"long", !6, i64 0}
!16 = !{!"int", !6, i64 0}
!18 = !{!19, !11, i64 0}
!19 = !{!"array@_ZTSA27_m", !11, i64 0}
!20 = !{!21, !23, i64 0}
!21 = !{!"array@_ZTSA27_A53_s", !22, i64 0}
!22 = !{!"array@_ZTSA53_s", !23, i64 0}
!23 = !{!"short", !6, i64 0}
!24 = !{!25, !16, i64 0}
!25 = !{!"array@_ZTSA53_i", !16, i64 0}
!26 = !{!27, !16, i64 0}
!27 = !{!"array@_ZTSA27_i", !16, i64 0}

