; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefixes=FRCTRL1,ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefixes=FRCTRL1,ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-fast-reduction-ctrl=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefixes=FRCTRL0,ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0 -S %s | FileCheck %s --check-prefixes=FRCTRL0,ALL

; Test src:
;
; #include  <omp.h>
; #include <stdio.h>
;  int main(void) {
;  int N = 100, M = 10;
;  int i, x;
;
;  x = 0;
;  #pragma omp parallel num_threads(M) reduction(task, + : x)
;  {
;    x++;
;    #pragma omp single
;    for (i = 0; i < N; i++) {
;      #pragma omp task in_reduction(+ : x)
;      x++;
;      }
;  }
;  printf("x=%d =M+N\n", x); // x= 110 =M+N
;
;  return 0;
; }


; This tests checks the codegen of task modifier on a reduction clause.
; check the definitions of the task struct.
; ALL: %__struct.kmp_task_t_red_rec = type { %__struct.kmp_taskred_input_t }
; ALL: %__struct.kmp_taskred_input_t = type { ptr, ptr, i64, ptr, ptr, ptr, i32 }

; check that the definitions of task_red_init and @main_task_red_comb
; ALL: define internal void @main_task_red_init
; ALL: define internal void @main_task_red_comb

; check that we populate the fields of struct.kmp_task_t_red_rec and call kmpc_taskred_modifier_init before the call to kmpc_single
; ALL: define internal void @main.DIR.OMP.PARALLEL
; ALL: %taskt.red.rec = alloca %__struct.kmp_task_t_red_rec
; ALL: %x.red.struct = getelementptr inbounds %__struct.kmp_task_t_red_rec, ptr %taskt.red.rec, i32 0, i32 0
; ALL: %x.red.item = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 0
; CHECK-NEXT: store ptr %x.red, ptr %x.red.item
; CHECK-NEXT: store ptr %x.fast_red, ptr %x.red.item
; ALL: %x.red.orig = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 1
; CHECK-NEXT: store ptr %x.red, ptr %x.red.orig
; CHECK-NEXT: store ptr %x.fast_red, ptr %x.red.orig
; ALL: %x.red.size = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 2
; ALL: store i64 4, ptr %x.red.size
; ALL: %x.red.init = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 3
; ALL: store ptr @main_task_red_init{{.*}}, ptr %x.red.init
; ALL: %x.red.fini = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 4
; ALL: store ptr null, ptr %x.red.fini
; ALL: %x.red.comb = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 5
; ALL: store ptr @main_task_red_comb{{.*}}, ptr %x.red.comb
; ALL: %x.red.flags = getelementptr inbounds %__struct.kmp_taskred_input_t, ptr %x.red.struct, i32 0, i32 6
; ALL: store i32 0, ptr %x.red.flags
; ALL: %[[TID:[^,]+]] = load i32, ptr %tid
; ALL: %task.reduction.modifier.init = call ptr @__kmpc_taskred_modifier_init(ptr @.kmpc_loc{{.*}}, i32 %[[TID]], i32 0, i32 1, ptr %taskt.red.rec)
; FRCTRL1: call i32 @__kmpc_single

; check that kmpc_task_reduction_modifier_fini is inserted after kmpc_end_single and kmpc_barrier
; FRCTRL1: call void @__kmpc_end_single
; FRCTRL1: call void @__kmpc_barrier
; FRCTRL1: call void @__kmpc_task_reduction_modifier_fini(ptr @.kmpc_loc.{{.*}}, i32 {{.*}}, i32 0)

; check that the copying of the private reduction variable x.red to x.fast_red is inserted after calling kmpc_task_reduction_modifier_fini
; FRCTRL1: %[[X_RED:[^,]+]] = load i32, ptr %x.red
; FRCTRL1: store i32 %[[X_RED]], ptr %x.fast_red

; check that when -vpo-paropt-fast-reduction-ctrl=0, x.fast_red is the only version of the variable. Hence there should be no occurence of x.red between the calls __kmpc_taskred_modifier_init and __kmpc_task_reduction_modifier_fini. Eventually no copying from x.red to x.fast_red is expected after __kmpc_task_reduction_modifier_fini.
; FRCTRL0-NOT: x.red
; FRCTRL0: call void @__kmpc_task_reduction_modifier_fini(ptr @.kmpc_loc.{{.*}}, i32 {{.*}}, i32 0)
; FRCTRL0-NOT: %{{.*}} = load i32, ptr %x.red
; FRCTRL0-NOT: store i32 %{{.*}}, ptr %x.fast_red

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"x=%d =M+N\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %N = alloca i32, align 4
  %M = alloca i32, align 4
  %i = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 100, ptr %N, align 4
  store i32 10, ptr %M, align 4
  store i32 0, ptr %x, align 4
  %0 = load i32, ptr %M, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
     "QUAL.OMP.NUM_THREADS"(i32 %0),
     "QUAL.OMP.REDUCTION.ADD:TASK.TYPED"(ptr %x, i32 0, i32 1),
     "QUAL.OMP.SHARED:TYPED"(ptr %N, i32 0, i32 1),
     "QUAL.OMP.SHARED:TYPED"(ptr %i, i32 0, i32 1) ]

  %2 = load i32, ptr %x, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr %x, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]

  fence acquire
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %4 = load i32, ptr %i, align 4
  %5 = load i32, ptr %N, align 4
  %cmp = icmp slt i32 %4, %5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.INREDUCTION.ADD:TYPED"(ptr %x, i32 0, i32 1) ]

  %7 = load i32, ptr %x, align 4
  %inc1 = add nsw i32 %7, 1
  store i32 %inc1, ptr %x, align 4
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASK"() ]
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr %i, align 4
  %inc2 = add nsw i32 %8, 1
  store i32 %inc2, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  fence release
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %9 = load i32, ptr %x, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %9)
  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)
