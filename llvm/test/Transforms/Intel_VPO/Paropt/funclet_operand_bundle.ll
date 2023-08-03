;RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
;RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;UNSUPPORTED : linux
;
;SRC input:
;
;#include <omp.h>
;float running_calc = 0.0;
;int main () {
; #pragma omp parallel num_threads(1)
;  try {
;       throw 1;
;  } catch (int t) {
;    #pragma omp for simd
;      for (int i = 0; i < 32; i++) {
;        int intermediate = i * i;
;        #pragma omp ordered threads simd ;
;       running_calc = intermediate + running_calc;
;    };
;  };
;}
;

;check that when running on windows every generated kmpc call that lies within a catch block has the funclet operand bundle.
;CHECK: define dso_local i32 @main
;CHECK:  call void @__kmpc_end(ptr @{{.*}})

;CHECK: define internal void @main.DIR.OMP.PARALLEL{{[^ ]*}}
;CHECK:  call void @__kmpc_for_static_init_{{.*}}(ptr @{{.*}}, i32 %{{.*}}, i32 34, ptr %{{.*}}, ptr %{{.*}}, ptr %{{.*}}, ptr %{{.*}}, i32 1, i32 1) [ "funclet"(token %{{.*}}) ]
;CHECK:  call void @__kmpc_ordered(ptr @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}}) ]
;CHECK:  call void @__kmpc_end_ordered(ptr @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}} ]
;CHECK:  call void @__kmpc_for_static_fini(ptr @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}} ]
;CHECK:  call void @__kmpc_barrier(ptr @{{.*}}, i32 %{{.*}}) [ "funclet"(token %{{.*}}) ]

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

%rtti.TypeDescriptor2 = type { ptr, ptr, [3 x i8] }
%eh.CatchableType = type { i32, i32, i32, i32, i32, i32, i32 }
%eh.CatchableTypeArray.1 = type { i32, [1 x i32] }
%eh.ThrowInfo = type { i32, i32, i32, i32 }

$"??_R0H@8" = comdat any

$"_CT??_R0H@84" = comdat any

$_CTA1H = comdat any

$_TI1H = comdat any

@"?running_calc@@3MA" = dso_local global float 0.000000e+00, align 4
@"??_7type_info@@6B@" = external constant ptr
@"??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { ptr @"??_7type_info@@6B@", ptr null, [3 x i8] c".H\00" }, comdat
@__ImageBase = external dso_local constant i8
@"_CT??_R0H@84" = linkonce_odr unnamed_addr constant %eh.CatchableType { i32 1, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"??_R0H@8" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32), i32 0, i32 -1, i32 0, i32 4, i32 0 }, section ".xdata", comdat
@_CTA1H = linkonce_odr unnamed_addr constant %eh.CatchableTypeArray.1 { i32 1, [1 x i32] [i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @"_CT??_R0H@84" to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32)] }, section ".xdata", comdat
@_TI1H = linkonce_odr unnamed_addr constant %eh.ThrowInfo { i32 0, i32 0, i32 0, i32 trunc (i64 sub nuw nsw (i64 ptrtoint (ptr @_CTA1H to i64), i64 ptrtoint (ptr @__ImageBase to i64)) to i32) }, section ".xdata", comdat

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() personality ptr @__CxxFrameHandler3 {
entry:
  %retval = alloca i32, align 4
  %tmp = alloca i32, align 4
  %t = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %intermediate = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?running_calc@@3MA", float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %t, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %intermediate, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  store i32 1, ptr %tmp, align 4
  invoke void @_CxxThrowException(ptr %tmp, ptr @_TI1H)
          to label %unreachable unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %entry
  %1 = catchswitch within none [label %catch] unwind label %terminate

catch:                                            ; preds = %catch.dispatch
  %2 = catchpad within %1 [ptr @"??_R0H@8", i32 0, ptr %t]
  store i32 0, ptr %.omp.lb, align 4
  store i32 31, ptr %.omp.ub, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %intermediate, i32 0, i32 1) ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV"(ptr %i, i32 1) ]

  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %catch
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %9 = load i32, ptr %i, align 4
  %10 = load i32, ptr %i, align 4
  %mul2 = mul nsw i32 %9, %10
  store i32 %mul2, ptr %intermediate, align 4
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(),
    "QUAL.OMP.ORDERED.THREADS"(),
    "QUAL.OMP.ORDERED.SIMD"() ]

  %12 = load i32, ptr %intermediate, align 4
  %conv = sitofp i32 %12 to float
  %13 = load float, ptr @"?running_calc@@3MA", align 4
  %add3 = fadd float %conv, %13
  store float %add3, ptr @"?running_calc@@3MA", align 4

  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.ORDERED"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %14, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  catchret from %2 to label %catchret.dest

catchret.dest:                                    ; preds = %omp.loop.exit
  br label %try.cont

try.cont:                                         ; preds = %catchret.dest
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %15 = load i32, ptr %retval, align 4
  ret i32 %15

unreachable:                                      ; preds = %entry
  unreachable

terminate:                                        ; preds = %catch.dispatch
  %16 = cleanuppad within none []
  call void @__std_terminate() [ "funclet"(token %16) ]
  unreachable
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @_CxxThrowException(ptr, ptr)
declare dso_local i32 @__CxxFrameHandler3(...)
declare dso_local void @__std_terminate()
