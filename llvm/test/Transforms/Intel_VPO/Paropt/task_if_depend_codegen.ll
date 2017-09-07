; RUN: opt < %s -domtree -loops  -lcssa-verification  -loop-rotate -vpo-cfg-restructuring -vpo-wrncollection -vpo-wrninfo -vpo-paropt-prepare -simplifycfg  -sroa  -loops -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s

; This file tests the implementation of omp task if and depend clause.
; void fn10();
; int Arg;
; void foo() {
;  #pragma omp task if (Arg) depend(inout : Arg)
;  fn10();
; }

target triple = "x86_64-unknown-linux-gnu"

@Arg = common global i32 0, align 4
@.source.0.0 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private unnamed_addr constant { i32, i32, i32, i32, i8* } { i32 0, i32 2, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0, i32 0, i32 0) }

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %tid.val = tail call i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }* @.kmpc_loc.0.0)
  %0 = load i32, i32* @Arg, align 4, !tbaa !1
  br label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IF"(i32 %0), "QUAL.OMP.DEPEND.INOUT"(i32* @Arg) ]
  call void (...) @fn10()
  br label %DIR.OMP.END.TASK.1

DIR.OMP.END.TASK.1:                               ; preds = %DIR.OMP.TASK.3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare void @fn10(...) #2

declare i32 @__kmpc_global_thread_num({ i32, i32, i32, i32, i8* }*)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

; CHECK:  call void @__kmpc_omp_task_with_deps({{.*}})
; CHECK:  call void @__kmpc_omp_wait_deps({{.*}})
; CHECK:  call void @__kmpc_omp_task_begin_if0({{.*}})
; CHECK:  call void @__kmpc_omp_task_complete_if0({{.*}})
