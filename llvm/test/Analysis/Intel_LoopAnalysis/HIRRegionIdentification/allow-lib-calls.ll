; RUN: opt < %s -passes="print<hir-region-identification>" -disable-output 2>&1 | FileCheck %s

; CHECK: Region 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@stdout = external dso_local local_unnamed_addr global ptr, align 8

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32 %n) {
entry:
  %0 = load ptr, ptr @stdout, align 8
  %tobool = icmp eq i32 %n, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret i32 1000

for.body:                                         ; preds = %if.end, %entry
  %i.08 = phi i32 [ 0, %entry ], [ %inc1, %if.end ]
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %call = tail call i32 @fflush(ptr %0)
  br label %if.end

if.end:                                           ; preds = %for.body, %if.then
  %inc1 = add nuw nsw i32 %i.08, 1
  %exitcond = icmp eq i32 %inc1, 1000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind
declare dso_local i32 @fflush(ptr nocapture)

