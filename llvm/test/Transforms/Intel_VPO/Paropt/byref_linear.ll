; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <assert.h>
;
; void byref_linear(int &yref, short *&zptr_ref, float (*&y_arrptr_ref)[10]) {
;   int *yref_addr = &yref;
;   short **zptr_ref_addr = &zptr_ref;
;   float(**y_arrptr_ref_addr)[10] = &y_arrptr_ref;
;
; #pragma omp for linear(yref, zptr_ref, y_arrptr_ref)
;   for (int i = 0; i < 5; i++) {
;     assert(yref_addr != &yref);
;     assert(zptr_ref_addr != &zptr_ref);
;     assert(y_arrptr_ref_addr != &y_arrptr_ref);
;   }
; }
;
; int main() {
;   int y = 10;
;   short z[10];
;   short *zptr = &z[0];
;   float yarr[10][10];
;   float (*y_arrptr)[10] = &yarr[1];
;
;   byref_linear(y, zptr, y_arrptr);
;
;   assert(y == 14);
;   assert(zptr == &z[4]);
;   assert(y_arrptr == &yarr[5]);
;   return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"yref_addr != &yref\00", align 1
@.str.1 = private unnamed_addr constant [17 x i8] c"byref_linear.cpp\00", align 1
@__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f = private unnamed_addr constant [51 x i8] c"void byref_linear(int &, short *&, float (*&)[10])\00", align 1
@.str.2 = private unnamed_addr constant [27 x i8] c"zptr_ref_addr != &zptr_ref\00", align 1
@.str.3 = private unnamed_addr constant [35 x i8] c"y_arrptr_ref_addr != &y_arrptr_ref\00", align 1
@.str.4 = private unnamed_addr constant [8 x i8] c"y == 14\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [11 x i8] c"int main()\00", align 1
@.str.5 = private unnamed_addr constant [14 x i8] c"zptr == &z[4]\00", align 1
@.str.6 = private unnamed_addr constant [21 x i8] c"y_arrptr == &yarr[5]\00", align 1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z12byref_linearRiRPsRPA10_f(ptr noundef nonnull align 4 dereferenceable(4) %yref, ptr noundef nonnull align 8 dereferenceable(8) %zptr_ref, ptr noundef nonnull align 8 dereferenceable(8) %y_arrptr_ref) #0 {
entry:
  %yref.addr = alloca ptr, align 8
  %zptr_ref.addr = alloca ptr, align 8
  %y_arrptr_ref.addr = alloca ptr, align 8
  %yref_addr = alloca ptr, align 8
  %zptr_ref_addr = alloca ptr, align 8
  %y_arrptr_ref_addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %yref, ptr %yref.addr, align 8
  store ptr %zptr_ref, ptr %zptr_ref.addr, align 8
  store ptr %y_arrptr_ref, ptr %y_arrptr_ref.addr, align 8
  %0 = load ptr, ptr %yref.addr, align 8
  store ptr %0, ptr %yref_addr, align 8
  %1 = load ptr, ptr %zptr_ref.addr, align 8
  store ptr %1, ptr %zptr_ref_addr, align 8
  %2 = load ptr, ptr %y_arrptr_ref.addr, align 8
  store ptr %2, ptr %y_arrptr_ref_addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 4, ptr %.omp.ub, align 4
  %3 = load ptr, ptr %yref.addr, align 8
  %4 = load ptr, ptr %zptr_ref.addr, align 8
  %5 = load ptr, ptr %y_arrptr_ref.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LINEAR:BYREF.TYPED"(ptr %yref.addr, i32 0, i32 1, i32 1),
    "QUAL.OMP.LINEAR:BYREF.PTR_TO_PTR.TYPED"(ptr %zptr_ref.addr, i16 0, i32 1, i32 1),
    "QUAL.OMP.LINEAR:BYREF.PTR_TO_PTR.TYPED"(ptr %y_arrptr_ref.addr, [10 x float] zeroinitializer, i32 1, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %7 = load i32, ptr %.omp.lb, align 4
  store i32 %7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Look for allocation of local copies for byref firstprivates.
; CHECK-DAG: [[YREF_LOCAL:%yref.addr.linear]] = alloca i32
; CHECK-DAG: store ptr [[YREF_LOCAL]], ptr [[YREF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[ZPTR_REF_LOCAL:%zptr_ref.addr.linear]] = alloca ptr
; CHECK-DAG: store ptr [[ZPTR_REF_LOCAL]], ptr [[ZPTR_REF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[YARRPTR_REF_LOCAL:%y_arrptr_ref.addr.linear]] = alloca ptr
; CHECK-DAG: store ptr [[YARRPTR_REF_LOCAL]], ptr [[YARRPTR_REF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]

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
  %cmp1 = icmp ne ptr %11, %12
  br i1 %cmp1, label %cond.true, label %cond.false
; Look for use of local 'yref' instead of the original inside the region.
; CHECK-DAG: [[L1:%[a-zA-Z._0-9]+]] = load ptr, ptr %yref_addr
; CHECK-DAG: [[L2:%[a-zA-Z._0-9]+]] = load ptr, ptr [[YREF_LOCAL_ADDR]]
; CHECK-DAG: icmp ne ptr [[L1]], [[L2]]

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(ptr noundef @.str, ptr noundef @.str.1, i32 noundef 10, ptr noundef @__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f) #4
  unreachable

13:                                               ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %13, %cond.true
  %14 = load ptr, ptr %zptr_ref_addr, align 8
  %15 = load ptr, ptr %zptr_ref.addr, align 8
  %cmp2 = icmp ne ptr %14, %15
  br i1 %cmp2, label %cond.true3, label %cond.false4
; Look for use of local 'zptr_ref' instead of the original inside the region.
; CHECK-DAG: [[L3:%[a-zA-Z._0-9]+]] = load ptr, ptr %zptr_ref_addr
; CHECK-DAG: [[L4:%[a-zA-Z._0-9]+]] = load ptr, ptr [[ZPTR_REF_LOCAL_ADDR]]
; CHECK-DAG: icmp ne ptr [[L3]], [[L4]]

cond.true3:                                       ; preds = %cond.end
  br label %cond.end5

cond.false4:                                      ; preds = %cond.end
  call void @__assert_fail(ptr noundef @.str.2, ptr noundef @.str.1, i32 noundef 11, ptr noundef @__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f) #4
  unreachable

16:                                               ; No predecessors!
  br label %cond.end5

cond.end5:                                        ; preds = %16, %cond.true3
  %17 = load ptr, ptr %y_arrptr_ref_addr, align 8
  %18 = load ptr, ptr %y_arrptr_ref.addr, align 8
  %cmp6 = icmp ne ptr %17, %18
  br i1 %cmp6, label %cond.true7, label %cond.false8

cond.true7:                                       ; preds = %cond.end5
  br label %cond.end9

cond.false8:                                      ; preds = %cond.end5
  call void @__assert_fail(ptr noundef @.str.3, ptr noundef @.str.1, i32 noundef 12, ptr noundef @__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f) #4
  unreachable

19:                                               ; No predecessors!
  br label %cond.end9

cond.end9:                                        ; preds = %19, %cond.true7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end9
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, ptr %.omp.iv, align 4
  %add10 = add nsw i32 %20, 1
  store i32 %add10, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]
  ret void
; Look for linear copyout instructions.
; CHECK-DAG: [[L7:%[a-zA-Z._0-9]+]] = load i32, ptr [[YREF_LOCAL]]
; CHECK-DAG: store i32 [[L7]], ptr [[L8:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[L8]] = load ptr, ptr %yref.addr
;
; CHECK-DAG: [[L9:%[a-zA-Z._0-9]+]] = load ptr, ptr [[ZPTR_REF_LOCAL]]
; CHECK-DAG: store ptr [[L9]], ptr [[L10:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[L10]] = load ptr, ptr %zptr_ref.addr
;
; CHECK-DAG: [[L11:%[a-zA-Z._0-9]+]] = load ptr, ptr [[YARRPTR_REF_LOCAL]],
; CHECK-DAG: store ptr [[L11]], ptr [[L12:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[L12]] = load ptr, ptr %y_arrptr_ref.addr
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
  %z = alloca [10 x i16], align 16
  %zptr = alloca ptr, align 8
  %yarr = alloca [10 x [10 x float]], align 16
  %y_arrptr = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  store i32 10, ptr %y, align 4
  %arrayidx = getelementptr inbounds [10 x i16], ptr %z, i64 0, i64 0
  store ptr %arrayidx, ptr %zptr, align 8
  %arrayidx1 = getelementptr inbounds [10 x [10 x float]], ptr %yarr, i64 0, i64 1
  store ptr %arrayidx1, ptr %y_arrptr, align 8
  call void @_Z12byref_linearRiRPsRPA10_f(ptr noundef nonnull align 4 dereferenceable(4) %y, ptr noundef nonnull align 8 dereferenceable(8) %zptr, ptr noundef nonnull align 8 dereferenceable(8) %y_arrptr)
  %0 = load i32, ptr %y, align 4
  %cmp = icmp eq i32 %0, 14
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  call void @__assert_fail(ptr noundef @.str.4, ptr noundef @.str.1, i32 noundef 25, ptr noundef @__PRETTY_FUNCTION__.main) #4
  unreachable

1:                                                ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %1, %cond.true
  %2 = load ptr, ptr %zptr, align 8
  %arrayidx2 = getelementptr inbounds [10 x i16], ptr %z, i64 0, i64 4
  %cmp3 = icmp eq ptr %2, %arrayidx2
  br i1 %cmp3, label %cond.true4, label %cond.false5

cond.true4:                                       ; preds = %cond.end
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end
  call void @__assert_fail(ptr noundef @.str.5, ptr noundef @.str.1, i32 noundef 26, ptr noundef @__PRETTY_FUNCTION__.main) #4
  unreachable

3:                                                ; No predecessors!
  br label %cond.end6

cond.end6:                                        ; preds = %3, %cond.true4
  %4 = load ptr, ptr %y_arrptr, align 8
  %arrayidx7 = getelementptr inbounds [10 x [10 x float]], ptr %yarr, i64 0, i64 5
  %cmp8 = icmp eq ptr %4, %arrayidx7
  br i1 %cmp8, label %cond.true9, label %cond.false10

cond.true9:                                       ; preds = %cond.end6
  br label %cond.end11

cond.false10:                                     ; preds = %cond.end6
  call void @__assert_fail(ptr noundef @.str.6, ptr noundef @.str.1, i32 noundef 27, ptr noundef @__PRETTY_FUNCTION__.main) #4
  unreachable

5:                                                ; No predecessors!
  br label %cond.end11

cond.end11:                                       ; preds = %5, %cond.true9
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
