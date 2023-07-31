; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; Verify that we don't incorrectly eliminate %t5.out by moving the use in
; %shl21.i before definition of %t5.

; CHECK: + DO i1 = 0, 245, 1   <DO_LOOP>
; CHECK: |   %t5.out = %t5;
; CHECK: |   if (%t5 < 128)
; CHECK: |   {
; CHECK: |      %t5 = %t5  <<  1;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %shl21.i = 2 * %t5.out  ^  256;
; CHECK: |      %t5 = %shl21.i  ^  (@alpha_to)[0][8];
; CHECK: |   }
; CHECK: |   (@alpha_to)[0][i1 + 9] = %t5;
; CHECK: |   (@index_of)[0][%t5] = i1 + 9;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@alpha_to = external hidden unnamed_addr global [256 x i32], align 16
@index_of = external hidden unnamed_addr global [256 x i32], align 16

define void @foo(i32 %t4) {
entry:
  br label %for.body12.i

for.body12.i:                                     ; preds = %if.end31.i, %entry
  %t5 = phi i32 [ %t4, %entry ], [ %xor22.sink.i, %if.end31.i ]
  %indvars.iv.i = phi i64 [ 9, %entry ], [ %indvars.iv.next.i, %if.end31.i ]
  %cmp15.i = icmp slt i32 %t5, 128
  br i1 %cmp15.i, label %if.else.i, label %if.then16.i

if.then16.i:                                      ; preds = %for.body12.i
  %t6 = load i32, ptr getelementptr inbounds ([256 x i32], ptr @alpha_to, i64 0, i64 8), align 16
  %xor20.i = shl i32 %t5, 1
  %shl21.i = xor i32 %xor20.i, 256
  %xor22.i = xor i32 %shl21.i, %t6
  br label %if.end31.i

if.else.i:                                        ; preds = %for.body12.i
  %shl28.i = shl i32 %t5, 1
  br label %if.end31.i

if.end31.i:                                       ; preds = %if.else.i, %if.then16.i
  %xor22.sink.i = phi i32 [ %shl28.i, %if.else.i ], [ %xor22.i, %if.then16.i ]
  %t7 = getelementptr inbounds [256 x i32], ptr @alpha_to, i64 0, i64 %indvars.iv.i
  store i32 %xor22.sink.i, ptr %t7, align 4
  %idxprom34.i = sext i32 %xor22.sink.i to i64
  %arrayidx35.i = getelementptr inbounds [256 x i32], ptr @index_of, i64 0, i64 %idxprom34.i
  %t8 = trunc i64 %indvars.iv.i to i32
  store i32 %t8, ptr %arrayidx35.i, align 4
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.i = icmp eq i64 %indvars.iv.next.i, 255
  br i1 %exitcond.i, label %generate_gf.exit, label %for.body12.i

generate_gf.exit:
  ret void
}
