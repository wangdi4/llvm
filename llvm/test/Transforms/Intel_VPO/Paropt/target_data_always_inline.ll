; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
; Original code:
; void foo(int *a, int *b, int *c) {
; #pragma omp target data map(a[:1])
;   {
; #pragma omp target enter data map(to:b[:1])
; #pragma omp target update to(c[:1])
; #pragma omp target exit data map(from:b[:1])
;   }
; }

; Verify that all four outlined functions are marked with alwaysinline:
; CHECK: define internal void @foo.{{.*}} #[[ATTR:[0-9]+]] {
; CHECK: define internal void @foo.{{.*}} #[[ATTR]] {
; CHECK: define internal void @foo.{{.*}} #[[ATTR]] {
; CHECK: define internal void @foo.{{.*}} #[[ATTR]] {
; CHECK: attributes #[[ATTR]] = {{{.*}}alwaysinline{{.*}}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* %a, i32* %b, i32* %c) #0 {
entry:
  %a.addr = alloca i32*, align 8
  %b.addr = alloca i32*, align 8
  %c.addr = alloca i32*, align 8
  store i32* %a, i32** %a.addr, align 8, !tbaa !3
  store i32* %b, i32** %b.addr, align 8, !tbaa !3
  store i32* %c, i32** %c.addr, align 8, !tbaa !3
  %0 = load i32*, i32** %a.addr, align 8
  %1 = load i32*, i32** %a.addr, align 8, !tbaa !3
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM"(i32* %0, i32* %arrayidx, i64 4, i64 3, i8* null, i8* null) ]
  %3 = load i32*, i32** %b.addr, align 8
  %4 = load i32*, i32** %b.addr, align 8, !tbaa !3
  %arrayidx1 = getelementptr inbounds i32, i32* %4, i64 0
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(), "QUAL.OMP.MAP.TO"(i32* %3, i32* %arrayidx1, i64 4, i64 1, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]
  %6 = load i32*, i32** %c.addr, align 8
  %7 = load i32*, i32** %c.addr, align 8, !tbaa !3
  %arrayidx2 = getelementptr inbounds i32, i32* %7, i64 0
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(), "QUAL.OMP.MAP.TO"(i32* %6, i32* %arrayidx2, i64 4, i64 1, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET.UPDATE"() ]
  %9 = load i32*, i32** %b.addr, align 8
  %10 = load i32*, i32** %b.addr, align 8, !tbaa !3
  %arrayidx3 = getelementptr inbounds i32, i32* %10, i64 0
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(), "QUAL.OMP.MAP.FROM"(i32* %9, i32* %arrayidx3, i64 4, i64 2, i8* null, i8* null) ]
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"clang version 12.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPi", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
