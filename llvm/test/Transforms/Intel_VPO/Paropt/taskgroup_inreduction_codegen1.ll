; VLA reductions are not supported for TASKGROUP yet.
; UNSUPPORTED: true
; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; It tests whether the OMP backend outlining supports the task group
; as well as the inreduction clause.
; CHECK:  %{{.*}} = call ptr @__kmpc_taskred_init({{.*}})
; CHECK:  %{{.*}} = call ptr @__kmpc_task_reduction_get_th_data({{.*}})

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZZ4mainE1a = internal global i32 0, align 4
@"@tid.addr" = external global i32

define dso_local void @_Z3foov() {
entry:
  ret void
}

define dso_local i32 @main(i32 %argc, ptr %argv) {
entry:
  %0 = zext i32 %argc to i64
  %1 = call ptr @llvm.stacksave()
  %vla = alloca i32, i64 %0, align 16
  br label %DIR.OMP.TASKGROUP.1

DIR.OMP.TASKGROUP.1:                              ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %0) ]

  br label %DIR.OMP.TASKGROUP.2

DIR.OMP.TASKGROUP.2:                              ; preds = %DIR.OMP.TASKGROUP.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %0) ]

  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.TASKGROUP.2
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.INREDUCTION.ADD:TYPED"(ptr %vla, i32 0, i64 %0) ]

  call void @_Z3foov()
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.6

DIR.OMP.END.TASK.6:                               ; preds = %DIR.OMP.END.TASK.5
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.7

DIR.OMP.END.PARALLEL.7:                           ; preds = %DIR.OMP.END.TASK.6
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASKGROUP"() ]
  call void @llvm.stackrestore(ptr %1)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64, ptr nocapture)

declare ptr @llvm.stacksave()

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.stackrestore(ptr)

declare void @llvm.lifetime.end.p0(i64, ptr nocapture)
