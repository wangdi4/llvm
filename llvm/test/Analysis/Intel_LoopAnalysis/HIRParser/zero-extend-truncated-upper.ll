; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Check parsing output for the loop verifying that we conservatively zero extend an upper (zext.i2.i32()) which has a truncate it in.

; CHECK: + DO i1 = 0, zext.i2.i32((-1 + (-1 * trunc.i32.i2(%spec.select.i)))), 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK: |   (%ptr)[0].1[i1 + %spec.select.i] = 0;
; CHECK: + END LOOP

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.layer_data = type { ptr, [2048 x i8], ptr, [16 x i8], i32, ptr, i32, i32, [64 x i32], [64 x i32], [64 x i32], [64 x i32], i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [12 x [64 x i16]] }
%struct.ee_file_t = type { ptr, [4096 x i8], i32, ptr, ptr, ptr, i32, i32, i32, i32, i32 }

define void @foo(i32 %n, ptr %ptr) {
entry:
  %t = icmp sgt i32 %n, 0
  %spec.select.i = select i1 %t, i32 %n, i32 0
  br label %while.body.i74.preheader

while.body.i74.preheader:                         ; preds = %if.then3.i
  br label %while.body.i74


while.body.i74:                                   ; preds = %while.body.i74, %while.body.i74.preheader
  %Buffer_Level.139.i = phi i32 [ %inc.i, %while.body.i74 ], [ %spec.select.i, %while.body.i74.preheader ]
  %inc.i = add nuw nsw i32 %Buffer_Level.139.i, 1
  %arrayidx.i = getelementptr inbounds %struct.layer_data, ptr %ptr, i32 0, i32 1, i32 %Buffer_Level.139.i
  store i8 0, ptr %arrayidx.i, align 1
  %and.i = and i32 %inc.i, 3
  %tobool7.i = icmp eq i32 %and.i, 0
  br i1 %tobool7.i, label %postexit, label %while.body.i74

postexit:
  br label %exit

exit:
 ret void
}

