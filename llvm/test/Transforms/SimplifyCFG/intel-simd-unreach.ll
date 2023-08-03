; RUN: opt -passes=simplifycfg %s -S | FileCheck %s
; CHECK-NOT: call{{.*}}directive

; In the loop below, there is no path that reaches the directive.region.exit.
; All paths lead back to the loop header (infinite loop).
; SimplifyCFG was removing the exit, without removing the entry.
; This could also be solved by preserving both entry and exit, but this is less
; reliable due to the number of different situations that could lead to removal.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.a = type { i16, i16 }

@e = external dso_local local_unnamed_addr global i16, align 2
@_ZL1d = external hidden unnamed_addr constant [33 x %struct.a], align 16

define dso_local void @_Z1jv() local_unnamed_addr #0 {
entry:
  br label %region.0

if.then.i:                                        ; preds = %then.6, %then.25, %then.22
  %indvars.iv.lcssa = phi i64 [ %2, %then.22 ], [ %4, %then.25 ], [ 32, %then.6 ]
  %c.i = getelementptr inbounds %struct.a, ptr @_ZL1d, i64 %indvars.iv.lcssa, i32 1
  %0 = load i16, ptr %c.i, align 2
  store i16 %0, ptr @e, align 2
  br label %_Z1fP1ai.exit

_Z1fP1ai.exit:                                    ; preds = %ifmerge.6, %if.then.i
  ret void

region.0:                                         ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.VPO.AUTO.VEC"() ]
  br label %loop.20

loop.20:                                          ; preds = %ifmerge.25, %region.0
  %i1.i64.0 = phi i64 [ 0, %region.0 ], [ %nextivloop.20, %ifmerge.25 ]
  %2 = mul i64 2, %i1.i64.0
  %3 = getelementptr inbounds %struct.a, ptr @_ZL1d, i64 %2, i32 0
  %gepload = load i16, ptr %3, align 4
  %hir.cmp.22 = icmp ne i16 %gepload, 0
  br i1 %hir.cmp.22, label %then.22, label %ifmerge.22

then.22:                                          ; preds = %loop.20
  br label %if.then.i

ifmerge.22:                                       ; preds = %loop.20
  %4 = add i64 %2, 1
  %5 = getelementptr inbounds %struct.a, ptr @_ZL1d, i64 %4, i32 0
  %gepload6 = load i16, ptr %5, align 4
  %hir.cmp.25 = icmp ne i16 %gepload6, 0
  br i1 %hir.cmp.25, label %then.25, label %ifmerge.25

then.25:                                          ; preds = %ifmerge.22
  br label %if.then.i

ifmerge.25:                                       ; preds = %ifmerge.22
  %nextivloop.20 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.20 = icmp ne i64 %i1.i64.0, 15
; BOTH BRANCHES LEAD TO LOOP HEADER
  br i1 %condloop.20, label %loop.20, label %afterloop.20

afterloop.20:                                     ; preds = %ifmerge.25
  br i1 true, label %then.6, label %ifmerge.6

then.6:                                           ; preds = %afterloop.20
  br label %if.then.i

ifmerge.6:                                        ; preds = %afterloop.20
  call void @llvm.directive.region.exit(token %1) [ "DIR.VPO.END.AUTO.VEC"() ]
  br label %_Z1fP1ai.exit
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress nofree }
attributes #1 = { nounwind }


