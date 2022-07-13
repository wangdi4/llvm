; REQUIRES: asserts
; RUN: opt -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

; C source test for OMP5.0 defaultmap construct. Need to compile with -fopenmp-version=50.
; int AAA[10];
; int foo() {
;   int iii = 123;
;   #pragma omp target defaultmap(to:aggregate) defaultmap(tofrom:scalar)
;   {
;     iii = iii + AAA[7];
;   }
;   return iii;
; }
;
; Defaultmap is processed in the FE, and the info is passed to the BE via IR
; in case BE needs it (eg, for compiler-created vars).
; This does not affect codegen currently, so we check by
; dumping the WRN and verify that it was parsed.
;
; BEGIN TARGET ID=1 {
;
;   DEFAULTMAP: TO : AGGREGATE                <--- CHECK
;   DEFAULTMAP: TOFROM : SCALAR               <--- CHECK
;   OFFLOAD_ENTRY_IDX: 0
;   MAP clause (size=2): CHAIN(<i32* %iii, i32* %iii, i64 4, 547> ) CHAIN(<[10 x i32]* @AAA, [10 x i32]* @AAA, i64 40, 545> )
;
;   EntryBB: DIR.OMP.TARGET.2
;   ExitBB: DIR.OMP.END.TARGET.4
;
; } END TARGET ID=1
;
; CHECK: DEFAULTMAP: TO : AGGREGATE
; CHECK: DEFAULTMAP: TOFROM : SCALAR

source_filename = "defaultmap.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@AAA = dso_local global [10 x i32] zeroinitializer, align 16
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo() #0 {
entry:
  %iii = alloca i32, align 4
  store i32 123, i32* %iii, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.DEFAULTMAP.TO:AGGREGATE"(), "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"(), "QUAL.OMP.MAP.TOFROM"(i32* %iii, i32* %iii, i64 4, i64 547), "QUAL.OMP.MAP.TO"([10 x i32]* @AAA, [10 x i32]* @AAA, i64 40, i64 545) ]
  br label %DIR.OMP.TARGET.33

DIR.OMP.TARGET.33:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.33
  %1 = load i32, i32* %iii, align 4
  %2 = getelementptr inbounds [10 x i32], [10 x i32]* @AAA, i64 0, i64 7
  %3 = load i32, i32* %2, align 4
  %add = add nsw i32 %1, %3
  store i32 %add, i32* %iii, align 4
  br label %DIR.OMP.END.TARGET.4.split

DIR.OMP.END.TARGET.4.split:                       ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.END.TARGET.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  %4 = load i32, i32* %iii, align 4
  ret i32 %4
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 57, i32 -684612205, !"foo", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}

