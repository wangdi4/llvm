; REQUIRES: asserts
; RUN: opt -switch-to-offload -debug-only=vpo-paropt-utils -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -switch-to-offload -debug-only=vpo-paropt-utils -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s
;
; INTEL_CUSTOMIZATION
; // To produce the "QUAL.OMP.PRIVATE:TYPED" IR, compile with
; // icx -c -fiopenmp -fopenmp-targets=spir64 -Xclang -fopenmp-typed-clauses
; end INTEL_CUSTOMIZATION
; void foo(int nnn) {
;   double aaa[nnn];
;   #pragma omp target parallel private(aaa)
;     aaa[7] = 789.0;
; }
;
; Check that the privatized VLA array is in addrspace(4)
; CHECK:  getItemInfo: Local Element Info for 'double addrspace(4)* %vla.ascast' (Typed):: Type: double, NumElements: i64 %1, AddrSpace: 4
;
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func void @foo(i32 noundef %nnn) #0 {
entry:
  %nnn.addr = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %nnn.addr.ascast = addrspacecast i32* %nnn.addr to i32 addrspace(4)*
  %saved_stack.ascast = addrspacecast i8** %saved_stack to i8* addrspace(4)*
  %__vla_expr0.ascast = addrspacecast i64* %__vla_expr0 to i64 addrspace(4)*
  %omp.vla.tmp.ascast = addrspacecast i64* %omp.vla.tmp to i64 addrspace(4)*
  store i32 %nnn, i32 addrspace(4)* %nnn.addr.ascast, align 4
  %0 = load i32, i32 addrspace(4)* %nnn.addr.ascast, align 4
  %1 = zext i32 %0 to i64
  %2 = call i8* @llvm.stacksave()
  store i8* %2, i8* addrspace(4)* %saved_stack.ascast, align 8
  %vla = alloca double, i64 %1, align 8
  %vla.ascast = addrspacecast double* %vla to double addrspace(4)*
  store i64 %1, i64 addrspace(4)* %__vla_expr0.ascast, align 8
  store i64 %1, i64 addrspace(4)* %omp.vla.tmp.ascast, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %vla.ascast, double 0.000000e+00, i64 %1), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i64 addrspace(4)* %omp.vla.tmp.ascast, i64 0, i32 1) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:TYPED"(double addrspace(4)* %vla.ascast, double 0.000000e+00, i64 %1), "QUAL.OMP.SHARED"(i64 addrspace(4)* %omp.vla.tmp.ascast) ]
  %5 = load i64, i64 addrspace(4)* %omp.vla.tmp.ascast, align 8
  %arrayidx = getelementptr inbounds double, double addrspace(4)* %vla.ascast, i64 7
  store double 7.890000e+02, double addrspace(4)* %arrayidx, align 8
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  %6 = load i8*, i8* addrspace(4)* %saved_stack.ascast, align 8
  call void @llvm.stackrestore(i8* %6)
  ret void
}

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #1

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 57, i32 -681255226, !"_Z3foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
