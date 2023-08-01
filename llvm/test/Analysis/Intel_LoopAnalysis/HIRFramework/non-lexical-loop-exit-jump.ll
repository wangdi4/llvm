; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-cost-model-throttling=0 -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to handle bottom tests where the loop exit jumps to a non-lexical successor.
; This is done by moving the loop exit goto [ goto bb145; ] after the loop.

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   bb123:
; CHECK: |   %tmp.out = &((%tmp)[0]);
; CHECK: |   %tmp125 = &((%tmp.out)[0]);
; CHECK: |
; CHECK: |   + UNKNOWN LOOP i2
; CHECK: |   |   <i2 = 0>
; CHECK: |   |   bb124:
; CHECK: |   |   %tmp125.out = &((%tmp125)[0]);
; CHECK: |   |   %tmp134 = (%tmp125.out)[0].3;
; CHECK: |   |   %tmp125 = &((%tmp134)[0]);
; CHECK: |   |   if (&((%tmp134)[0]) != &((undef)[0]))
; CHECK: |   |   {
; CHECK: |   |      <i2 = i2 + 1>
; CHECK: |   |      goto bb124;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   goto bb145;
; CHECK: |   Unused.{{[0-9]+}}:
; CHECK: |   %tmp = &((%tmp125.out)[0]);
; CHECK: |   if (&((%tmp125.out)[0]) != &((undef)[0]))
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto bb123;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.wobble = type { i64, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i64, i64, i32, i32 }
%struct.snork = type { i32, i64, ptr, ptr, i16, ptr, ptr, i64, i64 }

; Function Attrs: norecurse nounwind uwtable
define void @snork(i32 %c) local_unnamed_addr {
bb:
  br i1 undef, label %bb448, label %bb120

bb120:                                            ; preds = %bb119
  %cond = icmp sgt i32 %c, 0
  br i1 %cond, label %bb122, label %bb121

bb121:                                            ; preds = %bb120
  br label %bb123

bb122:                                            ; preds = %bb120
  br label %bb448

bb123:                                            ; preds = %bb143, %bb121
  %tmp = phi ptr [ %tmp137, %bb143 ], [ undef, %bb121 ]
  br label %bb124

bb124:                                            ; preds = %bb132, %bb123
  %tmp125 = phi ptr [ %tmp, %bb123 ], [ %tmp134, %bb132 ]
  br i1 %cond, label %bb136, label %bb132

bb132:                                            ; preds = %bb129, %bb124
  %tmp133 = getelementptr inbounds %struct.wobble, ptr %tmp125, i64 0, i32 3
  %tmp134 = load ptr, ptr %tmp133, align 8
  %tmp135 = icmp eq ptr %tmp134, undef
  br i1 %tmp135, label %bb145, label %bb124

bb136:                                            ; preds = %bb124
  %tmp137 = phi ptr [ %tmp125, %bb124 ]
  br label %bb143

bb143:                                            ; preds = %bb136
  %tmp144 = icmp eq ptr %tmp137, undef
  br i1 %tmp144, label %bb146, label %bb123

bb145:                                            ; preds = %bb132
  br label %bb448

bb146:                                            ; preds = %bb143
  br label %bb448

bb448:                                            ; preds = %bb446, %bb67, %bb
  ret void
}
