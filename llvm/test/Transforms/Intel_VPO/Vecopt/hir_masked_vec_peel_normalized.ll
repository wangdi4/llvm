; RUN: opt < %s -disable-output \
; RUN:     -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' \
; RUN:     -vplan-vec-scenario='m4;v4;m4'

; REQUIRES: asserts

; Test to check that we don't crash when vectorizing peel with a normalized loop.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %A) {
entry:
  %entry.token = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %if.end, %entry
  %iv = phi i64 [ 1, %entry ], [ %iv.next, %if.end ]
  %cond = call i1 @cond()
  br i1 %cond, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  store i32 0, ptr %A, align 8
  br label %if.end

if.else:                                          ; preds = %for.body
  store i32 0, ptr %A, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 1024
  br i1 %exitcond, label %exit, label %for.body

exit:                                             ; preds = %if.end
  call void @llvm.directive.region.exit(token %entry.token) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare i1 @cond() #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }
