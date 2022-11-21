; RUN: opt -passes="gvn" -S %s | FileCheck %s

; Test load/store motion around VPO directives.
; In the function "guard", the GUARD directive references x.red, and should
; block motion through the top and bottom of the region.

; The function "simd_private" has a SIMD PRIVATE clause and should also
; block motion in the same way.

; In the function "par_shared", we should not have undeclared live-in live-out
; from the region, such as %f or %tmp.
; The SHARED operands should prevent propagation of values that they alias.

; CHECK-LABEL: guard
; CHECK-LABEL: DIR.VPO.GUARD.MEM.MOTION.3:
; CHECK: load float, ptr %x.red
; CHECK-LABEL: DIR.OMP.END.SIMD.6:
; CHECK: load float, ptr %x.red

; CHECK-LABEL: simd_private
; CHECK-LABEL: DIR.OMP.END.SIMD.6:
; CHECK: load float, ptr %x.red

; CHECK-LABEL: par_shared
; CHECK-LABEL: DIR.OMP.PAR.1:
; CHECK: load float, ptr %x
; CHECK-LABEL: DIR.OMP.END.PAR.6:
; CHECK: load float, ptr %x
; CHECK: load float, ptr %y

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local float @guard() local_unnamed_addr #0 {
DIR.OMP.SIMD.131:
  %x.red = alloca float, align 4
  store float 6.0, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.131
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %DIR.VPO.GUARD.MEM.MOTION.3

DIR.VPO.GUARD.MEM.MOTION.3:                       ; preds = %DIR.OMP.SIMD.1
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(),
    "QUAL.OMP.LIVEIN"(ptr %x.red) ]
  %load1 = load float, ptr %x.red, align 4
  br label %DIR.OMP.SCAN.5

DIR.OMP.SCAN.5:                                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.3
  br label %DIR.OMP.END.SCAN.7

DIR.OMP.END.SCAN.7:                               ; preds = %DIR.OMP.SCAN.5
  store float 5.0, ptr %x.red, align 4
  br label %DIR.VPO.END.GUARD.MEM.MOTION.9

DIR.VPO.END.GUARD.MEM.MOTION.9:                   ; preds = %DIR.OMP.END.SCAN.7
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.OMP.END.SIMD.6

DIR.OMP.END.SIMD.6:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.9
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.SIMD"() ]
  %load2 = load float, ptr %x.red, align 4
  %rez = fadd float %load1, %load2
  ret float %rez
}

define dso_local float @simd_private() local_unnamed_addr #0 {
DIR.OMP.SIMD.131:
  %x.red = alloca float, align 4
  store float 6.0, ptr %x.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.131
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x.red, i32 0, i32 1)]
  store float 5.0, ptr %x.red, align 4
  br label %DIR.OMP.END.SIMD.6

DIR.OMP.END.SIMD.6:
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.SIMD"() ]
  %load1 = load float, ptr %x.red, align 4
  ret float %load1
}

define dso_local float @par_shared(float %f) local_unnamed_addr #0 {
DIR.OMP.PAR.131:
  %x = alloca float, align 4
  %y = alloca float, align 4
  store float %f, ptr %x, align 4
  br label %DIR.OMP.PAR.1

DIR.OMP.PAR.1:                                   ; preds = %DIR.OMP.SIMD.131
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y, i32 0, i32 1)]
  %tmp = load float, ptr %x, align 4
  store float %tmp, ptr %y, align 4
  br label %DIR.OMP.END.PAR.6

DIR.OMP.END.PAR.6:
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.PARALLEL"() ]
  %load1 = load float, ptr %x, align 4
  %load2 = load float, ptr %y, align 4
  %rez = fadd float %load1, %load2
  ret float %rez
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
