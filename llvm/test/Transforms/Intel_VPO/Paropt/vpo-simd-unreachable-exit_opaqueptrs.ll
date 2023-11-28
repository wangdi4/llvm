; RUN: opt -pass-remarks-missed=openmp -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s 2>&1 | FileCheck %s
; CHECK: OpenMP simd loop does not have a reachable exit
; CHECK-NOT: call{{.*}}directive.region.entry

; This is a UB loop (no exit) that made it into paropt. Check that it's
; detected and a warning is printed.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr #0 {
codeRepl:
  %call = tail call noundef i32 @_Z3foov()
  %i.linear.iv.i = alloca i32, align 4
  %sub1 = add nsw i32 %call, -1
  %.omp.ub.val.zext = zext i32 %sub1 to i64
  %.capture_expr.0.val.zext = zext i32 %call to i64
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() #0 [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv.i, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.127

DIR.OMP.SIMD.127:
  unreachable

__Z4main_l5.exit:
  br label %DIR.OMP.END.TARGET.821

DIR.OMP.END.TARGET.821:
  ret i32 0
}

declare dso_local noundef i32 @_Z3foov() local_unnamed_addr #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

attributes #0 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
