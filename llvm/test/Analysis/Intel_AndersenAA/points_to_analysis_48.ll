; This test verifies that "store i8 0, ptr %arrayidx11" is NOT removed by
; instcombine using Andersens's points-to analysis.
; AndersensAA shouldn't compute points-to of %call6 as empty by incorrectly
; modelling strtok library call when first argument of the call is nullptr.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: %call6 = call ptr @strtok(ptr noundef null,
; CHECK: %arrayidx11 = getelementptr inbounds i8, ptr %call6, i64 1
; CHECK: store i8 0, ptr %arrayidx11, align 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.main.string = private unnamed_addr constant [13 x i8] c"Language ,1\0A\00", align 1
@.str = private unnamed_addr constant [6 x i8] c"[%s]\0A\00", align 1

define dso_local i32 @main() local_unnamed_addr {
entry:
  %delims = alloca [2 x i8], align 1
  %call = call noalias dereferenceable_or_null(3072) ptr @malloc(i64 noundef 3072)
  %i = getelementptr inbounds [2 x i8], ptr %delims, i64 0, i64 0
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 1 dereferenceable(13) %call, ptr noundef nonnull align 1 dereferenceable(13) @__const.main.string, i64 13, i1 false)
  store i8 44, ptr %i, align 1
  %arrayidx2 = getelementptr inbounds [2 x i8], ptr %delims, i64 0, i64 1
  store i8 0, ptr %arrayidx2, align 1
  %call4 = call ptr @strtok(ptr noundef %call, ptr noundef nonnull %i)
  %cmp = icmp eq ptr %call4, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %call6 = call ptr @strtok(ptr noundef null, ptr noundef nonnull %i)
  %arrayidx11 = getelementptr inbounds i8, ptr %call6, i64 1
  store i8 0, ptr %arrayidx11, align 1
  br label %cleanup

cleanup:                                          ; preds = %if.end, %entry
  ret i32 0
}

declare dso_local noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr

declare dso_local ptr @strtok(ptr noundef, ptr nocapture noundef readonly) local_unnamed_addr

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

attributes #0 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
