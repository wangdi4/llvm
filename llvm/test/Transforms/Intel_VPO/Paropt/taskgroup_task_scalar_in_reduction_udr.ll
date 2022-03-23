; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
; #include <stdio.h>
;
; void my_init(short* in, short* orig) {*in = 10; printf("init\n");}
; #pragma omp declare reduction (my_add: short : omp_out = omp_out + omp_in) initializer(my_init(&omp_priv, &omp_orig))
;
; short a;
; void foo() {
; //#pragma omp parallel num_threads(10)
; //#pragma omp single
; #pragma omp taskgroup task_reduction(my_add:a)
;   {
; #pragma omp task in_reduction(my_add:a)
;     {
; //      printf("task\n");
;       a += 1;
;     }
;   }
; }
;
; //int main() {
; //    a = 0;
; //    foo();
; //    printf("%d\n", a);
; //    return 0;
; //}

; Check that the shared struct has space for @a, and
; reduction item struct type is defined.
; CHECK: %__struct.kmp_privates.t = type {}
; CHECK: %__struct.kmp_taskred_input_t = type { i8*, i8*, i64, i8*, i8*, i8*, i32 }
; CHECK: %__struct.shared.t = type { i16* }

; Check that the reduction struct is correctly initialized
; using the size of i16, and @a
; CHECK: [[RED_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 0
; CHECK: store i8* bitcast (i16* @a to i8*), i8** [[RED_ITEM_PTR_GEP]], align 8
; CHECK: [[ORIG_ITEM_PTR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 1
; CHECK: store i8* bitcast (i16* @a to i8*), i8** [[ORIG_ITEM_PTR_GEP]], align 8
; CHECK: [[RED_ITEM_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 2
; CHECK: store i64 2, i64* [[RED_ITEM_SIZE_GEP]]
; CHECK: [[RED_INIT_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 3
; CHECK: store i8* bitcast (void (i16*, i16*)* @[[RED_INIT_FUNC:[^ ]+task_red_init[^ ]+]] to i8*), i8** [[RED_INIT_GEP]], align 8
; CHECK: [[RED_FINI_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 4
; CHECK: store i8* null, i8** [[RED_FINI_GEP]], align 8
; CHECK: [[RED_COMB_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_taskred_input_t, %__struct.kmp_taskred_input_t* %{{[^ ,]+}}, i32 0, i32 5
; CHECK: store i8* bitcast (void (i16*, i16*)* @[[RED_COMB_FUNC:[^ ]+task_red_comb[^ ]+]] to i8*), i8** %a.red.comb, align 8

; Check that the replacement var for the reduction operand is the correct type
; CHECK: [[A_RED:%[^ ]+]] = call i8* @__kmpc_task_reduction_get_th_data{{.+}}
; CHECK: [[A_RED_CAST:%[^ ]+]] = bitcast i8* [[A_RED]] to i16*

; Check that the initializer/combiner functions are as expected.
; CHECK: define internal void @[[RED_INIT_FUNC]](i16* [[PRIV_ARG:%[^ ]+]], i16* [[ORIG_ARG:%[^ ]+]]) {
; CHECK: call void @.omp_initializer.(i16* [[PRIV_ARG]], i16* [[ORIG_ARG]])

; CHECK: define internal void @[[RED_COMB_FUNC]](i16* [[RHS:%[^ ]+]], i16* [[LHS:%[^ ]+]]) {
; CHECK: call void @.omp_combiner.(i16* [[RHS]], i16* [[LHS]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"init\0A\00", align 1
@a = dso_local global i16 0, align 2

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3foov() #2 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(), "QUAL.OMP.REDUCTION.UDR"(i16* @a, i8* null, i8* null, void (i16*, i16*)* @.omp_combiner., void (i16*, i16*)* @.omp_initializer.) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.INREDUCTION.UDR"(i16* @a, i8* null, i8* null, void (i16*, i16*)* @.omp_combiner., void (i16*, i16*)* @.omp_initializer.) ]

  %2 = load i16, i16* @a, align 2
  %conv = sext i16 %2 to i32
  %add = add nsw i32 %conv, 1
  %conv1 = trunc i32 %add to i16
  store i16 %conv1, i16* @a, align 2

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: noinline uwtable
declare void @.omp_combiner.(i16* noalias %0, i16* noalias %1) #4

; Function Attrs: noinline uwtable
declare void @.omp_initializer.(i16* noalias %0, i16* noalias %1) #4

attributes #0 = { noinline optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { noinline uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
