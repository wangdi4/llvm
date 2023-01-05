; This test verifies that "store i8 0, i8* %arrayidx11" is NOT removed by
; instcombine using Andersens's points-to analysis.
; AndersensAA shouldn't compute points-to of %call6 as empty by incorrectly
; modelling strtok library call when first argument of the call is nullptr.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: %call6 = call i8* @strtok(i8* noundef null,
; CHECK: %arrayidx11 = getelementptr inbounds i8, i8* %call6, i64 1
; CHECK: store i8 0, i8* %arrayidx11, align 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.main.string = private unnamed_addr constant [13 x i8] c"Language ,1\0A\00", align 1
@.str = private unnamed_addr constant [6 x i8] c"[%s]\0A\00", align 1

define dso_local i32 @main() local_unnamed_addr {
entry:
  %delims = alloca [2 x i8], align 1
  %call = call noalias dereferenceable_or_null(3072) i8* @malloc(i64 noundef 3072)
  %i = getelementptr inbounds [2 x i8], [2 x i8]* %delims, i64 0, i64 0
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* noundef nonnull align 1 dereferenceable(13) %call, i8* noundef nonnull align 1 dereferenceable(13) getelementptr inbounds ([13 x i8], [13 x i8]* @__const.main.string, i64 0, i64 0), i64 13, i1 false)
  store i8 44, i8* %i, align 1
  %arrayidx2 = getelementptr inbounds [2 x i8], [2 x i8]* %delims, i64 0, i64 1
  store i8 0, i8* %arrayidx2, align 1
  %call4 = call i8* @strtok(i8* noundef %call, i8* noundef nonnull %i)
  %cmp = icmp eq i8* %call4, null
  br i1 %cmp, label %cleanup, label %if.end

if.end:                                           ; preds = %entry
  %call6 = call i8* @strtok(i8* noundef null, i8* noundef nonnull %i)
  %arrayidx11 = getelementptr inbounds i8, i8* %call6, i64 1
  store i8 0, i8* %arrayidx11, align 1
  br label %cleanup

cleanup:                                          ; preds = %if.end, %entry
  ret i32 0
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare dso_local noalias noundef i8* @malloc(i64 noundef) local_unnamed_addr

declare dso_local i8* @strtok(i8* noundef, i8* nocapture noundef readonly) local_unnamed_addr
