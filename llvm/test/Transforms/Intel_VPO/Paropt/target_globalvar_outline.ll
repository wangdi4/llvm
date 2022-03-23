; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; unsigned int NY = 100;
;
; void lbm_1() {
; #pragma omp target teams
;   {
;     NY += 2;
;     printf("NY= %d \n", NY); // should print 102
;   }
;   return;
; }
;
; int main() {
;   lbm_1();
;   return 0;
; }

; ModuleID = 'test_2.target.cpp'
source_filename = "test_2.target.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@NY = external global i32, align 4
@.str = private unnamed_addr constant [9 x i8] c"NY= %d \0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define hidden void @_Z5lbm_1v() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* @NY) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32* @NY) ]

; Teams Construct
; CHECK: define internal void @_Z5lbm_1v.DIR.OMP.TEAMS.{{.*}}(i32* %tid, i32* %bid, i32* %NY)
; CHECK: [[LOAD:%[0-9]+]] = load i32, i32* %NY, align 4
; CHECK: %add = add i32 [[LOAD]], 2
; CHECK: store i32 %add, i32* %NY, align 4

; Target Construct
; CHECK: define internal void @__omp_offloading_{{.*}}__Z5lbm_1v_l7(i64 {{.*}})
; CHECK: {{.*}}.fpriv = alloca i32, align 1

  %2 = load i32, i32* @NY, align 4, !tbaa !5
  %add = add i32 %2, 2
  store i32 %add, i32* @NY, align 4, !tbaa !5
  %3 = load i32, i32* @NY, align 4, !tbaa !5
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32 %3) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 64768, i32 11346413, !"_Z5lbm_2v", i32 18, i32 1, i32 0}
!1 = !{i32 0, i32 64768, i32 11346413, !"_Z5lbm_1v", i32 7, i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{!"clang version 10.0.0"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}

