; VLA reductions are not supported for TASKGROUP yet.
; UNSUPPORTED: true
; RUN: opt -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; It tests whether the OMP backend outlining supports the task group
; as well as the inreduction clause.
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZZ4mainE1a = internal global i32 0, align 4
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** %argv) #1 {
entry:
  %0 = zext i32 %argc to i64
  %1 = call i8* @llvm.stacksave()
  %vla = alloca i32, i64 %0, align 16
  br label %DIR.OMP.TASKGROUP.1

DIR.OMP.TASKGROUP.1:                              ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %vla) ]
  br label %DIR.OMP.TASKGROUP.2

DIR.OMP.TASKGROUP.2:                              ; preds = %DIR.OMP.TASKGROUP.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD"(i32* %vla) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.TASKGROUP.2
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.INREDUCTION.ADD"(i32* %vla) ]
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
  call void @llvm.stackrestore(i8* %1)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}

; CHECK:  %{{.*}} = call i8* @__kmpc_taskred_init({{.*}})
; CHECK:  %{{.*}} = call i8* @__kmpc_task_reduction_get_th_data({{.*}})
