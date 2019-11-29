; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int (&a)[10]) {
; #pragma omp task firstprivate(a)
;   {
;     a[1] += 1;
;     printf("%d\n", a[1]);
;   }
; }
;
;  //int main() {
;  //  int a[10];
;  //  a[1] = 0;
;  //  foo(a);
;  //  return 0;
;  //}

; ModuleID = 'task_array_byref_fp.cpp'
source_filename = "task_array_byref_fp.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; Check for the space allocated for the private copy.
; CHECK: %__struct.kmp_privates.t = type { [10 x i32] }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z3fooRA10_i([10 x i32]* dereferenceable(40) %a) #0 {
entry:
  %a.addr = alloca [10 x i32]*, align 8
  store [10 x i32]* %a, [10 x i32]** %a.addr, align 8
  %0 = load [10 x i32]*, [10 x i32]** %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE:BYREF"([10 x i32]** %a.addr) ]
; For the outlined function for the task, check that the address of the local
; copy for '%a.addr' is stored to an [10 x i32]**, and then that is used instead
; of '%a.addr' in the region.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: [[A_PRIVATE:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, %__struct.kmp_privates.t* %{{[^ ]+}}, i32 0, i32 0
; CHECK: store [10 x i32]* [[A_PRIVATE]], [10 x i32]** [[A_PRIVATE_ADDR:%[^ ]+]]

; CHECK: [[A_PRIVATE_ADDR_LOAD:%[^ ]+]] = load [10 x i32]*, [10 x i32]** [[A_PRIVATE_ADDR]]
; CHECK: [[A_PRIVATE_ADDR_LOAD_GEP:%[^ ]+]] = getelementptr inbounds [10 x i32], [10 x i32]* [[A_PRIVATE_ADDR_LOAD]], i64 0, i64 1
; CHECK: [[A_PRIVATE_ADDR_LOAD_GEP_LOAD:%[^ ]+]] = load i32, i32* [[A_PRIVATE_ADDR_LOAD_GEP]]
; CHECK: %{{[^ ]+}} = add nsw i32 [[A_PRIVATE_ADDR_LOAD_GEP_LOAD]], 1

  %2 = load [10 x i32]*, [10 x i32]** %a.addr, align 8
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 1
  %3 = load i32, i32* %arrayidx, align 4
  %add = add nsw i32 %3, 1

  store i32 %add, i32* %arrayidx, align 4
  %4 = load [10 x i32]*, [10 x i32]** %a.addr, align 8
  %arrayidx1 = getelementptr inbounds [10 x i32], [10 x i32]* %4, i64 0, i64 1
  %5 = load i32, i32* %arrayidx1, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %5)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
