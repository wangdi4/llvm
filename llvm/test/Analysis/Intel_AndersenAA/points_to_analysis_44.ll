; This test verifies that t_printf routine is treated as wrapper to
; vsprintf by checking <universal> is NOT in points-to set of GVar<mem>.

; RUN: opt < %s -passes='require<anders-aa>' -disable-output -print-anders-points-to 2>&1 | FileCheck %s

; CHECK: [1] GVar<mem>   --> ({{[0-9]+}}): main:p

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__va_list_tag = type { i32, i32, i8*, i8* }

@pf_buf = internal global [1024 x i8] zeroinitializer, align 16
@.str.37 = private unnamed_addr constant [11 x i8] c">> START!\0A\00", align 1
@GVar = internal global i8* zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define internal void @t_printf(i8* nocapture noundef readonly %0, ...) #0 {
  %2 = alloca [1 x %struct.__va_list_tag], align 16
  %3 = bitcast [1 x %struct.__va_list_tag]* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %3) #28
  %4 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i64 0, i64 0
  call void @llvm.va_start(i8* nonnull %3)
  %5 = call i32 @vsprintf(i8* noundef getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i64 0, i64 0), i8* noundef %0, %struct.__va_list_tag* noundef nonnull %4) #28
  %6 = call i32 @xlate_nl_inplace(i8* noundef getelementptr inbounds ([1024 x i8], [1024 x i8]* @pf_buf, i64 0, i64 0)) #28
  call void @llvm.va_end(i8* nonnull %3)
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %3) #28
  ret void
}


define i32 @main() personality i8* undef {
  %p = call noalias i8* @malloc(i64 20)
  store i8* %p, i8** @GVar
  tail call void (i8*, ...) @t_printf(i8* getelementptr ([11 x i8], [11 x i8]* @.str.37, i64 0, i64 0), i8** getelementptr (i8*, i8** @GVar, i64 0))
  ret i32 1
}

declare dso_local noundef i32 @vsprintf(i8* nocapture noundef, i8* nocapture noundef readonly, %struct.__va_list_tag* noundef)
declare i32 @xlate_nl_inplace(i8* nocapture noundef %0)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.va_start(i8*)
declare void @llvm.va_end(i8*)
declare i8* @malloc(i64)
