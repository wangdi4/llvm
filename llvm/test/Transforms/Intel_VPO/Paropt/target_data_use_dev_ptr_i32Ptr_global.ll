; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test src:
;
; int a = 10;
; int main() {
;   printf("%p\n", &a);
;   #pragma omp target data use_device_addr(a)
;   //#pragma omp target has_device_addr(a)
;     printf("%p\n", &a);
; }

; Check for the map type/size for the operand.
; CHECK: @.offload_sizes = private unnamed_addr constant [1 x i64] zeroinitializer
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 64]

; Check that we cast the gobal @a, and map it
; CHECK: %[[A_CAST1:[^ ]+]] = bitcast i8* bitcast (i32* @a to i8*) to i32*
; CHECK: %[[A_CAST2:[^ ]+]] = bitcast i32* %[[A_CAST1]] to i8*
; CHECK: %[[A_MAP_GEP:[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK: store i8* %[[A_CAST2]], i8** %[[A_MAP_GEP]], align 8
; CHECK: call void @__tgt_target_data_begin({{.+}})

; Check that the updated version of @a is passed into the outlined region for target data.
; CHECK: %[[A_MAP_GEP_CAST:[^ ]+]] = bitcast i8** %[[A_MAP_GEP]] to i32**
; CHECK: %[[A_UPDATED:[^ ]+]] = load i32*, i32** %[[A_MAP_GEP_CAST]], align 8
; CHECK: call void @[[OUTLINED_FUNC:main.DIR.OMP.TARGET.DATA[^ ]*]](i32* %[[A_UPDATED]])

; Check that the passed-in value of A is used in the outlined function.
; CHECK-DAG: define internal void @[[OUTLINED_FUNC]](i32* %[[A_PASS:[^ ]+]])
; CHECK: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %[[A_PASS]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = dso_local global i32 10, align 4
@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* @a)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_PTR"(i32* @a) ]

  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* @a) #2

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret i32 0
}

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
