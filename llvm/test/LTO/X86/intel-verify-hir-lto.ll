; Verify that HIR is run in LTO mode

; RUN: llvm-as < %s > %t1
; RUN: llvm-lto -debug-pass=Arguments %t1 2>&1 | FileCheck %s

; CHECK: -hir-framework

;Module Before HIR; ModuleID = 'printf.c'
source_filename = "printf.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"hello world\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr "loopopt-pipeline"="full" {
entry:
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0))
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr 


