; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s
; CHECK: define{{.*}}Z3foov
; CHECK: codeRepl:
; CHECK: %y1 = alloca
; CHECK: ptrtoint{{.*}}%y1
; CHECK: call{{.*}}kmpc_omp_task
; CHECK: define{{.*}}foo{{.*}}split
; CHECK: %y1 = alloca
; CHECK: %y1.gep = {{.*}}private
; CHECK: store{{.*}}1.000{{.*}}%y1.gep

; void foo()
;
; {
;  float y1; double b1;
;  #pragma omp task depend(out:y1)
;  {
;    y1 = 1;
;  }
; }

; Depend operand with no uses in the parent, gets wrapped into the task
; function. Code is generated on the parent's side with no definition.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() local_unnamed_addr {
entry:
  %y1 = alloca float, align 4
  %0 = bitcast float* %y1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %entry
  br label %DIR.OMP.TASK.2

DIR.OMP.TASK.2:                                   ; preds = %DIR.OMP.TASK.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DEPEND.OUT"(float* %y1), "QUAL.OMP.FIRSTPRIVATE"(float* %y1) ]
  br label %DIR.OMP.TASK.34

DIR.OMP.TASK.34:                                  ; preds = %DIR.OMP.TASK.2
  br label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %DIR.OMP.TASK.34
  store float 1.000000e+00, float* %y1, align 4
  br label %DIR.OMP.END.TASK.4.split

DIR.OMP.END.TASK.4.split:                         ; preds = %DIR.OMP.TASK.3
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.END.TASK.4.split
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)


!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
