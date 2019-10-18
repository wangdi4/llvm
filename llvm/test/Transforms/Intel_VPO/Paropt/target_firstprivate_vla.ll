; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s
;
; Test Src:
;
; #include <stdio.h>
;
; double *yptr;
;
; __attribute__((noinline)) void foo(int n) {
;   double y[n];
;   yptr = (double*) y;
;   y[2] = 2;
;
; #pragma omp target firstprivate(y) map(to:yptr)
;   {
;     printf("y[2] = %f, yptr = %p, y = %p\n", y[2], (int*) yptr, (int*) y);
;     y[2] = 0;
;   }
;   printf("y[2] = %f (expected = %f)\n", y[2], 2.0);
; }
;
; int main() {
;   foo(10);
; }
;
; ModuleID = 'target_firstprivate_vla.c'
source_filename = "target_firstprivate_vla.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@yptr = common dso_local global double* null, align 8
@.str = private unnamed_addr constant [30 x i8] c"y[2] = %f, yptr = %p, y = %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [27 x i8] c"y[2] = %f (expected = %f)\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8

  %vla = alloca double, i64 %1, align 16 ; VLA with size %1

  store double* %vla, double** @yptr, align 8
  %arrayidx = getelementptr inbounds double, double* %vla, i64 2
  store double 2.000000e+00, double* %arrayidx, align 16

; Check that VLA size is captured.
; CHECK: store i64 %1, i64* [[SIZE_ADDR:%[a-zA-Z._0-9]+]]

; Check that the captured size expression is sent in to an outlined
; function for the target construct.
; CHECK: call void [[OUTLINE_FUNCTION:@__omp_offloading.+foo.+]](double** {{.+}}, i64* [[SIZE_ADDR]], double* %vla)
; Check that inside the outlined function, the captured size is loaded
; and used in allocating a local VLA for %vla
; CHECK: define internal void [[OUTLINE_FUNCTION]](double** {{.*}}, i64* [[SIZE_ADDR_ARG:[^ ]+]], double* {{.+}})
; CHECK: [[VLA_SIZE_VAL:[^ ]+]] = load i64, i64* [[SIZE_ADDR_ARG]]
; CHECK: {{.+}} = alloca double, i64 [[VLA_SIZE_VAL]]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(double* %vla), "QUAL.OMP.MAP.TO"(double** @yptr) ]
  %arrayidx1 = getelementptr inbounds double, double* %vla, i64 2
  %4 = load double, double* %arrayidx1, align 16
  %5 = load double, double* %arrayidx1, align 16
  %6 = load double*, double** @yptr, align 8
  %7 = bitcast double* %6 to i32*
  %8 = bitcast double* %vla to i32*
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([30 x i8], [30 x i8]* @.str, i64 0, i64 0), double %5, i32* %7, i32* %8)
  %arrayidx2 = getelementptr inbounds double, double* %vla, i64 2
  store double 0.000000e+00, double* %arrayidx2, align 16
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]


  %arrayidx3 = getelementptr inbounds double, double* %vla, i64 2
  %9 = load double, double* %arrayidx3, align 16
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([27 x i8], [27 x i8]* @.str.1, i64 0, i64 0), double %9, double 2.000000e+00)
  %10 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %10)
  ret void
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  call void @foo(i32 10)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 2051, i32 123125353, !"foo", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
