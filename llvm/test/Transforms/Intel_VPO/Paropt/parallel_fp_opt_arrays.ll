; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s

; Test src:
;
; void foo() {
;   char a[1], b[2], c[6], d[8], e[12];
; #pragma omp parallel firstprivate(a, b, c, d, e)
;   ;
; }

; Check that pass-by-value optimization only happens for [1 x i8] array
; (because the TYPED clause for it is identical to a scalar i8).

; This can be optimized later to expand to types like [2 x i8] and [4 x i8] that
; can legally be loaded/stored as i16/i32 etc.

; CHECK: define internal void @foo{{.*}}PARALLEL{{.*}}(ptr %tid, ptr %bid, ptr %e, ptr %d, ptr %c, ptr %b, i64 %a.val.zext)
; CHECK:  %e.fpriv = alloca [12 x i8], align 1
; CHECK:  %d.fpriv = alloca [8 x i8], align 1
; CHECK:  %c.fpriv = alloca [6 x i8], align 1
; CHECK:  %b.fpriv = alloca [2 x i8], align 1
; CHECK:  %a.fpriv = alloca i8, align 1

; CHECK:  %e.fpriv.gep = getelementptr inbounds [12 x i8], ptr %e.fpriv, i32 0, i32 0
; CHECK:  %d.fpriv.gep = getelementptr inbounds [8 x i8], ptr %d.fpriv, i32 0, i32 0
; CHECK:  %c.fpriv.gep = getelementptr inbounds [6 x i8], ptr %c.fpriv, i32 0, i32 0
; CHECK:  %b.fpriv.gep = getelementptr inbounds [2 x i8], ptr %b.fpriv, i32 0, i32 0

; CHECK:  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %e.fpriv.gep, ptr align 1 %e, i64 12, i1 false)
; CHECK:  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %d.fpriv.gep, ptr align 1 %d, i64 8, i1 false)
; CHECK:  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %c.fpriv.gep, ptr align 1 %c, i64 6, i1 false)
; CHECK:  call void @llvm.memcpy.p0.p0.i64(ptr align 1 %b.fpriv.gep, ptr align 1 %b, i64 2, i1 false)

; CHECK:  %a.val.zext.trunc = trunc i64 %a.val.zext to i8
; CHECK:  store i8 %a.val.zext.trunc, ptr %a.fpriv, align 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %a = alloca [1 x i8], align 1
  %b = alloca [2 x i8], align 1
  %c = alloca [6 x i8], align 1
  %d = alloca [8 x i8], align 1
  %e = alloca [12 x i8], align 1

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a, i8 0, i64 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %b, i8 0, i64 2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %c, i8 0, i64 6),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %d, i8 0, i64 8),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %e, i8 0, i64 12) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
