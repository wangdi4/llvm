; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; It tests whether the OMP backend outlining supports the inreduction
; for arrays.
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr1 = external dso_local global [1000 x i32], align 16
@"@tid.addr" = external global i32

; Function Attrs: uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(), "QUAL.OMP.REDUCTION.MIN"([1000 x i32]* @arr1) ]
  br label %DIR.OMP.TASKGROUP.1

DIR.OMP.TASKGROUP.1:                              ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD"([1000 x i32]* @arr1) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.TASKGROUP.1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.INREDUCTION.MIN"([1000 x i32]* @arr1) ]
  call void @_Z3bari(i32 0)
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.PARALLEL.2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.6

DIR.OMP.END.PARALLEL.6:                           ; preds = %DIR.OMP.END.TASK.5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3bari(i32) #2

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
;
; CHECK:  %{{.*}} = call i8* @__kmpc_taskred_init({{.*}})
; CHECK:  %{{.*}} = call i8* @__kmpc_task_reduction_get_th_data({{.*}})
; CHECK:  call void @llvm.memcpy.p0i8.p0i8.i64({{.*}})
