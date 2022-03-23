; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; // #include <stdio.h>
; void foo(int n) {
; //   printf("FOO %d\n", n);
; }
; int two=2;
; int main() {
; //  #pragma omp parallel num_threads(5)
; //  #pragma omp single
;   {
;     #pragma omp task priority(1)
;       foo(1);
;
;     #pragma omp task priority(two)
;       foo(2);
;
;     #pragma omp task priority(two+14)
;       foo(16);
;   }
;   return 0;
; }
;

; Paropt emits code like this to set up the Priority field in the task thunk:
;
; %.task.alloc = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @.kmpc_loc.0.0, i32 %1, i32 161, i64 72, i64 0, i32 (i32, i8*)* bitcast (void (i32, %__struct.kmp_task_t_with_privates*)* @main.DIR.OMP.TASK.214 to i32 (i32, i8*)*))
; %.taskt.with.privates = bitcast i8* %.task.alloc to %__struct.kmp_task_t_with_privates*
; %.taskt = getelementptr inbounds %__struct.kmp_task_t_with_privates, %__struct.kmp_task_t_with_privates* %.taskt.with.privates, i32 0, i32 0
; %.priority.gep = getelementptr inbounds %__struct.kmp_task_t, %__struct.kmp_task_t* %.taskt, i32 0, i32 4
; store i64 1, i64* %.priority.gep, align 8  ; PRIORITY==1 in this example

; #pragma omp task priority(1)
;
; The kmp_task_t thunk is created with the "33" flag, a result of 0x01 | 0x20,
; where 0x01 is for tied tasks, and 0x20 indicates that priority is specified.
; CHECK: call i8* @__kmpc_omp_task_alloc({{.*}}i32 33
; The priority is inserted to field #4 of the thunk
; CHECK: [[PRIORITYGEP1:%[^ ]+]] = getelementptr{{.*}}struct.kmp_task_t{{.*}}i32 0, i32 4
; CHECK: store i64 1, i64* [[PRIORITYGEP1]]

; #pragma omp task priority(two)
;
; CHECK: [[LOAD2:%[^ ]+]] = load i32, i32* @two
; CHECK: call i8* @__kmpc_omp_task_alloc({{.*}}i32 33
; CHECK: [[PRIORITYGEP2:%[^ ]+]] = getelementptr{{.*}}struct.kmp_task_t{{.*}}i32 0, i32 4
; CHECK: [[PRIORITYCAST2:%[^ ]+]] = zext i32 [[LOAD2]] to i64
; CHECK: store i64 [[PRIORITYCAST2]], i64* [[PRIORITYGEP2]]

; #pragma omp task priority(two+14)
;
; CHECK: [[LOAD:%[^ ]+]] = load i32, i32* @two
; CHECK: [[ADD:%[^ ]+]] = add nsw i32 [[LOAD]], 14
; CHECK: call i8* @__kmpc_omp_task_alloc({{.*}}i32 33
; CHECK: [[PRIORITYGEP3:%[^ ]+]] = getelementptr{{.*}}struct.kmp_task_t{{.*}}i32 0, i32 4
; CHECK: [[PRIORITYCAST3:%[^ ]+]] = zext i32 [[ADD]] to i64
; CHECK: store i64 [[PRIORITYCAST3]], i64* [[PRIORITYGEP3]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@two = dso_local global i32 2, align 4

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3fooi(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.PRIORITY"(i32 1) ]
  call void @_Z3fooi(i32 1) #2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  %1 = load i32, i32* @two, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.PRIORITY"(i32 %1) ]
  call void @_Z3fooi(i32 2) #2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  %3 = load i32, i32* @two, align 4
  %add = add nsw i32 %3, 14
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.PRIORITY"(i32 %add) ]
  call void @_Z3fooi(i32 16) #2
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
