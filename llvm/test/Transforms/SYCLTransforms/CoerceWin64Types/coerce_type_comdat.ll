; RUN: opt -passes=sycl-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s


; This test checks function comdat change

%"class.ihc::hls_float" = type { %class.ac_int }
%class.ac_int = type { %"class.ac_private::iv_conv" }
%"class.ac_private::iv_conv" = type { %"class.ac_private::iv" }
%"class.ac_private::iv" = type { i32 }
%class.ac_int.13 = type { %"class.ac_private::iv_conv.14" }
%"class.ac_private::iv_conv.14" = type { %"class.std::ios_base::Init" }
%"class.std::ios_base::Init" = type { i8 }

$_ZN3ihc9hls_floatILi8ELi23ELNS_9fp_config8FP_RoundE0EE12set_exponentE6ac_intILi8ELb0EE = comdat any
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0
; Function Attrs: nounwind readnone speculatable willreturn
declare float @llvm.log.f32(float) #0
; Function Attrs: nounwind readnone speculatable willreturn
declare float @llvm.sqrt.f32(float) #0
; Function Attrs: nounwind readnone speculatable willreturn
declare float @llvm.pow.f32(float, float) #0
; Function Attrs: nounwind readnone speculatable willreturn
declare float @llvm.exp.f32(float) #0
; Function Attrs: nounwind

; CHECK: define linkonce_odr void @_ZN3ihc9hls_floatILi8ELi23ELNS_9fp_config8FP_RoundE0EE12set_exponentE6ac_intILi8ELb0EE(ptr %this, i8 %bits) comdat align 2 {

define linkonce_odr void @_ZN3ihc9hls_floatILi8ELi23ELNS_9fp_config8FP_RoundE0EE12set_exponentE6ac_intILi8ELb0EE(ptr %this, ptr byval(%class.ac_int.13) align 1 %bits) #2 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8, !tbaa !9
  call void @llvm.dbg.declare(metadata ptr %this.addr, metadata !DIExpression(), metadata !DIExpression())
  call void @llvm.dbg.declare(metadata ptr %bits, metadata !DIExpression(), metadata !DIExpression())
  %this1 = load ptr, ptr %this.addr, align 8
  ret void
}

attributes #0 = { nounwind readnone speculatable willreturn }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "not-ocl-sycl"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4, !4}
!opencl.used.optional.core.features = !{!5, !5}
!opencl.compiler.options = !{!6, !4}
!llvm.ident = !{!7, !8}
!sycl.kernels = !{!4}

!0 = !{i32 2, !"Dwarf Version", i32 4}
!1 = !{i32 2, !"Debug Info Version", i32 3}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{!"-cl-std=CL1.2", !"-g", !"-I/nfs/tor/disks/a10_sdk/a10gx/compile", !"-cl-opt-disable"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C++ TBAA"}

; DEBUGIFY-NOT: WARNING
