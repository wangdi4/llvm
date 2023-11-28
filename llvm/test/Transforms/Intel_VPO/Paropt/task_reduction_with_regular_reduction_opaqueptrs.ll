; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
;#include  <omp.h>
;#include <stdio.h>
;int main(void) {
;  int N = 100, M = 10;
;  int i, x, y, z;
;
;  x = 0;
;  y = 0;
;  z = 0;
;  #pragma omp parallel num_threads(M) reduction (+:z) reduction(task, + : x, y)
;  {
;    x++;
;    y++;
;    z++;
;   #pragma omp single
;    for (i = 0; i < N; i++) {
;      #pragma omp task in_reduction(+ : x, y)
;      x++;
;      y++;
;      }
;  }
;  //printf("x=%d =M+N\n", x); // x= 110 =M+N
;  //printf("y=%d \n", y); // y= 110 =M+N
;  //printf("z=%d \n", z); // z= 10 =M+N
;
;  return 0;
;}

; check that when we have a task reduction (on variables x,y) mixed with a regular reduction on some other variable (z), the regular reduction variable (z) is not put in the task-reduction struct. so in this example %struct.fast_red_t will have 3 i32 members one for each variable (x, y and z), howerver %__struct.kmp_task_t_red_rec has two member of type %__struct.kmp_taskred_input_t. The members of %__struct.kmp_taskred_input_t are initialized with x and y,  and sent to @__kmpc_taskred_modifier_init.
; this test also checks that if we have multiple task reduction variables (x and y) we generate only 1 call to __kmpc_taskred_modifier_init and 1 call to __kmpc_task_reduction_modifier_fini

; CHECK: %struct.fast_red_t = type <{ i32, i32, i32 }>
; CHECK: %__struct.kmp_task_t_red_rec = type { %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t }
; CHECK: %__struct.kmp_taskred_input_t = type { ptr, ptr, i64, ptr, ptr, ptr, i32 }

; CHECK: %taskt.red.rec = alloca %__struct.kmp_task_t_red_rec
; CHECK: %x.red.struct = getelementptr inbounds %__struct.kmp_task_t_red_rec, ptr %taskt.red.rec, i32 0, i32 0
; CHECK: %x.red.item = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 0
; CHECK: store ptr %x.red, ptr %x.red.item
; CHECK: %y.red.struct = getelementptr inbounds %__struct.kmp_task_t_red_rec, ptr %taskt.red.rec, i32 0, i32 1
; CHECK: %y.red.item = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %y.red.struct, i32 0, i32 0
; CHECK: store ptr %y.red, ptr %y.red.item
; CHECK: %task.reduction.modifier.init = call ptr @__kmpc_taskred_modifier_init(ptr @.kmpc_loc.{{.*}}, i32 %{{.*}}, i32 0, i32 2, ptr %taskt.red.rec)
; CHECK-NOT: %task.reduction.modifier.init = call ptr @__kmpc_taskred_modifier_init
; CHECK: call void @__kmpc_task_reduction_modifier_fini(ptr @.kmpc_loc.{{.*}}, i32 %{{.*}}, i32 0)
; CHECK-NOT: call void @__kmpc_task_reduction_modifier_fini
; CHECK: %{{.*}} = load i32, ptr %z.red
; CHECK: store i32 %{{.*}}, ptr %z.fast_red
; CHECK: %{{.*}} = load i32, ptr %x.red
; CHECK: store i32 %{{.*}}, ptr %x.fast_red
; CHECK: %{{.*}} = load i32, ptr %y.red
; CHECK: store i32 %{{.*}}, ptr %y.fast_red

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %N = alloca i32, align 4
  %M = alloca i32, align 4
  %i = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 100, ptr %N, align 4
  store i32 10, ptr %M, align 4
  store i32 0, ptr %x, align 4
  store i32 0, ptr %y, align 4
  store i32 0, ptr %z, align 4
  %0 = load i32, ptr %M, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.NUM_THREADS"(i32 %0),
     "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %z, i32 0, i32 1),
     "QUAL.OMP.REDUCTION.ADD:TASK.TYPED"(ptr %x, i32 0, i32 1),
     "QUAL.OMP.REDUCTION.ADD:TASK.TYPED"(ptr %y, i32 0, i32 1),
     "QUAL.OMP.SHARED:TYPED"(ptr %N, i32 0, i32 1),
     "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1) ]

  %2 = load i32, ptr %x, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr %x, align 4
  %3 = load i32, ptr %y, align 4
  %inc1 = add nsw i32 %3, 1
  store i32 %inc1, ptr %y, align 4
  %4 = load i32, ptr %z, align 4
  %inc2 = add nsw i32 %4, 1
  store i32 %inc2, ptr %z, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]

  fence acquire
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %6 = load i32, ptr %i, align 4
  %7 = load i32, ptr %N, align 4
  %cmp = icmp slt i32 %6, %7
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.INREDUCTION.ADD:TYPED"(ptr %x, i32 0, i32 1),
     "QUAL.OMP.INREDUCTION.ADD:TYPED"(ptr %y, i32 0, i32 1) ]

  %9 = load i32, ptr %x, align 4
  %inc3 = add nsw i32 %9, 1
  store i32 %inc3, ptr %x, align 4
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASK"() ]
  %10 = load i32, ptr %y, align 4
  %inc4 = add nsw i32 %10, 1
  store i32 %inc4, ptr %y, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %11 = load i32, ptr %i, align 4
  %inc5 = add nsw i32 %11, 1
  store i32 %inc5, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  fence release
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
