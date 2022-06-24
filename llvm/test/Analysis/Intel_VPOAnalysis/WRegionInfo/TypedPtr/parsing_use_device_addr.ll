; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; Test src: Input IR was hand modified because FE does not yet handle use_device_addr
; #include <stdio.h>
; int main() {
;   int a[10];
;   int *array_device = &a[0];
;   printf("%p\n", &array_device[0]);
; #pragma omp parallel num_threads(1)
;   {
; #pragma omp target data use_device_ptr(array_device)
;     {
;      printf("%p\n", &array_device[0]);
;     } // end target data
;   } // end parallel
; }
;
; Check that use_device_addr was parsed as a use_device_ptr
; Check for the debug string.
; CHECK: USE_DEVICE_PTR clause (size=1): {{.*}}
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %array_device = alloca i32*, align 8
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 0
  store i32* %arrayidx, i32** %array_device, align 8
  %0 = load i32*, i32** %array_device, align 8
  %arrayidx1 = getelementptr inbounds i32, i32* %0, i64 0
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %arrayidx1)
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 1), "QUAL.OMP.SHARED"(i32** %array_device) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  %2 = load i32*, i32** %array_device, align 8
  br label %DIR.OMP.TARGET.DATA.4

DIR.OMP.TARGET.DATA.4:                            ; preds = %DIR.OMP.PARALLEL.3
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_ADDR:PTR_TO_PTR"(i32** %array_device), "QUAL.OMP.MAP.TOFROM"(i32* %2, i32* %2, i64 0, i64 96) ]
  br label %DIR.OMP.TARGET.DATA.5

DIR.OMP.TARGET.DATA.5:                            ; preds = %DIR.OMP.TARGET.DATA.4
  %4 = load i32*, i32** %array_device, align 8
  %arrayidx2 = getelementptr inbounds i32, i32* %4, i64 0
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %arrayidx2) #2
  br label %DIR.OMP.END.TARGET.DATA.6

DIR.OMP.END.TARGET.DATA.6:                        ; preds = %DIR.OMP.TARGET.DATA.5
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET.DATA"() ]
  br label %DIR.OMP.END.TARGET.DATA.7

DIR.OMP.END.TARGET.DATA.7:                        ; preds = %DIR.OMP.END.TARGET.DATA.6
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.END.TARGET.DATA.7
  ret i32 0
}

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
