; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that we correctly remove redundant switch cases after complete unroll. We were not taking into account the truncation trunc.i32.i3() in the switch condition when returning the constant which resulted in incorrect switch case removal.

; CHECK: Function

; CHECK: + DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK: |   switch(i1)
; CHECK: |   {
; CHECK: |   case 0:
; CHECK: |      %puts18 = @puts(&((@str.11)[0]));
; CHECK: |      break;
; CHECK: |   case 1:
; CHECK: |      %puts17 = @puts(&((@str.10)[0]));
; CHECK: |      break;
; CHECK: |   case 2:
; CHECK: |      %puts16 = @puts(&((@str.9)[0]));
; CHECK: |      break;
; CHECK: |   case -3:
; CHECK: |      %puts15 = @puts(&((@str.8)[0]));
; CHECK: |      break;
; CHECK: |   case -2:
; CHECK: |      %puts14 = @puts(&((@str.7)[0]));
; CHECK: |      break;
; CHECK: |   default:
; CHECK: |      %puts19 = @puts(&((@str.12)[0]));
; CHECK: |      break;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Function

; CHECK: %puts18 = @puts(&((@str.11)[0]));
; CHECK: %puts17 = @puts(&((@str.10)[0]));
; CHECK: %puts16 = @puts(&((@str.9)[0]));
; CHECK: %puts19 = @puts(&((@str.12)[0]));
; CHECK: %puts19 = @puts(&((@str.12)[0]));
; CHECK: %puts15 = @puts(&((@str.8)[0]));
; CHECK: %puts14 = @puts(&((@str.7)[0]));
; CHECK: %puts19 = @puts(&((@str.12)[0]));

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"0\0A\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"1\0A\00", align 1
@.str.2 = private unnamed_addr constant [3 x i8] c"2\0A\00", align 1
@.str.3 = private unnamed_addr constant [3 x i8] c"5\0A\00", align 1
@.str.4 = private unnamed_addr constant [3 x i8] c"6\0A\00", align 1
@.str.5 = private unnamed_addr constant [9 x i8] c"default\0A\00", align 1
@.str.6 = private unnamed_addr constant [7 x i8] c"Done.\0A\00", align 1
@str = private unnamed_addr constant [6 x i8] c"Done.\00"
@str.7 = private unnamed_addr constant [2 x i8] c"6\00"
@str.8 = private unnamed_addr constant [2 x i8] c"5\00"
@str.9 = private unnamed_addr constant [2 x i8] c"2\00"
@str.10 = private unnamed_addr constant [2 x i8] c"1\00"
@str.11 = private unnamed_addr constant [2 x i8] c"0\00"
@str.12 = private unnamed_addr constant [8 x i8] c"default\00"

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %c.020 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %trunc = trunc i32 %c.020 to i3
  switch i3 %trunc, label %sw.default [
    i3 0, label %sw.bb
    i3 1, label %sw.bb1
    i3 2, label %sw.bb3
    i3 -3, label %sw.bb5
    i3 -2, label %sw.bb7
  ]

sw.bb:                                            ; preds = %for.body
  %puts18 = tail call i32 @puts(ptr @str.11)
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %puts17 = tail call i32 @puts(ptr @str.10)
  br label %for.inc

sw.bb3:                                           ; preds = %for.body
  %puts16 = tail call i32 @puts(ptr @str.9)
  br label %for.inc

sw.bb5:                                           ; preds = %for.body
  %puts15 = tail call i32 @puts(ptr @str.8)
  br label %for.inc

sw.bb7:                                           ; preds = %for.body
  %puts14 = tail call i32 @puts(ptr @str.7)
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %puts19 = tail call i32 @puts(ptr @str.12)
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.bb3, %sw.bb5, %sw.bb7, %sw.default
  %inc = add nuw nsw i32 %c.020, 1
  %exitcond = icmp eq i32 %inc, 8
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  %puts = tail call i32 @puts(ptr @str)
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @puts(ptr nocapture readonly)

