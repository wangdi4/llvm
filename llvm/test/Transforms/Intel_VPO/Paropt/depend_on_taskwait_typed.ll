; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -opaque-pointers -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=OPQPTR %s
; RUN: opt -opaque-pointers -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=OPQPTR %s

; Original code:
; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;  int Result[10];
;  int a;
;  #pragma omp taskwait depend(in: a)
;   printf("Result = %d .... \n", Result[0]);
;}

; CHECK: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %__struct.kmp_depend_info, %__struct.kmp_depend_info* %{{.*}}, i32 0, i32 1
; CHECK: store i64 4, i64* [[DEPNUMBYTES]]
; CHECK:  call void @__kmpc_omp_wait_deps(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 1, i8* %{{.*}}, i32 0, i8* null)
; CHECK:  call void @__kmpc_omp_taskwait(%struct.ident_t* @{{.*}}, i32 %{{.*}})

; OPQPTR: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %__struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 1
; OPQPTR: store i64 4, ptr [[DEPNUMBYTES]]
; OPQPTR:  call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 1, ptr %{{.*}}, i32 0, ptr null)
; OPQPTR:  call void @__kmpc_omp_taskwait(ptr @{{.*}}, i32 %{{.*}})

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %Result = alloca [10 x i32], align 16
  %a = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(), "QUAL.OMP.DEPEND.IN:TYPED"(i32* %a, i32 0, i32 1) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKWAIT"() ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %Result, i64 0, i64 0
  %1 = load i32, i32* %arrayidx, align 16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i64 0, i64 0), i32 %1)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"clang version 13.0.0"}
