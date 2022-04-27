; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s

; Make sure Paropt can handle const-expr map-size/if operands which
; use global vars that are operands on a clause on the region.

; Test src:
;
; #include <stdio.h>
; long N = 100;
; int a[10], b[10];
;
; void foo() {
; #pragma omp target map(a[0:(long)&N]) map(b[0:(long)&N]) if((long)&N)
;   {
;     printf("a[1] = %d, %ld\n", a[1], ((long)&N * 4));
;   }
;   return;
; }
;

; CHECK: renameNonPointerConstExprVInEntryDirective: Expr 'i1 icmp ne (i64 ptrtoint (i64* @N to i64), i64 0)' hoisted to Instruction 'i1 %[[IF:cexpr.inst]]'.
; CHECK: renameNonPointerConstExprVInEntryDirective: Expr 'i64 mul nuw (i64 ptrtoint (i64* @N to i64), i64 4)' hoisted to Instruction 'i64 %[[SIZE:cexpr.inst[^ ']+]]'.
; CHECK: createRenamedValueForV : Renamed 'i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 0)' (via launder intrinsic) to: 'i32* %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed '[10 x i32]* @a' (via launder intrinsic) to: '[10 x i32]* %a'.
; CHECK: createRenamedValueForV : Renamed 'i32* getelementptr inbounds ([10 x i32], [10 x i32]* @b, i64 0, i64 0)' (via launder intrinsic) to: 'i32* %{{.*}}'.
; CHECK: createRenamedValueForV : Renamed '[10 x i32]* @b' (via launder intrinsic) to: '[10 x i32]* %b'.
; CHECK: createRenamedValueForV : Renamed 'i64* @N' (via launder intrinsic) to: 'i64* %N'.

; CHECK: clearLaunderIntrinBeforeRegion: Number of launder intrinsics for the region is 5.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.
; CHECK-DAG: clearLaunderIntrinBeforeRegion: Clearing 1 unhandled intrinsics.
; CHECK: clearLaunderIntrinBeforeRegion: Replacing launder intrinsic 'i8* %{{.+}}' with its operand.

; CHECK-NOT: call i8* @llvm.launder.invariant.group

; Check that the hoisted IF Instruction is used in the target codegen
; CHECK: %[[CHECK:[^ ]+]] = icmp ne i1 %[[IF]], false
; CHECK: br i1 %[[CHECK]], label %{{.+}}, label %{{.+}}

; Check that the same SIZE Instruction is used in maps for a and b, which had
; the same constant-expr size.
; CHECK: %[[SIZE_GEP1:[^ ]+]] = getelementptr inbounds [3 x i64], [3 x i64]* %.offload_sizes, i32 0, i32 0
; CHECK: store i64 %[[SIZE]], i64* %[[SIZE_GEP1]], align 8
; CHECK: %[[SIZE_GEP2:[^ ]+]] = getelementptr inbounds [3 x i64], [3 x i64]* %.offload_sizes, i32 0, i32 1
; CHECK: store i64 %[[SIZE]], i64* %[[SIZE_GEP2]], align 8

; Check that globals @a, @b, @N are not used in the outlined function.
; CHECK: define internal void @__omp_offloading_{{.*}}foov{{.*}}([10 x i32]* %a, [10 x i32]* %b, i64 %N.val)
; CHECK: %N.fpriv = alloca i64, align 1
; CHECK: store i64 %N.val, i64* %N.fpriv, align 8
; CHECK: %[[A1:[^ ]+]] = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 1
; CHECK: %[[A1_LOAD:[^ ]+]] = load i32, i32* %[[A1]], align 4
; CHECK: %[[N1:[^ ]+]] = ptrtoint i64* %N.fpriv to i64
; CHECK: %[[N2:[^ ]+]] = mul nsw i64 %[[N1]], 4
; CHECK: %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i32 %[[A1_LOAD]], i64 %[[N2]]) #5

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@N = external global i64, align 8
@a = external global [10 x i32], align 16
@b = external global [10 x i32], align 16
@.str = private unnamed_addr constant [16 x i8] c"a[1] = %d, %ld\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone uwtable mustprogress
define hidden void @_Z3foov() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.IF"(i1 icmp ne (i64 ptrtoint (i64* @N to i64), i64 0)), "QUAL.OMP.MAP.TOFROM"([10 x i32]* @a, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 0), i64 mul nuw (i64 ptrtoint (i64* @N to i64), i64 4), i64 35, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"([10 x i32]* @b, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @b, i64 0, i64 0), i64 mul nuw (i64 ptrtoint (i64* @N to i64), i64 4), i64 3, i8* null, i8* null), "QUAL.OMP.FIRSTPRIVATE"(i64* @N) ]

  %1 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 1), align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @.str, i64 0, i64 0), i32 %1, i64 mul nsw (i64 ptrtoint (i64* @N to i64), i64 4)) #3

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare i32 @printf(i8*, ...) #2

attributes #0 = { convergent noinline nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 66309, i32 64032527, !"_Z3foov", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
