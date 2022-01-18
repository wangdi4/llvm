; INTEL_CUSTOMIZATION
; CMPLRLLVM-33102
; end INTEL_CUSTOMIZATION
; The consecutive parallel-if and parallel regions are causing the dominator
; tree to be rebuilt incorrectly.

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,verify' -vpo-paropt-enable-outline-verification=true -vpo-paropt-strict-outline-verification=true -S %s 2>&1 | FileCheck %s

; CHECK-NOT: DominatorTree is different
; CHECK-LABEL: codeRepl
; CHECK: br i1 true, label %if.then, label %if.else
; CHECK-LABEL: if.then:
; CHECK: call{{.*}}kmpc_fork_call
; CHECK-LABEL: if.else:
; CHECK: call{{.*}}Z1bv{{.*}}PARALLEL
; CHECK-LABEL: if.end:
; CHECK: br label %DIR.OMP.PARALLEL.6

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z1bv() local_unnamed_addr #0 {
DIR.OMP.PARALLEL.3.split:
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.3.split
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.IF"(i1 true) ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.6

DIR.OMP.PARALLEL.6:                               ; preds = %DIR.OMP.END.PARALLEL.8, %DIR.OMP.END.PARALLEL.4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.PARALLEL.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.6, !llvm.loop !1
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 13.0.0"}
!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.mustprogress"}
