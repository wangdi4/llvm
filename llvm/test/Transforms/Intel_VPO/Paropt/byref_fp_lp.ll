; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <assert.h>
;
; void byref_fp_lp(int &yref, float (&y_arr_ref)[10][10]) {
;   int *yref_addr = &yref;
;   float(*y_arr_ref_addr)[10][10] = &y_arr_ref;
;   yref = 2;
;   y_arr_ref[1][1] = 3;
;
; #pragma omp parallel for firstprivate(yref, y_arr_ref) lastprivate(yref, y_arr_ref)
;   for (int i = 0; i < 10; i++) {
;     assert(yref_addr != &yref);
;     assert(y_arr_ref_addr != &y_arr_ref);
;     assert(yref == 2);
;     assert(y_arr_ref[1][1] == 3);
;
;     yref = 5;
;     y_arr_ref[1][1] = 6;
;   }
; }
;
; int main() {
;   int y = 10;
;   float yarr[10][10];
;   byref_fp_lp(y, yarr);
;   assert(y == 5);
;   assert(yarr[1][1] == 6);
;   return 0;
; }

; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Look for allocation of local copies for byref firstprivates.
; CHECK: [[YREF_ARR_LOCAL:%y_arr_ref.addr.lpriv]] = alloca [10 x [10 x float]]
; CHECK: [[YREF_LOCAL:%yref.addr.lpriv]] = alloca i32
; CHECK: store ptr [[YREF_ARR_LOCAL]], ptr [[YREF_ARR_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK: store ptr [[YREF_LOCAL]], ptr [[YREF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]

; Look for initialization of local copies of byref firstprivates.
; CHECK: [[L0:%[a-zA-Z._0-9]+]] = load ptr, ptr %y_arr_ref.addr
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 [[YREF_ARR_LOCAL]], ptr align 4 [[L0]], i64 400, i1 false)

; CHECK: [[L1:%[a-zA-Z._0-9]+]] = load ptr, ptr %yref.addr
; CHECK: [[L2:%[a-zA-Z._0-9]+]] = load i32, ptr [[L1]]
; CHECK: store i32 [[L2]], ptr [[YREF_LOCAL]]

; Look for use of local 'yref' instead of the original inside the region.
; CHECK: [[L3:%[a-zA-Z._0-9]+]] = load ptr, ptr %yref_addr
; CHECK: [[L4:%[a-zA-Z._0-9]+]] = load ptr, ptr [[YREF_LOCAL_ADDR]]
; CHECK: icmp ne ptr [[L3]], [[L4]]

; Look for use of local 'y_arr_ref' instead of the original inside the region.
; CHECK: [[L5:%[a-zA-Z._0-9]+]] = load ptr, ptr %y_arr_ref_addr
; CHECK: [[L6:%[a-zA-Z._0-9]+]] = load ptr, ptr [[YREF_ARR_LOCAL_ADDR]]
; CHECK: icmp ne ptr [[L5]], [[L6]]

; Look for lastprivate copyout instructions.
; CHECK: call{{.*}}@__kmpc_for_static_fini
; CHECK: [[L7:%[a-zA-Z._0-9]+]] = load ptr, ptr %yref.addr
; CHECK: [[L8:%[a-zA-Z._0-9]+]] = load i32, ptr [[YREF_LOCAL]]
; CHECK: store i32 [[L8]], ptr [[L7]]

; CHECK: [[L9:%[a-zA-Z._0-9]+]] = load ptr, ptr %y_arr_ref.addr
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 [[L9]], ptr align 4 [[YREF_ARR_LOCAL]], i64 400, i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"yref_addr != &yref\00", align 1
@.str.1 = private unnamed_addr constant [16 x i8] c"byref_fp_lp.cpp\00", align 1
@__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f = private unnamed_addr constant [43 x i8] c"void byref_fp_lp(int &, float (&)[10][10])\00", align 1
@.str.2 = private unnamed_addr constant [29 x i8] c"y_arr_ref_addr != &y_arr_ref\00", align 1
@.str.3 = private unnamed_addr constant [10 x i8] c"yref == 2\00", align 1
@.str.4 = private unnamed_addr constant [21 x i8] c"y_arr_ref[1][1] == 3\00", align 1
@.str.5 = private unnamed_addr constant [7 x i8] c"y == 5\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [11 x i8] c"int main()\00", align 1
@.str.6 = private unnamed_addr constant [16 x i8] c"yarr[1][1] == 6\00", align 1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z11byref_fp_lpRiRA10_A10_f(ptr noundef nonnull align 4 dereferenceable(4) %yref, ptr noundef nonnull align 4 dereferenceable(400) %y_arr_ref) #0 {
entry:
  %yref.addr = alloca ptr, align 8
  %y_arr_ref.addr = alloca ptr, align 8
  %yref_addr = alloca ptr, align 8
  %y_arr_ref_addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
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
  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %4 = load ptr, ptr %yref.addr, align 8
  %5 = load ptr, ptr %y_arr_ref.addr, align 8
  %6 = load ptr, ptr %yref.addr, align 8
  %7 = load ptr, ptr %y_arr_ref.addr, align 8
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %yref.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:BYREF.TYPED"(ptr %y_arr_ref.addr, [10 x [10 x float]] zeroinitializer, i32 1),
    "QUAL.OMP.LASTPRIVATE:BYREF.TYPED"(ptr %yref.addr, i32 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:BYREF.TYPED"(ptr %y_arr_ref.addr, [10 x [10 x float]] zeroinitializer, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %yref_addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %y_arr_ref_addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %9 = load i32, ptr %.omp.lb, align 4
  store i32 %9, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %10 = load i32, ptr %.omp.iv, align 4
  %11 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %13 = load ptr, ptr %yref_addr, align 8
  %14 = load ptr, ptr %yref.addr, align 8
  %cmp2 = icmp ne ptr %13, %14
  br i1 %cmp2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(ptr noundef @.str, ptr noundef @.str.1, i32 noundef 12, ptr noundef @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f) #4
  unreachable

15:                                               ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %15, %cond.true
  %16 = load ptr, ptr %y_arr_ref_addr, align 8
  %17 = load ptr, ptr %y_arr_ref.addr, align 8
  %cmp3 = icmp ne ptr %16, %17
  br i1 %cmp3, label %cond.true4, label %cond.false5

cond.true4:                                       ; preds = %cond.end
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end
  call void @__assert_fail(ptr noundef @.str.2, ptr noundef @.str.1, i32 noundef 13, ptr noundef @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f) #4
  unreachable

18:                                               ; No predecessors!
  br label %cond.end6

cond.end6:                                        ; preds = %18, %cond.true4
  %19 = load ptr, ptr %yref.addr, align 8
  %20 = load i32, ptr %19, align 4
  %cmp7 = icmp eq i32 %20, 2
  br i1 %cmp7, label %cond.true8, label %cond.false9

cond.true8:                                       ; preds = %cond.end6
  br label %cond.end10

cond.false9:                                      ; preds = %cond.end6
  call void @__assert_fail(ptr noundef @.str.3, ptr noundef @.str.1, i32 noundef 14, ptr noundef @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f) #4
  unreachable

21:                                               ; No predecessors!
  br label %cond.end10

cond.end10:                                       ; preds = %21, %cond.true8
  %22 = load ptr, ptr %y_arr_ref.addr, align 8
  %arrayidx11 = getelementptr inbounds [10 x [10 x float]], ptr %22, i64 0, i64 1
  %arrayidx12 = getelementptr inbounds [10 x float], ptr %arrayidx11, i64 0, i64 1
  %23 = load float, ptr %arrayidx12, align 4
  %cmp13 = fcmp fast oeq float %23, 3.000000e+00
  br i1 %cmp13, label %cond.true14, label %cond.false15

cond.true14:                                      ; preds = %cond.end10
  br label %cond.end16

cond.false15:                                     ; preds = %cond.end10
  call void @__assert_fail(ptr noundef @.str.4, ptr noundef @.str.1, i32 noundef 15, ptr noundef @__PRETTY_FUNCTION__._Z11byref_fp_lpRiRA10_A10_f) #4
  unreachable

24:                                               ; No predecessors!
  br label %cond.end16

cond.end16:                                       ; preds = %24, %cond.true14
  %25 = load ptr, ptr %yref.addr, align 8
  store i32 5, ptr %25, align 4
  %26 = load ptr, ptr %y_arr_ref.addr, align 8
  %arrayidx17 = getelementptr inbounds [10 x [10 x float]], ptr %26, i64 0, i64 1
  %arrayidx18 = getelementptr inbounds [10 x float], ptr %arrayidx17, i64 0, i64 1
  store float 6.000000e+00, ptr %arrayidx18, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end16
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %27 = load i32, ptr %.omp.iv, align 4
  %add19 = add nsw i32 %27, 1
  store i32 %add19, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
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
  call void @_Z11byref_fp_lpRiRA10_A10_f(ptr noundef nonnull align 4 dereferenceable(4) %y, ptr noundef nonnull align 4 dereferenceable(400) %yarr)
  %0 = load i32, ptr %y, align 4
  %cmp = icmp eq i32 %0, 5
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  call void @__assert_fail(ptr noundef @.str.5, ptr noundef @.str.1, i32 noundef 26, ptr noundef @__PRETTY_FUNCTION__.main) #4
  unreachable

1:                                                ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %1, %cond.true
  %arrayidx = getelementptr inbounds [10 x [10 x float]], ptr %yarr, i64 0, i64 1
  %arrayidx1 = getelementptr inbounds [10 x float], ptr %arrayidx, i64 0, i64 1
  %2 = load float, ptr %arrayidx1, align 4
  %cmp2 = fcmp fast oeq float %2, 6.000000e+00
  br i1 %cmp2, label %cond.true3, label %cond.false4

cond.true3:                                       ; preds = %cond.end
  br label %cond.end5

cond.false4:                                      ; preds = %cond.end
  call void @__assert_fail(ptr noundef @.str.6, ptr noundef @.str.1, i32 noundef 27, ptr noundef @__PRETTY_FUNCTION__.main) #4
  unreachable

3:                                                ; No predecessors!
  br label %cond.end5

cond.end5:                                        ; preds = %3, %cond.true3
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
