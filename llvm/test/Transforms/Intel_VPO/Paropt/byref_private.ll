; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <assert.h>
;
; void byref_private(int &yref, float (&y_arr_ref)[10][10]) {
;   int *yref_addr = &yref;
;   float(*y_arr_ref_addr)[10][10] = &y_arr_ref;
;   yref = 2;
;   y_arr_ref[1][1] = 3;
;
; #pragma omp for private(yref, y_arr_ref)
;   for (int i = 0; i < 10; i++) {
;     assert(yref_addr != &yref);
;     assert(y_arr_ref_addr != &y_arr_ref);
;     assert(yref != 2);
;     assert(y_arr_ref[1][1] != 3);
;   }
; }
;
; int main() {
;   int y = 10;
;   float yarr[10][10];
;   byref_private(y, yarr);
;   return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"yref_addr != &yref\00", align 1
@.str.1 = private unnamed_addr constant [18 x i8] c"byref_private.cpp\00", align 1
@__PRETTY_FUNCTION__._Z13byref_privateRiRA10_A10_f = private unnamed_addr constant [45 x i8] c"void byref_private(int &, float (&)[10][10])\00", align 1
@.str.2 = private unnamed_addr constant [29 x i8] c"y_arr_ref_addr != &y_arr_ref\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"yref != 2\00", align 1
@.str.4 = private unnamed_addr constant [21 x i8] c"y_arr_ref[1][1] != 3\00", align 1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z13byref_privateRiRA10_A10_f(ptr noundef nonnull align 4 dereferenceable(4) %yref, ptr noundef nonnull align 4 dereferenceable(400) %y_arr_ref) #0 {
entry:
  %yref.addr = alloca ptr, align 8
  %y_arr_ref.addr = alloca ptr, align 8
  %yref_addr = alloca ptr, align 8
  %y_arr_ref_addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %tmp2 = alloca ptr, align 8
  %tmp3 = alloca ptr, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %yref, ptr %yref.addr, align 8
  store ptr %y_arr_ref, ptr %y_arr_ref.addr, align 8
  %0 = load ptr, ptr %yref.addr, align 8
  store ptr %0, ptr %yref_addr, align 8
  %1 = load ptr, ptr %y_arr_ref.addr, align 8
  store ptr %1, ptr %y_arr_ref_addr, align 8
  %2 = load ptr, ptr %yref.addr, align 8
  store i32 2, ptr %2, align 4
  %3 = load ptr, ptr %y_arr_ref.addr, align 8
  %arrayidx = getelementptr inbounds [10 x [10 x float]], ptr %3, i64 0, i64 1
  %arrayidx1 = getelementptr inbounds [10 x float], ptr %arrayidx, i64 0, i64 1
  store float 3.000000e+00, ptr %arrayidx1, align 4
  store ptr undef, ptr %tmp2, align 8
  store ptr undef, ptr %tmp3, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %4 = load ptr, ptr %yref.addr, align 8
  %5 = load ptr, ptr %y_arr_ref.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:BYREF.TYPED"(ptr %yref.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:BYREF.TYPED"(ptr %y_arr_ref.addr, [10 x [10 x float]] zeroinitializer, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Look for allocation of local copies for byref privates.
; CHECK: [[YREF_ARR_LOCAL:%y_arr_ref.addr.priv]] = alloca [10 x [10 x float]]
; CHECK: store ptr [[YREF_ARR_LOCAL]], ptr [[YREF_ARR_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK: [[YREF_LOCAL:%yref.addr.priv]] = alloca i32
; CHECK: store ptr [[YREF_LOCAL]], ptr [[YREF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]

  %7 = load i32, ptr %.omp.lb, align 4
  store i32 %7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i32, ptr %.omp.iv, align 4
  %9 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %11 = load ptr, ptr %yref_addr, align 8
  %12 = load ptr, ptr %yref.addr, align 8
  %cmp4 = icmp ne ptr %11, %12
; Look for use of local 'yref' instead of the original inside the region.
; CHECK: [[L1:%[a-zA-Z._0-9]+]] = load ptr, ptr %yref_addr
; CHECK: [[L2:%[a-zA-Z._0-9]+]] = load ptr, ptr [[YREF_LOCAL_ADDR]]
; CHECK: icmp ne ptr [[L1]], [[L2]]
  br i1 %cmp4, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(ptr noundef @.str, ptr noundef @.str.1, i32 noundef 11, ptr noundef @__PRETTY_FUNCTION__._Z13byref_privateRiRA10_A10_f) #4
  unreachable

13:                                               ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %13, %cond.true
  %14 = load ptr, ptr %y_arr_ref_addr, align 8
  %15 = load ptr, ptr %y_arr_ref.addr, align 8
  %cmp5 = icmp ne ptr %14, %15
; Look for use of local 'y_arr_ref' instead of the original inside the region.
; CHECK: [[L3:%[a-zA-Z._0-9]+]] = load ptr, ptr %y_arr_ref_addr
; CHECK: [[L4:%[a-zA-Z._0-9]+]] = load ptr, ptr [[YREF_ARR_LOCAL_ADDR]]
; CHECK: icmp ne ptr [[L3]], [[L4]]

  br i1 %cmp5, label %cond.true6, label %cond.false7

cond.true6:                                       ; preds = %cond.end
  br label %cond.end8

cond.false7:                                      ; preds = %cond.end
  call void @__assert_fail(ptr noundef @.str.2, ptr noundef @.str.1, i32 noundef 12, ptr noundef @__PRETTY_FUNCTION__._Z13byref_privateRiRA10_A10_f) #4
  unreachable

16:                                               ; No predecessors!
  br label %cond.end8

cond.end8:                                        ; preds = %16, %cond.true6
  %17 = load ptr, ptr %yref.addr, align 8
  %18 = load i32, ptr %17, align 4
  %cmp9 = icmp ne i32 %18, 2
  br i1 %cmp9, label %cond.true10, label %cond.false11

cond.true10:                                      ; preds = %cond.end8
  br label %cond.end12

cond.false11:                                     ; preds = %cond.end8
  call void @__assert_fail(ptr noundef @.str.3, ptr noundef @.str.1, i32 noundef 13, ptr noundef @__PRETTY_FUNCTION__._Z13byref_privateRiRA10_A10_f) #4
  unreachable

19:                                               ; No predecessors!
  br label %cond.end12

cond.end12:                                       ; preds = %19, %cond.true10
  %20 = load ptr, ptr %y_arr_ref.addr, align 8
  %arrayidx13 = getelementptr inbounds [10 x [10 x float]], ptr %20, i64 0, i64 1
  %arrayidx14 = getelementptr inbounds [10 x float], ptr %arrayidx13, i64 0, i64 1
  %21 = load float, ptr %arrayidx14, align 4
  %cmp15 = fcmp fast une float %21, 3.000000e+00
  br i1 %cmp15, label %cond.true16, label %cond.false17

cond.true16:                                      ; preds = %cond.end12
  br label %cond.end18

cond.false17:                                     ; preds = %cond.end12
  call void @__assert_fail(ptr noundef @.str.4, ptr noundef @.str.1, i32 noundef 14, ptr noundef @__PRETTY_FUNCTION__._Z13byref_privateRiRA10_A10_f) #4
  unreachable

22:                                               ; No predecessors!
  br label %cond.end18

cond.end18:                                       ; preds = %22, %cond.true16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end18
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, ptr %.omp.iv, align 4
  %add19 = add nsw i32 %23, 1
  store i32 %add19, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noreturn nounwind
declare dso_local void @__assert_fail(ptr noundef, ptr noundef, i32 noundef, ptr noundef) #2

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #3 {
entry:
  %retval = alloca i32, align 4
  %y = alloca i32, align 4
  %yarr = alloca [10 x [10 x float]], align 16
  store i32 0, ptr %retval, align 4
  store i32 10, ptr %y, align 4
  call void @_Z13byref_privateRiRA10_A10_f(ptr noundef nonnull align 4 dereferenceable(4) %y, ptr noundef nonnull align 4 dereferenceable(400) %yarr)
  ret i32 0
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { noreturn nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
