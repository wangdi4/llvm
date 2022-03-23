; RUN: opt -argpromotion -S %s | FileCheck %s
; RUN: opt -passes='argpromotion' -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; struct str {
;   float x;
;   float y;
; };
;
; static float foo(struct str arg) {
;   return arg.x + arg.y;
; }
; float bar(struct str arg) {
;   return foo(arg);
; }
; #pragma omp end declare target

; Verify that argpromotion pass does not try to replace
; addrspace(4) byval argument with addrspace(0) alloca and
; applies regular promotion instead:
; CHECK: define internal spir_func float @foo(float{{[^,]*}}, float{{[^,]*}})

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.str = type { float, float }

; Function Attrs: nounwind
define hidden spir_func float @bar(%struct.str addrspace(4)* byval(%struct.str) align 4 %arg) #0 {
entry:
  %retval = alloca float, align 4
  %retval.ascast = addrspacecast float* %retval to float addrspace(4)*
  %call = call fast spir_func float @foo(%struct.str addrspace(4)* byval(%struct.str) align 4 %arg)
  ret float %call
}

; Function Attrs: nounwind
define internal spir_func float @foo(%struct.str addrspace(4)* byval(%struct.str) align 4 %arg) #0 {
entry:
  %retval = alloca float, align 4
  %retval.ascast = addrspacecast float* %retval to float addrspace(4)*
  %x = getelementptr inbounds %struct.str, %struct.str addrspace(4)* %arg, i32 0, i32 0, !intel-tbaa !4
  %0 = load float, float addrspace(4)* %x, align 4, !tbaa !4
  %y = getelementptr inbounds %struct.str, %struct.str addrspace(4)* %arg, i32 0, i32 1, !intel-tbaa !9
  %1 = load float, float addrspace(4)* %y, align 4, !tbaa !9
  %add = fadd fast float %0, %1
  ret float %add
}

attributes #0 = { nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{}
!3 = !{!"clang version 10.0.0"}
!4 = !{!5, !6, i64 0}
!5 = !{!"struct@str", !6, i64 0, !6, i64 4}
!6 = !{!"float", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!5, !6, i64 4}
