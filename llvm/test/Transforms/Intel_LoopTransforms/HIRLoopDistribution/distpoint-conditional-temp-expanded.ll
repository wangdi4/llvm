;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check that %x.0 = 55; should be defined in the second loop.

; BEGIN REGION { }
;       + DO i1 = 0, 19, 1   <DO_LOOP>
;       |   %x.0 = 55;
;       |   if (%n > 10)
;       |   {
;       |      %x.0 = 0; <distribute_point>
;       |   }
;       |   %conv = sitofp.i32.double(i1 + %x.0);
;       |   (@A)[0][i1] = %conv;
;       + END LOOP
; END REGION

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   %x.0 = 55;
; CHECK:           + END LOOP

; CHECK:           + DO i1 = 0, 19, 1   <DO_LOOP>
; CHECK:           |   %x.0 = 55;
; CHECK:           |   if (%n > 10)
; CHECK:           |   {
; CHECK:           |      %x.0 = 0;
; CHECK:           |   }
; CHECK:           |   %conv = sitofp.i32.double(i1 + %x.0);
; CHECK:           |   (@A)[0][i1] = %conv;
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x double] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp sgt i32 %n, 10
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.DISTRIBUTE_POINT"() ]
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %x.0 = phi i32 [ 0, %if.then ], [ 55, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %add2 = add nuw nsw i32 %x.0, %1
  %conv = sitofp i32 %add2 to double
  %arrayidx = getelementptr inbounds [100 x double], ptr @A, i64 0, i64 %indvars.iv
  store double %conv, ptr %arrayidx, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

