; XFAIL: *
; CMPLRLLVM-35135: Test case marked as XFAIL. It needs to be updated due to
; enabling instruction simplify and CFG simplify passes before devirtualization
; in the new pass manager (CMPLRLLVM-34961).

; Reduced from polyhedron2011F/gas_dyn2.
; CMPLRLLVM-28038.
; We don't want to run hoisting in the back part of the LTO-only pipeline.
; This avoids over-conversion of branches to selects, and improves the
; performance of several benchmarks.

; RUN: opt %s -S -passes='lto<O3>' | FileCheck  %s
; The or-blocks under bb168 should not be hoisted and folded into a select.
; CHECK: bb168:
; CHECK-NOT: select
; CHECK: bb{{.*}}:
; CHECK: or i64

; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = external hidden unnamed_addr global [1 x i32], align 16

define hidden fastcc void @hoge(i32 %arg) unnamed_addr #0 {
bb:
  %tmp9 = sext i32 %arg to i64
  %tmp10 = icmp sgt i64 %tmp9, 0
  %tmp11 = select i1 %tmp10, i64 %tmp9, i64 0
  %tmp12 = alloca float, i64 %tmp11, align 4
  %tmp16 = lshr i64 %tmp9, 3
  %tmp94 = add nsw i64 %tmp16, -1
  br label %bb95

bb95:                                             ; preds = %bb190, %bb
  %tmp98 = phi i64 [ 0, %bb ], [ %tmp193, %bb190 ]
  %tmp99 = shl i64 %tmp98, 3
  %tmp154 = or i64 %tmp99, 5
  %tmp160 = getelementptr inbounds float, float* %tmp12, i64 %tmp154
  %tmp166 = or i64 %tmp99, 6
  br label %bb168

bb168:                                            ; preds = %bb95
  %tmp172 = getelementptr inbounds float, float* %tmp12, i64 %tmp166
  %tmp173 = load float, float* %tmp172, align 4
  %tmp174 = fcmp fast olt float %tmp173, 0.000000e+00
  br i1 %tmp174, label %bb177, label %bb175

bb175:                                            ; preds = %bb168
  %tmp176 = or i64 %tmp99, 7
  br label %bb180

bb177:                                            ; preds = %bb168
  %tmp178 = or i64 %tmp99, 7
  %tmp179 = trunc i64 %tmp178 to i32
  br label %bb180

bb180:                                            ; preds = %bb177, %bb175
  %tmp181 = phi i64 [ %tmp176, %bb175 ], [ %tmp178, %bb177 ]
  %tmp182 = phi i32 [ 0, %bb175 ], [ %tmp179, %bb177 ]
  %tmp183 = phi float [ 0.000000e+00, %bb175 ], [ %tmp173, %bb177 ]
  %tmp184 = getelementptr inbounds float, float* %tmp12, i64 %tmp181
  %tmp185 = load float, float* %tmp184, align 4
  %tmp186 = fcmp fast olt float %tmp185, %tmp183
  br i1 %tmp186, label %bb187, label %bb190

bb187:                                            ; preds = %bb180
  br label %bb190

bb190:                                            ; preds = %bb187, %bb180
  %tmp191 = phi i32 [ 0, %bb187 ], [ %tmp182, %bb180 ]
  %tmp192 = phi float [ %tmp185, %bb187 ], [ %tmp183, %bb180 ]
  %tmp193 = add nuw nsw i64 %tmp98, 1
  %tmp194 = icmp eq i64 %tmp98, %tmp94
  br i1 %tmp194, label %bb195, label %bb95

bb195:                                            ; preds = %bb190
  %tmp196 = phi i32 [ %tmp191, %bb190 ]
  %tmp197 = phi float [ %tmp192, %bb190 ]
  br label %bb203

bb203:                                            ; preds = %bb215, %bb195
  %tmp204 = phi i32 [ %tmp196, %bb195 ], [ %tmp217, %bb215 ]
  %tmp205 = phi float [ %tmp197, %bb195 ], [ %tmp218, %bb215 ]
  %tmp206 = phi i64 [ 0, %bb195 ], [ %tmp216, %bb215 ]
  %tmp207 = getelementptr inbounds float, float* %tmp12, i64 %tmp206
  %tmp208 = load float, float* %tmp207, align 4
  %tmp209 = fcmp fast olt float %tmp208, %tmp205
  br i1 %tmp209, label %bb212, label %bb210

bb210:                                            ; preds = %bb203
  %tmp211 = add nuw nsw i64 %tmp206, 1
  br label %bb215

bb212:                                            ; preds = %bb203
  %tmp213 = add i64 %tmp206, 1
  %tmp214 = trunc i64 %tmp213 to i32
  br label %bb215

bb215:                                            ; preds = %bb212, %bb210
  %tmp216 = phi i64 [ %tmp211, %bb210 ], [ %tmp213, %bb212 ]
  %tmp217 = phi i32 [ %tmp204, %bb210 ], [ %tmp214, %bb212 ]
  %tmp218 = phi float [ %tmp205, %bb210 ], [ %tmp208, %bb212 ]
  %tmp219 = icmp eq i64 %tmp206, 0
  br i1 %tmp219, label %bb220, label %bb203

bb220:                                            ; preds = %bb215
  %tmp221 = phi i32 [ %tmp217, %bb215 ]
  store i32 %tmp221, i32* getelementptr inbounds ([1 x i32], [1 x i32]* @global, i64 0, i64 0), align 16
  ret void
}

attributes #0 = { "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"LTOPostLink", i32 1}
