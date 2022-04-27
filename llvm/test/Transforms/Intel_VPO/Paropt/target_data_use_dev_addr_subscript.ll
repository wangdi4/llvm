; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
; int main() {
;   int a[10];
;   a[0] = 111;
; //  int *b = &a[0];
; //  printf("%p, %d\n", &b[0], b[0]);
; //#pragma omp target data map(b[0:3])
;   {
; //    printf("%p, %d\n", &b[0], b[0]);
; #pragma omp target data use_device_addr(a[0])
;     {
;       printf("%p\n", &a[2]); // Should print &a[2] on device
;     }
;   }
; }
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 0
  store i32 111, i32* %arrayidx, align 16

  ; Check that the map created for %a has the correct map-type (64)
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 64]

; Check for the updated value of %a.
; CHECK: [[GEP:%[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK: call void @__tgt_target_data_begin({{.+}})
; CHECK: [[GEP_CAST:%[^ ]+]] = bitcast i8** [[GEP]] to [10 x i32]**
; CHECK: %a.updated.val = load [10 x i32]*, [10 x i32]** [[GEP_CAST]]

; Check that the updated value of %a is passed into the outlined function..
; CHECK: call void @main.DIR.OMP.TARGET.DATA{{[^ ]+}}([10 x i32]* %a.updated.val)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_ADDR:ARRSECT"([10 x i32]* %a, i64 1, i64 0, i64 1, i64 1) ]

  %arrayidx1 = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %arrayidx1) #1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
