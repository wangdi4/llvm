; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=true -S %s | FileCheck %s

; Test src:
;
; void foo() {
;   char a[1], b[2], c[6], d[8], e[12];
; #pragma omp parallel firstprivate(a, b, c, d, e)
;   ;
; }

; Check that pass-by-value optimization does not happen for arrays.

; CHECK: define internal void @foo{{.*}}PARALLEL{{.*}}(i32* %tid, i32* %bid, [12 x i8]* %e, [8 x i8]* %d, [6 x i8]* %c, [2 x i8]* %b, [1 x i8]* %a)
; CHECK:  %e.fpriv = alloca [12 x i8], align 1
; CHECK:  %d.fpriv = alloca [8 x i8], align 1
; CHECK:  %c.fpriv = alloca [6 x i8], align 1
; CHECK:  %b.fpriv = alloca [2 x i8], align 1
; CHECK:  %a.fpriv = alloca [1 x i8], align 1

; CHECK:  [[E_FP_CAST:%.+]] = bitcast [12 x i8]* %e.fpriv to i8*
; CHECK:  [[E_CAST:%.+]] = bitcast [12 x i8]* %e to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[E_FP_CAST]], i8* align 1 [[E_CAST]], i64 12, i1 false)

; CHECK:  [[D_FP_CAST:%.+]] = bitcast [8 x i8]* %d.fpriv to i8*
; CHECK:  [[D_CAST:%.+]] = bitcast [8 x i8]* %d to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[D_FP_CAST]], i8* align 1 [[D_CAST]], i64 8, i1 false)

; CHECK:  [[C_FP_CAST:%.+]] = bitcast [6 x i8]* %c.fpriv to i8*
; CHECK:  [[C_CAST:%.+]] = bitcast [6 x i8]* %c to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[C_FP_CAST]], i8* align 1 [[C_CAST]], i64 6, i1 false)
; CHECK:  br label %DIR.OMP.PARALLEL.22.split.split5

; CHECK:  [[B_FP_CAST:%.+]] = bitcast [2 x i8]* %b.fpriv to i8*
; CHECK:  [[B_CAST:%.+]] = bitcast [2 x i8]* %b to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[B_FP_CAST]], i8* align 1 [[B_CAST]], i64 2, i1 false)

; CHECK:  [[A_FP_CAST:%.+]] = bitcast [1 x i8]* %a.fpriv to i8*
; CHECK:  [[A_CAST:%.+]] = bitcast [1 x i8]* %a to i8*
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[A_FP_CAST]], i8* align 1 [[A_CAST]], i64 1, i1 false)

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
    "QUAL.OMP.FIRSTPRIVATE"([1 x i8]* %a),
    "QUAL.OMP.FIRSTPRIVATE"([2 x i8]* %b),
    "QUAL.OMP.FIRSTPRIVATE"([6 x i8]* %c),
    "QUAL.OMP.FIRSTPRIVATE"([8 x i8]* %d),
    "QUAL.OMP.FIRSTPRIVATE"([12 x i8]* %e) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
