; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
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
;
; ModuleID = 'byref_linear.cpp'
source_filename = "byref_linear.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
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

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z12byref_linearRiRPsRPA10_f(i32* dereferenceable(4) %yref, i16** dereferenceable(8) %zptr_ref, [10 x float]** dereferenceable(8) %y_arrptr_ref) #0 {
entry:
  %yref.addr = alloca i32*, align 8
  %zptr_ref.addr = alloca i16**, align 8
  %y_arrptr_ref.addr = alloca [10 x float]**, align 8
  %yref_addr = alloca i32*, align 8
  %zptr_ref_addr = alloca i16**, align 8
  %y_arrptr_ref_addr = alloca [10 x float]**, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %yref, i32** %yref.addr, align 8
  store i16** %zptr_ref, i16*** %zptr_ref.addr, align 8
  store [10 x float]** %y_arrptr_ref, [10 x float]*** %y_arrptr_ref.addr, align 8
  %0 = load i32*, i32** %yref.addr, align 8
  store i32* %0, i32** %yref_addr, align 8
  %1 = load i16**, i16*** %zptr_ref.addr, align 8
  store i16** %1, i16*** %zptr_ref_addr, align 8
  %2 = load [10 x float]**, [10 x float]*** %y_arrptr_ref.addr, align 8
  store [10 x float]** %2, [10 x float]*** %y_arrptr_ref_addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 4, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  %3 = load i32*, i32** %yref.addr, align 8
  %4 = load i16**, i16*** %zptr_ref.addr, align 8
  %5 = load [10 x float]**, [10 x float]*** %y_arrptr_ref.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.LINEAR:BYREF"(i32** %yref.addr, i16*** %zptr_ref.addr, [10 x float]*** %y_arrptr_ref.addr, i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %7 = load i32, i32* %.omp.lb, align 4
  store i32 %7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond
; CHECK-NOT: call token @llvm.directive.region.entry()
; CHECK-NOT: call token @llvm.directive.region.exit()

; Look for allocation of local copies for byref firstprivates.
; CHECK-DAG: [[YREF_LOCAL:%yref.addr.linear]] = alloca i32
; CHECK-DAG: store i32* [[YREF_LOCAL]], i32** [[YREF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[ZPTR_REF_LOCAL:%zptr_ref.addr.linear]] = alloca i16*
; CHECK-DAG: store i16** [[ZPTR_REF_LOCAL]], i16*** [[ZPTR_REF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[YARRPTR_REF_LOCAL:%y_arrptr_ref.addr.linear]] = alloca [10 x float]*
; CHECK-DAG: store [10 x float]** [[YARRPTR_REF_LOCAL]], [10 x float]*** [[YARRPTR_REF_LOCAL_ADDR:%[a-zA-Z._0-9]+]]

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i32, i32* %.omp.iv, align 4
  %9 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %11 = load i32*, i32** %yref_addr, align 8
  %12 = load i32*, i32** %yref.addr, align 8
  %cmp1 = icmp ne i32* %11, %12
  br i1 %cmp1, label %cond.true, label %cond.false
; Look for use of local 'yref' instead of the original inside the region.
; CHECK-DAG: [[L1:%[a-zA-Z._0-9]+]] = load i32*, i32** %yref_addr
; CHECK-DAG: [[L2:%[a-zA-Z._0-9]+]] = load i32*, i32** [[YREF_LOCAL_ADDR]]
; CHECK-DAG: icmp ne i32* [[L1]], [[L2]]

cond.true:                                        ; preds = %omp.inner.for.body
  br label %cond.end

cond.false:                                       ; preds = %omp.inner.for.body
  call void @__assert_fail(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 11, i8* getelementptr inbounds ([51 x i8], [51 x i8]* @__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f, i32 0, i32 0)) #4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %13 = load i16**, i16*** %zptr_ref_addr, align 8
  %14 = load i16**, i16*** %zptr_ref.addr, align 8
  %cmp2 = icmp ne i16** %13, %14
  br i1 %cmp2, label %cond.true3, label %cond.false4
; Look for use of local 'zptr_ref' instead of the original inside the region.
; CHECK-DAG: [[L3:%[a-zA-Z._0-9]+]] = load i16**, i16*** %zptr_ref_addr
; CHECK-DAG: [[L4:%[a-zA-Z._0-9]+]] = load i16**, i16*** [[ZPTR_REF_LOCAL_ADDR]]
; CHECK-DAG: icmp ne i16** [[L3]], [[L4]]

cond.true3:                                       ; preds = %cond.end
  br label %cond.end5

cond.false4:                                      ; preds = %cond.end
  call void @__assert_fail(i8* getelementptr inbounds ([27 x i8], [27 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 12, i8* getelementptr inbounds ([51 x i8], [51 x i8]* @__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f, i32 0, i32 0)) #4
  br label %cond.end5

cond.end5:                                        ; preds = %cond.false4, %cond.true3
  %15 = load [10 x float]**, [10 x float]*** %y_arrptr_ref_addr, align 8
  %16 = load [10 x float]**, [10 x float]*** %y_arrptr_ref.addr, align 8
  %cmp6 = icmp ne [10 x float]** %15, %16
  br i1 %cmp6, label %cond.true7, label %cond.false8

cond.true7:                                       ; preds = %cond.end5
  br label %cond.end9

cond.false8:                                      ; preds = %cond.end5
  call void @__assert_fail(i8* getelementptr inbounds ([35 x i8], [35 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 13, i8* getelementptr inbounds ([51 x i8], [51 x i8]* @__PRETTY_FUNCTION__._Z12byref_linearRiRPsRPA10_f, i32 0, i32 0)) #4
  br label %cond.end9

cond.end9:                                        ; preds = %cond.false8, %cond.true7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %cond.end9
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, i32* %.omp.iv, align 4
  %add10 = add nsw i32 %17, 1
  store i32 %add10, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.LOOP"() ]
  ret void
; Look for linear copyout instructions.
; CHECK-DAG: [[L7:%[a-zA-Z._0-9]+]] = load i32, i32* [[YREF_LOCAL]]
; CHECK-DAG: store i32 [[L7]], i32* [[L8:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[L8]] = load i32*, i32** %yref.addr
;
; CHECK-DAG: [[L9:%[a-zA-Z._0-9]+]] = load i16*, i16** [[ZPTR_REF_LOCAL]]
; CHECK-DAG: store i16* [[L9]], i16** [[L10:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[L10]] = load i16**, i16*** %zptr_ref.addr
;
; CHECK-DAG: [[L11:%[a-zA-Z._0-9]+]] = load [10 x float]*, [10 x float]** [[YARRPTR_REF_LOCAL]]
; CHECK-DAG: store [10 x float]* [[L11]], [10 x float]** [[L12:%[a-zA-Z._0-9]+]]
; CHECK-DAG: [[L12]] = load [10 x float]**, [10 x float]*** %y_arrptr_ref.addr
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noreturn nounwind
declare dso_local void @__assert_fail(i8*, i8*, i32, i8*) #2

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #3 {
entry:
  %retval = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca [10 x i16], align 16
  %zptr = alloca i16*, align 8
  %yarr = alloca [10 x [10 x float]], align 16
  %y_arrptr = alloca [10 x float]*, align 8
  store i32 0, i32* %retval, align 4
  store i32 10, i32* %y, align 4
  %arrayidx = getelementptr inbounds [10 x i16], [10 x i16]* %z, i64 0, i64 0
  store i16* %arrayidx, i16** %zptr, align 8
  %arrayidx1 = getelementptr inbounds [10 x [10 x float]], [10 x [10 x float]]* %yarr, i64 0, i64 1
  store [10 x float]* %arrayidx1, [10 x float]** %y_arrptr, align 8
  call void @_Z12byref_linearRiRPsRPA10_f(i32* dereferenceable(4) %y, i16** dereferenceable(8) %zptr, [10 x float]** dereferenceable(8) %y_arrptr)
  %0 = load i32, i32* %y, align 4
  %cmp = icmp eq i32 %0, 14
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  br label %cond.end

cond.false:                                       ; preds = %entry
  call void @__assert_fail(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.4, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 26, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) #4
  unreachable
                                                  ; No predecessors!
  br label %cond.end

cond.end:                                         ; preds = %1, %cond.true
  %2 = load i16*, i16** %zptr, align 8
  %arrayidx2 = getelementptr inbounds [10 x i16], [10 x i16]* %z, i64 0, i64 4
  %cmp3 = icmp eq i16* %2, %arrayidx2
  br i1 %cmp3, label %cond.true4, label %cond.false5

cond.true4:                                       ; preds = %cond.end
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end
  call void @__assert_fail(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str.5, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 27, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) #4
  unreachable
                                                  ; No predecessors!
  br label %cond.end6

cond.end6:                                        ; preds = %3, %cond.true4
  %4 = load [10 x float]*, [10 x float]** %y_arrptr, align 8
  %arrayidx7 = getelementptr inbounds [10 x [10 x float]], [10 x [10 x float]]* %yarr, i64 0, i64 5
  %cmp8 = icmp eq [10 x float]* %4, %arrayidx7
  br i1 %cmp8, label %cond.true9, label %cond.false10

cond.true9:                                       ; preds = %cond.end6
  br label %cond.end11

cond.false10:                                     ; preds = %cond.end6
  call void @__assert_fail(i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str.6, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.1, i32 0, i32 0), i32 28, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) #4
  unreachable
                                                  ; No predecessors!
  br label %cond.end11

cond.end11:                                       ; preds = %5, %cond.true9
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
