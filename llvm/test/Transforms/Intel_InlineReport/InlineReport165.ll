; RUN: opt -passes='vpo-paropt,cgscc(inline)' -disable-output -inline-report=0x2f847 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,vpo-paropt,cgscc(inline),inlinereportemitter' -disable-output -inline-report=0x2f8c6 < %s 2>&1 | FileCheck %s

; Tests below are run with inline report compacting enabled, to test that
; the feature does not lead to a compfail.

; Check that INLINE of myfoo into _Z3foov.DIR.OMP.TASK.2.split is recognized
; for the classic and metadata inlining reports, and that myfoo is dead static
; eliminated.

; CHECK: DEAD STATIC FUNC: myfoo
; CHECK: COMPILE FUNC: _Z3foov
; CHECK: BROKER: __kmpc_omp_task_alloc(_Z3foov.DIR.OMP.TASK.2.split)
; CHECK: COMPILE FUNC: _Z3foov.DIR.OMP.TASK.2.split
; CHECK: DELETE: llvm.directive.region.entry
; CHECK: INLINE: myfoo {{.*}}Callee has single callsite and local linkage
; CHECK: DELETE: llvm.directive.region.exit

; void foo()
;
; {
;  float y1; double b1;
;  #pragma omp task firstprivate(y1)
;  {
;    y1 = 1;
;    myfoo();
;  }
; }

; Firstprivate operand with no uses in the parent, gets wrapped into the task
; function. Code is generated on the parent's side with no definition.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal void @myfoo() {
  ret void
}

define dso_local void @_Z3foov() local_unnamed_addr {
entry:
  %y1 = alloca float, align 4
  %0 = bitcast ptr %y1 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %0)
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %entry
  br label %DIR.OMP.TASK.2

DIR.OMP.TASK.2:                                   ; preds = %DIR.OMP.TASK.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y1, float 0.000000e+00, i32 1) ]

  br label %DIR.OMP.TASK.34

DIR.OMP.TASK.34:                                  ; preds = %DIR.OMP.TASK.2
  br label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %DIR.OMP.TASK.34
  store float 1.000000e+00, ptr %y1, align 4
  call void @myfoo()
  br label %DIR.OMP.END.TASK.4.split

DIR.OMP.END.TASK.4.split:                         ; preds = %DIR.OMP.TASK.3
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.END.TASK.4.split
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]

  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %0)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
