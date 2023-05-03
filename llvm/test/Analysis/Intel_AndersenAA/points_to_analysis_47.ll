; This test verifies that t_printf routine is treated as wrapper to
; vsprintf by checking <universal> is NOT in points-to set of GVar<mem>.
; This is same points_to_analysis_44.ll except vsprintf function
; is called through "al_sprintf" simple wrapper function.

; RUN: opt < %s -passes='require<anders-aa>' -disable-output -print-anders-points-to 2>&1 | FileCheck %s

; CHECK: [1] GVar<mem>   --> ({{[0-9]+}}): main:p

; REQUIRES: asserts

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.__va_list_tag = type { i32, i32, ptr, ptr }

@pf_buf = internal global [1024 x i8] zeroinitializer, align 16
@.str.37 = private unnamed_addr constant [11 x i8] c">> START!\0A\00", align 1
@GVar = internal global ptr null, align 16

define internal fastcc i32 @al_sprintf(ptr nocapture noundef %arg, ptr nocapture noundef readonly %arg1, ptr noundef %arg2) {
bb:
  %i = tail call i32 @vsprintf(ptr noundef %arg, ptr noundef %arg1, ptr noundef %arg2)
  ret i32 %i
}

define internal void @t_printf(ptr nocapture noundef readonly %arg, ...) {
bb:
  %i = alloca [1 x %struct.__va_list_tag], align 16
  %i1 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %i)
  %i2 = getelementptr inbounds [1 x %struct.__va_list_tag], ptr %i, i64 0, i64 0
  call void @llvm.va_start(ptr nonnull %i2)
  %i3 = call i32 @al_sprintf(ptr noundef @pf_buf, ptr noundef %arg, ptr noundef nonnull %i2)
  %i4 = call i32 @xlate_nl_inplace(ptr noundef @pf_buf)
  call void @llvm.va_end(ptr nonnull %i2)
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %i)
  ret void
}

define i32 @main() personality ptr undef {
bb:
  %p = call noalias ptr @malloc(i64 20)
  store ptr %p, ptr @GVar, align 8
  tail call void (ptr, ...) @t_printf(ptr @.str.37, ptr @GVar)
  ret i32 1
}

declare dso_local noundef i32 @vsprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ptr noundef)

declare i32 @xlate_nl_inplace(ptr nocapture noundef)

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start(ptr) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end(ptr) #0

declare ptr @malloc(i64)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nocallback nofree nosync nounwind willreturn }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
