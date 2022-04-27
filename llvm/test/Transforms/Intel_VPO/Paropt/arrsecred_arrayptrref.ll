; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
; Test src:

; void b(int (*(&a))[10]) {
; #pragma omp parallel reduction(+: a[0][4])
;   *a;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z1bRPA10_i([10 x i32]** nonnull align 8 dereferenceable(8) %a) #0 {
entry:
  %a.addr = alloca [10 x i32]**, align 8
  store [10 x i32]** %a, [10 x i32]*** %a.addr, align 8
  %0 = load [10 x i32]**, [10 x i32]*** %a.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT"([10 x i32]*** %a.addr, i64 2, i64 0, i64 1, i64 1, i64 4, i64 1, i64 1) ]
  %2 = load [10 x i32]**, [10 x i32]*** %a.addr, align 8
  %3 = load [10 x i32]*, [10 x i32]** %2, align 8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}

; CHECK-LABEL: @_Z1bRPA10_i.DIR.OMP.PARALLEL.{{.*}}(
; CHECK:         [[A_ADDR_RED:%.*]] = alloca i32, i64 1, align 8
; CHECK-NEXT:    [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR:%.*]] = alloca i32*, align 8
; CHECK-NEXT:    [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR_CAST_REF:%.*]] = alloca [10 x i32]**, align 8
; CHECK-NEXT:    [[A_ADDR_FAST_RED_GEP_MINUS_OFFSET_ADDR:%.*]] = alloca i32*, align 8
; CHECK-NEXT:    [[A_ADDR_FAST_RED_GEP_MINUS_OFFSET_ADDR_CAST_REF:%.*]] = alloca [10 x i32]**, align 8

; CHECK:         [[A_ADDR_RED_GEP_MINUS_OFFSET:%.*]] = getelementptr i32, i32* [[A_ADDR_RED]], i64 -4
; CHECK-NEXT:    store i32* [[A_ADDR_RED_GEP_MINUS_OFFSET]], i32** [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR]], align 8
; CHECK-NEXT:    [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR_CAST:%.*]] = bitcast i32** [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR]] to [10 x i32]**
; CHECK-NEXT:    store [10 x i32]** [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR_CAST]], [10 x i32]*** [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR_CAST_REF]], align 8

; CHECK:         [[TMP2:%.*]] = load [10 x i32]**, [10 x i32]*** [[A_ADDR_RED_GEP_MINUS_OFFSET_ADDR_CAST_REF]], align 8
; CHECK-NEXT:    [[TMP3:%.*]] = load [10 x i32]*, [10 x i32]** [[TMP2]], align 8

