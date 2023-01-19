; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo(int *a) {
; #pragma omp target data map(a[:1])
;   ;
; }

; Verify that the outlined function for target data is marked with alwaysinline:
; CHECK: define internal void @foo.{{.*}} #[[ATTR:[0-9]+]] {
; CHECK: attributes #[[ATTR]] = {{{.*}}alwaysinline{{.*}}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

define dso_local void @foo(i32* %a) {
entry:
  %a.addr = alloca i32*, align 8
  store i32* %a, i32** %a.addr, align 8
  %0 = load i32*, i32** %a.addr, align 8
  %1 = load i32*, i32** %a.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 0

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM"(i32* %0, i32* %arrayidx, i64 4, i64 3, i8* null, i8* null) ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
