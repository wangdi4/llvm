; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; CHECK: @main() local
; CHECK: %x = alloca
; CHECK: %task{{.*}}priv{{.*}} = alloca

; The %x alloca must not be moved into the parallel region, as the memory
; is written asynchronously by the tasks and must not be popped off the
; stack until all the tasks end (at the end of the program)

target triple = "x86_64-unknown-linux-gnu"

@gg = dso_local global i32 4, align 4

define dso_local i32 @main() local_unnamed_addr {
entry:
  %x = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %x)
  br label %DIR.OMP.PARALLEL.1.split

DIR.OMP.PARALLEL.1.split:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @gg, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.2.split

DIR.OMP.PARALLEL.2.split:                         ; preds = %DIR.OMP.PARALLEL.1.split
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @gg, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %x, i32 0, i32 1) ]
  %2 = load volatile i32, ptr @gg, align 4
  store i32 %2, ptr %x, align 4
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.PARALLEL.2.split
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %x)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
