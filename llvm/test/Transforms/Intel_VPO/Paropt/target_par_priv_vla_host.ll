; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s

; Test src:

; #include <omp.h>
; #include <stdio.h>
; int main()
; {
;   int n_dims = 2;
;   int tmp[n_dims];
;   tmp[0] = 10;
;
; #pragma omp target defaultmap(tofrom:scalar)
;   {
; #pragma omp parallel private(tmp)
;     {
;       printf("%d\n", tmp[0]);
;     }
;   }
;   printf("%d\n", tmp[1]);
; }

; This test contains the host IR corresponding to target_par_priv_vla_tgt.ll.

; Check that we capture the VLA size expression at both parallel and target constructs.
; CHECK: VPOParopt Transform: PARALLEL construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %1 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to) clause for: 'i64* [[SIZE_ADDR1:%[^ ]+]]'
; CHECK: VPOParopt Transform: TARGET construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %1 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to) clause for: 'i64* [[SIZE_ADDR2:%[^ ]+]]'

; Check that the captured VLA size is passed in to the outlined function for the target region.
; CHECK: define dso_local i32 @main()
; CHECK: [[SIZE_ADDR2]] = alloca i64, align 8
; CHECK: store i64 %1, i64* [[SIZE_ADDR2]], align 8
; CHECK: call void @__omp_offloading{{.*}}main{{.*}}(i64* [[SIZE_ADDR2]], i32* %vla, i64* %{{.*}})

; Check that the captured VLA size is used in the parallel region for allocation of the private VLA.
; CHECK: define internal void @main.DIR.OMP.PARALLEL{{.*}}(i32* %{{.*}}, i32* %{{.*}}, i64* [[SIZE_ADDR1]], i64* %{{.*}})
; CHECK: [[SIZE_VAL1:%[^ ]+]] = load i64, i64* [[SIZE_ADDR1]], align 8
; CHECK: %{{.*}} = alloca i32, i64 [[SIZE_VAL1]], align 16

; Check that the captured VLA size is used in the target region for allocation of the private VLA.
; CHECK: define internal void @__omp_offloading{{.*}}main{{.*}}(i64* noalias [[SIZE_ADDR2]], i32* %{{.*}}, i64* %{{.*}})
; CHECK: [[SIZE_ADDR1]] = alloca i64, align 8
; CHECK: [[SIZE_VAL2:%[^ ]+]] = load i64, i64* [[SIZE_ADDR2]], align 8
; CHECK: %{{.*}} = alloca i32, i64 [[SIZE_VAL2]], align 16
; Check that the captured VLA size is passed in to the outlined function for the parallel region.
; CHECK: store i64 [[SIZE_VAL2]], i64* [[SIZE_ADDR1]], align 8
; CHECK: call void {{.+}} @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 2, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64*, i64*)* @main.DIR.OMP.PARALLEL{{.*}} to void (i32*, i32*, ...)*), i64* [[SIZE_ADDR1]], i64* %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %n_dims = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp1 = alloca i64, align 8
  store i32 2, i32* %n_dims, align 4
  %0 = load i32, i32* %n_dims, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8** %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  %ptridx = getelementptr inbounds i32, i32* %vla, i64 0
  store i32 10, i32* %ptridx, align 16
  store i64 %1, i64* %omp.vla.tmp, align 8


  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.DEFAULTMAP.TOFROM:SCALAR"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %vla), "QUAL.OMP.FIRSTPRIVATE"(i64* %omp.vla.tmp), "QUAL.OMP.PRIVATE"(i64* %omp.vla.tmp1) ]


  %4 = load i64, i64* %omp.vla.tmp, align 8
  store i64 %4, i64* %omp.vla.tmp1, align 8
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32* %vla), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp1) ]


  %6 = load i64, i64* %omp.vla.tmp1, align 8
  %ptridx2 = getelementptr inbounds i32, i32* %vla, i64 0
  %7 = load i32, i32* %ptridx2, align 16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %7) #1


  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]


  %ptridx3 = getelementptr inbounds i32, i32* %vla, i64 1
  %8 = load i32, i32* %ptridx3, align 4
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %8)
  %9 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %9)
  ret i32 0
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2055, i32 150358973, !"_Z4main", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 10.0.0"}
