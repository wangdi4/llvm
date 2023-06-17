; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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

; CHECK-NOT: QUAL.OMP.REDUCTION.AND
; CHECK: %[[RED:[^,]+]] = load <2 x i64>, ptr %v3.fast_red, align 16
; CHECK-NEXT: %[[ORIG:[^,]+]] = load <2 x i64>, ptr %v3, align 16
; CHECK-NEXT: %[[ORIG_BOOL:[^,]+]] = icmp ne <2 x i64> %[[ORIG]], zeroinitializer
; CHECK-NEXT: %[[RED_BOOL:[^,]+]] = icmp ne <2 x i64> %[[RED]], zeroinitializer
; CHECK-NEXT: %[[BOOL_VAL:[^,]+]] = select <2 x i1> %[[ORIG_BOOL]], <2 x i1> %[[RED_BOOL]], <2 x i1> %[[ORIG_BOOL]]
; CHECK-NEXT: %[[EXT:[^,]+]] = zext <2 x i1> %[[BOOL_VAL]] to <2 x i64>
; CHECK-NEXT: store <2 x i64> %[[EXT]], ptr %v3, align 16

; ModuleID = 'reduction_bool_and_m128i.cpp'
source_filename = "reduction_bool_and_m128i.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"test[%d]=%d, ref[%d]=%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca <2 x i64>, align 16
  %i = alloca i32, align 4
  %v4 = alloca <2 x i64>, align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %test = alloca ptr, align 8
  %ref = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  %call = call noundef <2 x i64> @_ZL14_mm_setr_epi32iiii(i32 noundef 2147483647, i32 noundef 2147483647, i32 noundef 2147483647, i32 noundef 2147483647)
  store <2 x i64> %call, ptr %v3, align 16
  %call1 = call noundef <2 x i64> @_ZL14_mm_setr_epi32iiii(i32 noundef 0, i32 noundef 0, i32 noundef 1, i32 noundef 0)
  store <2 x i64> %call1, ptr %v4, align 16
  store i32 0, ptr %.omp.lb, align 4
  store i32 2, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.AND:TYPED"(ptr %v3, <2 x i64> zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %v4, <2 x i64> zeroinitializer, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load <2 x i64>, ptr %v3, align 16
  %6 = load <2 x i64>, ptr %v4, align 16
  %cmp2 = icmp ne <2 x i64> %5, zeroinitializer
  %cmp3 = icmp ne <2 x i64> %6, zeroinitializer
  %7 = and <2 x i1> %cmp2, %cmp3
  %sext = sext <2 x i1> %7 to <2 x i64>
  store <2 x i64> %sext, ptr %v3, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %8, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  store ptr %v3, ptr %test, align 8
  store ptr %v4, ptr %ref, align 8
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.loop.exit
  %9 = load i32, ptr %i, align 4
  %cmp5 = icmp slt i32 %9, 4
  br i1 %cmp5, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %10 = load ptr, ptr %test, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds i32, ptr %10, i64 %idxprom
  %12 = load i32, ptr %arrayidx, align 4
  %13 = load ptr, ptr %ref, align 8
  %14 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %14 to i64
  %arrayidx7 = getelementptr inbounds i32, ptr %13, i64 %idxprom6
  %15 = load i32, ptr %arrayidx7, align 4
  %cmp8 = icmp ne i32 %12, %15
  br i1 %cmp8, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %16 = load i32, ptr %i, align 4
  %17 = load ptr, ptr %test, align 8
  %18 = load i32, ptr %i, align 4
  %idxprom9 = sext i32 %18 to i64
  %arrayidx10 = getelementptr inbounds i32, ptr %17, i64 %idxprom9
  %19 = load i32, ptr %arrayidx10, align 4
  %20 = load i32, ptr %i, align 4
  %21 = load ptr, ptr %ref, align 8
  %22 = load i32, ptr %i, align 4
  %idxprom11 = sext i32 %22 to i64
  %arrayidx12 = getelementptr inbounds i32, ptr %21, i64 %idxprom11
  %23 = load i32, ptr %arrayidx12, align 4
  %call13 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %16, i32 noundef %19, i32 noundef %20, i32 noundef %23)
  %call14 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  store i32 3, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %24 = load i32, ptr %i, align 4
  %inc = add nsw i32 %24, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call15 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  store i32 0, ptr %retval, align 4
  br label %return

return:                                           ; preds = %for.end, %if.then
  %25 = load i32, ptr %retval, align 4
  ret i32 %25
}

define internal noundef <2 x i64> @_ZL14_mm_setr_epi32iiii(i32 noundef %__i0, i32 noundef %__i1, i32 noundef %__i2, i32 noundef %__i3) {
entry:
  %__i0.addr = alloca i32, align 4
  %__i1.addr = alloca i32, align 4
  %__i2.addr = alloca i32, align 4
  %__i3.addr = alloca i32, align 4
  store i32 %__i0, ptr %__i0.addr, align 4
  store i32 %__i1, ptr %__i1.addr, align 4
  store i32 %__i2, ptr %__i2.addr, align 4
  store i32 %__i3, ptr %__i3.addr, align 4
  %0 = load i32, ptr %__i3.addr, align 4
  %1 = load i32, ptr %__i2.addr, align 4
  %2 = load i32, ptr %__i1.addr, align 4
  %3 = load i32, ptr %__i0.addr, align 4
  %call = call noundef <2 x i64> @_ZL13_mm_set_epi32iiii(i32 noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3)
  ret <2 x i64> %call
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...) 

define internal noundef <2 x i64> @_ZL13_mm_set_epi32iiii(i32 noundef %__i3, i32 noundef %__i2, i32 noundef %__i1, i32 noundef %__i0) {
entry:
  %__i3.addr = alloca i32, align 4
  %__i2.addr = alloca i32, align 4
  %__i1.addr = alloca i32, align 4
  %__i0.addr = alloca i32, align 4
  %.compoundliteral = alloca <4 x i32>, align 16
  store i32 %__i3, ptr %__i3.addr, align 4
  store i32 %__i2, ptr %__i2.addr, align 4
  store i32 %__i1, ptr %__i1.addr, align 4
  store i32 %__i0, ptr %__i0.addr, align 4
  %0 = load i32, ptr %__i0.addr, align 4
  %vecinit = insertelement <4 x i32> undef, i32 %0, i32 0
  %1 = load i32, ptr %__i1.addr, align 4
  %vecinit1 = insertelement <4 x i32> %vecinit, i32 %1, i32 1
  %2 = load i32, ptr %__i2.addr, align 4
  %vecinit2 = insertelement <4 x i32> %vecinit1, i32 %2, i32 2
  %3 = load i32, ptr %__i3.addr, align 4
  %vecinit3 = insertelement <4 x i32> %vecinit2, i32 %3, i32 3
  store <4 x i32> %vecinit3, ptr %.compoundliteral, align 16
  %4 = load <4 x i32>, ptr %.compoundliteral, align 16
  %5 = bitcast <4 x i32> %4 to <2 x i64>
  ret <2 x i64> %5
}

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
