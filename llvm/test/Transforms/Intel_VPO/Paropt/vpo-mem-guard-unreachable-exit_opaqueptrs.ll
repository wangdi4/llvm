; RUN: opt -pass-remarks-missed=openmp -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s 2>&1 | FileCheck %s

;; Check that unmatched "DIR.VPO.GUARD.MEM.MOTION" is removed, together with
;; DIR.OMP.SIMD.

; CHECK: OpenMP simd loop does not have a reachable exit
; CHECK-NOT: call{{.*}}directive.region.entry
; CHECK-NOT: call{{.*}}directive.region.entry

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main() {
codeRepl:
  %call = tail call noundef i32 @_Z3foov()
  %i.linear.iv.i = alloca i32, align 4
  %x = alloca float, align 4
  %sub1 = add nsw i32 %call, -1
  %.omp.ub.val.zext = zext i32 %sub1 to i64
  %.capture_expr.0.val.zext = zext i32 %call to i64
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:INSCAN.TYPED"(ptr %x, float 0.000000e+00, i32 1, i64 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv.i, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.GUARD.BEGIN

DIR.OMP.GUARD.BEGIN:
  %pre.scan.guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(),
    "QUAL.OMP.LIVEIN"(ptr %x) ]
  br label %DIR.OMP.SIMD.127

DIR.OMP.SIMD.127:
  unreachable

__Z4main_l5.exit:
  br label %DIR.OMP.END.TARGET.821

DIR.OMP.END.TARGET.821:
  ret i32 0
}

declare i32 @_Z3foov()

declare token @llvm.directive.region.entry()
