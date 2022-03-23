; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check reduction with bool and operation + m128i type.
; #include <stdio.h>
; #include <xmmintrin.h>
; #include <limits.h>
;
; int
; main() {
;
;   __m128i v3;
;   int i;
;
;   v3 = _mm_setr_epi32(INT_MAX, INT_MAX, INT_MAX, INT_MAX);
;   __m128i v4 = _mm_setr_epi32(0, 0, 1, 0);
;
; #pragma omp parallel for reduction(&&: v3)
;   for( i = 0; i < 3; i++ ) {
;     v3 = v3 && v4;
;   }
;   int *test = (int *)&v3;
;   int *ref = (int *)&v4;
;   for (i=0; i<4; i++) {
;     if ( test[i] != ref[i]) {
;     printf("test[%d]=%d, ref[%d]=%d\n",i, test[i], i, ref[i]);
;     printf("FAILED\n");
;       return 3;
;     }
;   }
;     printf("PASSED\n");
;   return 0;
; }


; ModuleID = 'reduction_bool_and_m128i.cpp'
source_filename = "reduction_bool_and_m128i.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"test[%d]=%d, ref[%d]=%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca <2 x i64>, align 16
  %i = alloca i32, align 4
  %v4 = alloca <2 x i64>, align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %test = alloca i32*, align 8
  %ref = alloca i32*, align 8
  store i32 0, i32* %retval, align 4
  %call = call <2 x i64> @_ZL14_mm_setr_epi32iiii(i32 2147483647, i32 2147483647, i32 2147483647, i32 2147483647)
  store <2 x i64> %call, <2 x i64>* %v3, align 16
  %call1 = call <2 x i64> @_ZL14_mm_setr_epi32iiii(i32 0, i32 0, i32 1, i32 0)
  store <2 x i64> %call1, <2 x i64>* %v4, align 16
  store i32 0, i32* %.omp.lb, align 4
  store i32 2, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.AND"(<2 x i64>* %v3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(<2 x i64>* %v4), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]

; CHECK-NOT: QUAL.OMP.REDUCTION.AND
; CHECK: %[[RED:[^,]+]] = load <2 x i64>, <2 x i64>* %v3.fast_red, align 16
; CHECK-NEXT: %[[ORIG:[^,]+]] = load <2 x i64>, <2 x i64>* %v3, align 16
; CHECK-NEXT: %[[ORIG_BOOL:[^,]+]] = icmp ne <2 x i64> %[[ORIG]], zeroinitializer
; CHECK-NEXT: %[[RED_BOOL:[^,]+]] = icmp ne <2 x i64> %[[RED]], zeroinitializer
; CHECK-NEXT: %[[BOOL_VAL:[^,]+]] = select <2 x i1> %[[ORIG_BOOL]], <2 x i1> %[[RED_BOOL]], <2 x i1> %[[ORIG_BOOL]]
; CHECK-NEXT: %[[EXT:[^,]+]] = zext <2 x i1> %[[BOOL_VAL]] to <2 x i64>
; CHECK-NEXT: store <2 x i64> %[[EXT]], <2 x i64>* %v3, align 16

  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load <2 x i64>, <2 x i64>* %v3, align 16
  %6 = load <2 x i64>, <2 x i64>* %v4, align 16
  %cmp2 = icmp ne <2 x i64> %5, zeroinitializer
  %cmp3 = icmp ne <2 x i64> %6, zeroinitializer
  %7 = and <2 x i1> %cmp2, %cmp3
  %sext = sext <2 x i1> %7 to <2 x i64>
  store <2 x i64> %sext, <2 x i64>* %v3, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %8, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %9 = bitcast <2 x i64>* %v3 to i32*
  store i32* %9, i32** %test, align 8
  %10 = bitcast <2 x i64>* %v4 to i32*
  store i32* %10, i32** %ref, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.loop.exit
  %11 = load i32, i32* %i, align 4
  %cmp5 = icmp slt i32 %11, 4
  br i1 %cmp5, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %12 = load i32*, i32** %test, align 8
  %13 = load i32, i32* %i, align 4
  %idxprom = sext i32 %13 to i64
  %ptridx = getelementptr inbounds i32, i32* %12, i64 %idxprom
  %14 = load i32, i32* %ptridx, align 4
  %15 = load i32*, i32** %ref, align 8
  %16 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %16 to i64
  %ptridx7 = getelementptr inbounds i32, i32* %15, i64 %idxprom6
  %17 = load i32, i32* %ptridx7, align 4
  %cmp8 = icmp ne i32 %14, %17
  br i1 %cmp8, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %18 = load i32, i32* %i, align 4
  %19 = load i32*, i32** %test, align 8
  %20 = load i32, i32* %i, align 4
  %idxprom9 = sext i32 %20 to i64
  %ptridx10 = getelementptr inbounds i32, i32* %19, i64 %idxprom9
  %21 = load i32, i32* %ptridx10, align 4
  %22 = load i32, i32* %i, align 4
  %23 = load i32*, i32** %ref, align 8
  %24 = load i32, i32* %i, align 4
  %idxprom11 = sext i32 %24 to i64
  %ptridx12 = getelementptr inbounds i32, i32* %23, i64 %idxprom11
  %25 = load i32, i32* %ptridx12, align 4
  %call13 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i64 0, i64 0), i32 %18, i32 %21, i32 %22, i32 %25)
  %call14 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  store i32 3, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %26 = load i32, i32* %i, align 4
  %inc = add nsw i32 %26, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call15 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i64 0, i64 0))
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %for.end, %if.then
  %27 = load i32, i32* %retval, align 4
  ret i32 %27
}

; Function Attrs: alwaysinline uwtable
define internal <2 x i64> @_ZL14_mm_setr_epi32iiii(i32 %__i0, i32 %__i1, i32 %__i2, i32 %__i3) #1 {
entry:
  %__i0.addr = alloca i32, align 4
  %__i1.addr = alloca i32, align 4
  %__i2.addr = alloca i32, align 4
  %__i3.addr = alloca i32, align 4
  store i32 %__i0, i32* %__i0.addr, align 4
  store i32 %__i1, i32* %__i1.addr, align 4
  store i32 %__i2, i32* %__i2.addr, align 4
  store i32 %__i3, i32* %__i3.addr, align 4
  %0 = load i32, i32* %__i3.addr, align 4
  %1 = load i32, i32* %__i2.addr, align 4
  %2 = load i32, i32* %__i1.addr, align 4
  %3 = load i32, i32* %__i0.addr, align 4
  %call = call <2 x i64> @_ZL13_mm_set_epi32iiii(i32 %0, i32 %1, i32 %2, i32 %3)
  ret <2 x i64> %call
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(i8*, ...) #3

; Function Attrs: alwaysinline nounwind uwtable
define internal <2 x i64> @_ZL13_mm_set_epi32iiii(i32 %__i3, i32 %__i2, i32 %__i1, i32 %__i0) #4 {
entry:
  %__i3.addr = alloca i32, align 4
  %__i2.addr = alloca i32, align 4
  %__i1.addr = alloca i32, align 4
  %__i0.addr = alloca i32, align 4
  %.compoundliteral = alloca <4 x i32>, align 16
  store i32 %__i3, i32* %__i3.addr, align 4
  store i32 %__i2, i32* %__i2.addr, align 4
  store i32 %__i1, i32* %__i1.addr, align 4
  store i32 %__i0, i32* %__i0.addr, align 4
  %0 = load i32, i32* %__i0.addr, align 4
  %vecinit = insertelement <4 x i32> undef, i32 %0, i32 0
  %1 = load i32, i32* %__i1.addr, align 4
  %vecinit1 = insertelement <4 x i32> %vecinit, i32 %1, i32 1
  %2 = load i32, i32* %__i2.addr, align 4
  %vecinit2 = insertelement <4 x i32> %vecinit1, i32 %2, i32 2
  %3 = load i32, i32* %__i3.addr, align 4
  %vecinit3 = insertelement <4 x i32> %vecinit2, i32 %3, i32 3
  store <4 x i32> %vecinit3, <4 x i32>* %.compoundliteral, align 16
  %4 = load <4 x i32>, <4 x i32>* %.compoundliteral, align 16
  %5 = bitcast <4 x i32> %4 to <2 x i64>
  ret <2 x i64> %5
}

attributes #0 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { alwaysinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
