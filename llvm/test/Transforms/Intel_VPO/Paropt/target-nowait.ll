; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-enable-async-helper-thread -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-enable-async-helper-thread -S %s | FileCheck %s


; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;   int Result[10];
;
; #pragma omp target map(Result) nowait
; {
;    Result[0] = omp_get_num_teams();
; }
;
; #pragma omp parallel num_threads(4)
; {
;   printf("tid = %d\n", omp_get_thread_num());
; }
; #pragma omp taskwait
;   printf("Result = %d .... \n", Result[0]);
; }

; The flag of 3rd argument in __kmpc_omp_task_alloc is expected to be 129.
; CHECK: call i8* @__kmpc_omp_task_alloc(%struct.ident_t* {{.*}}, i32 {{.*}}, i32 129, {{.*}}

; ModuleID = 'target-nowait.cpp'
source_filename = "target-nowait.cpp"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1
; @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: norecurse uwtable mustprogress
define dso_local i32 @main() #0 {
entry:
  %Result = alloca [10 x i32], align 16
  %0 = bitcast [10 x i32]* %Result to i8*
  call void @llvm.lifetime.start.p0i8(i64 40, i8* %0) #2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.TARGET.TASK"(), "QUAL.OMP.SHARED"([10 x i32]* %Result) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.NOWAIT"(), "QUAL.OMP.MAP.TOFROM"([10 x i32]* %Result, [10 x i32]* %Result, i64 40, i64 35, i8* null, i8* null) ]
  %call = call i32 @omp_get_num_teams() #2
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0, !intel-tbaa !4
  store i32 %call, i32* %arrayidx, align 16, !tbaa !4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 4) ]
  %call1 = call i32 @omp_get_thread_num() #2
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %call1) #2
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASKWAIT"() ]
  %arrayidx3 = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0, !intel-tbaa !4
  %5 = load i32, i32* %arrayidx3, align 16, !tbaa !4
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1, i64 0, i64 0), i32 %5)
  %6 = bitcast [10 x i32]* %Result to i8*
  call void @llvm.lifetime.end.p0i8(i64 40, i8* %6) #2
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind
declare dso_local i32 @omp_get_num_teams() #3

declare dso_local i32 @printf(i8*, ...) #4

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { norecurse uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 64770, i32 13746639, !"_Z4main", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 9.0.0"}
!4 = !{!5, !6, i64 0}
!5 = !{!"array@_ZTSA10_i", !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
