; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

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
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  br label %DIR.OMP.PARALLEL.1.split

DIR.OMP.PARALLEL.1.split:                         ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(i32* @gg),
    "QUAL.OMP.SHARED"(i32* %x) ]
  br label %DIR.OMP.PARALLEL.2.split

DIR.OMP.PARALLEL.2.split:                         ; preds = %DIR.OMP.PARALLEL.1.split
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED"(i32* @gg),
    "QUAL.OMP.SHARED"(i32* %x) ]
  %3 = load volatile i32, i32* @gg, align 4
  store i32 %3, i32* %x, align 4
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.PARALLEL.2.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret i32 0
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
