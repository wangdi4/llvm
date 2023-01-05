; Tests that the two GVN limit options, will disable hoisting and GVN
; redundancy elimination.

; RUN: opt -passes="gvn-hoist,gvn" -S -gvn-max-inst-x-bb=1 -gvn-hoist-max-inst-x-bb=1 %s | FileCheck %s

; CHECK-LABEL: if.then
; CHECK: add nsw i32 %a, %b
; CHECK-NEXT: add nsw i32 %a, %b
; CHECK-LABEL: if.else:
; CHECK: add nsw i32 %a, %b
; CHECK-NEXT: add nsw i32 %a, %b

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local i32 @_Z3fooiii(i32 %a, i32 %b, i32 %cond) #0 {
entry:
  %tobool = icmp ne i32 %cond, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %add = add nsw i32 %a, %b
  %add2 = add nsw i32 %a, %b
  br label %if.end

if.else:                                          ; preds = %entry
  %add1 = add nsw i32 %a, %b
  %add3 = add nsw i32 %a, %b
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %c.0 = phi i32 [ %add2, %if.then ], [ %add3, %if.else ]
  ret i32 %c.0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)


!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
