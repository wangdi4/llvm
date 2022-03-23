; RUN: opt -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s -check-prefix=TFORM

; Test src:

; class A {
; public:
;   A();
;   ~A();
;   int a;
; };
; void foo() {
;   A *b;
; #pragma omp target
;   b->a = 10;
; }

; The tese IR is obtained using -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -instcombine -simplifycfg.
; The test is to ensure that we can handle cases where there is no store to the ADDR
; operand of an "OPERAND.ADDR" pair (such as "%.addr").

; This can happen when an optimization replaces the first operand with undef, and thus deletes the store.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%class.A = type { i32 }

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %b.map.ptr.tmp = alloca %class.A*, align 8
  %b.map.ptr.tmp.addr = alloca %class.A**, align 8
  %.addr = alloca %class.A*, align 8
  store %class.A** %b.map.ptr.tmp, %class.A*** %b.map.ptr.tmp.addr, align 8
  %end.dir.temp = alloca i1, align 1

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(%class.A* undef, %class.A* undef, i64 0, i64 544), "QUAL.OMP.PRIVATE"(%class.A** %b.map.ptr.tmp), "QUAL.OMP.OPERAND.ADDR"(%class.A** %b.map.ptr.tmp, %class.A*** %b.map.ptr.tmp.addr), "QUAL.OMP.OPERAND.ADDR"(%class.A* undef, %class.A** %.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]

  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TARGET.4.split, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %entry
  %b.map.ptr.tmp1 = load volatile %class.A**, %class.A*** %b.map.ptr.tmp.addr, align 8

; Check that after restore operands, uses of %1 are replaced with undef.
; RESTR: store %class.A* undef, %class.A** %b.map.ptr.tmp{{.*}}
; RESTR: %a = getelementptr inbounds %class.A, %class.A* undef, i64 0, i32 0

; Check that parop-transform doesn't comp-fail when compiling the output of restore-operands.
; TFORM: call i32 @__tgt_target(i64 %{{.*}}, i8* @__omp_offloading{{[^ ,]*}}foo{{[^ ,]*}}, i32 0, i8** null, i8** null, i64* null, i64* null)

  %1 = load volatile %class.A*, %class.A** %.addr, align 8
  store %class.A* %1, %class.A** %b.map.ptr.tmp1, align 8
  %a = getelementptr inbounds %class.A, %class.A* %1, i64 0, i32 0
  store i32 10, i32* %a, align 4
  br label %DIR.OMP.END.TARGET.4.split

DIR.OMP.END.TARGET.4.split:                       ; preds = %entry, %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: noinline uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2055, i32 150480225, !"_Z3foov", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
