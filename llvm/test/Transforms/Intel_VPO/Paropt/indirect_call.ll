; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
; The test is to make sure paropt does not crash when it
; encounters an indirect call.
;
; #include <stdio.h>
;
; extern int bar();
; void foo() {
;   int (*bar_ptr)(void) = bar;
;
; #pragma omp critical
;   printf("%d\n", bar_ptr());
; }
;

source_filename = "indirect_call.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@"@tid.addr" = external global i32
@.gomp_critical_user_.var = common global [8 x i32] zeroinitializer
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr global { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }
@.source.0.0.1 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0.2 = private unnamed_addr global { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.1, i32 0, i32 0) }

define dso_local void @foo() {
entry:
  %bar_ptr = alloca i32 ()*, align 8
  store i32 ()* bitcast (i32 (...)* @bar to i32 ()*), i32 ()** %bar_ptr, align 8
  %my.tid = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_critical({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0, i32 %my.tid, [8 x i32]* @.gomp_critical_user_.var)
; CHECK: call void @__kmpc_critical
  fence acquire
  %0 = load i32 ()*, i32 ()** %bar_ptr, align 8
  %call = call i32 %0()
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %call)
  fence release
  %my.tid2 = load i32, i32* @"@tid.addr", align 4
  call void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0.2, i32 %my.tid2, [8 x i32]* @.gomp_critical_user_.var)
; CHECK: call void @__kmpc_end_critical
  ret void
}

declare dso_local i32 @bar(...)
declare dso_local i32 @printf(i8*, ...)
declare void @__kmpc_critical({ i32, i32, i32, i32, i8* }*, i32, [8 x i32]*)
declare void @__kmpc_end_critical({ i32, i32, i32, i32, i8* }*, i32, [8 x i32]*)

