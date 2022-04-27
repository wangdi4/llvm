; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s

; Test src:

; typedef struct {
;   int *a;
; } b;
; #pragma omp declare target
; b c;
; #pragma omp end declare target
; int d() {
; #pragma omp target map(c.a[1])
;   c.a[0];
; }

; CHECK: renameNonPointerConstExprVInEntryDirective: Expr 'i64 sdiv exact (i64 sub (i64 ptrtoint (i32** getelementptr (i32*, i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0), i32 1) to i64), i64 ptrtoint (%struct.b* @c to i64)), i64 ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64))' hoisted to Instruction 'i64 %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed 'i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0)' (via launder intrinsic) to: 'i32** %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed '%struct.b* @c' (via launder intrinsic) to: '%struct.b* %c'.

; CHECK: clearLaunderIntrinBeforeRegion: Number of launder intrinsics for the region is 2.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.

; CHECK-NOT: call i8* @llvm.launder.invariant.group

; Check that globals @c is not used in the outlined function.
; CHECK: define internal void @__omp_offloading{{.*}}(%struct.b* %c)
; CHECK: %{{[^ ]+}} = getelementptr inbounds %struct.b, %struct.b* %c, i32 0, i32 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.b = type { i32* }

@c = hidden target_declare global %struct.b zeroinitializer, align 8

; Function Attrs: convergent noinline nounwind optnone uwtable mustprogress
define hidden i32 @_Z1dv() #0 {
entry:
  %retval = alloca i32, align 4
  %0 = load i32*, i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0), align 8
  %ptridx = getelementptr inbounds i32, i32* %0, i64 1

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.MAP.TOFROM"(%struct.b* @c, i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0), i64 sdiv exact (i64 sub (i64 ptrtoint (i32** getelementptr (i32*, i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0), i32 1) to i64), i64 ptrtoint (%struct.b* @c to i64)), i64 ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)), i64 32, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM:CHAIN"(i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0), i32* %ptridx, i64 4, i64 281474976710675, i8* null, i8* null) ]

  %2 = load i32*, i32** getelementptr inbounds (%struct.b, %struct.b* @c, i32 0, i32 0), align 8
  %ptridx1 = getelementptr inbounds i32, i32* %2, i64 0

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %3 = load i32, i32* %retval, align 4
  ret i32 %3
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3, !4, !5}

!0 = !{i32 0, i32 66309, i32 47066959, !"_Z1dv", i32 8, i32 1, i32 0}
!1 = !{i32 1, !"_Z1c", i32 0, i32 0, %struct.b* @c}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"PIC Level", i32 2}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{i32 7, !"frame-pointer", i32 2}
