; RUN: %oclopt -coerce-types -S %s -o - | FileCheck %s
; This test checks function comdat change

; ModuleID = './comdat.ll'
source_filename = "./kernel.cl"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

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

; CHECK: declare void @___ZN3ihc9hls_floatILi8ELi23ELNS_9fp_config8FP_RoundE0EE12set_exponentE6ac_intILi8ELb0EE_before.CoerceTypes(%"class.ihc::hls_float"*, %class.ac_int.13* byval(%class.ac_int.13) align 1) align 2
; CHECK: define linkonce_odr void @_ZN3ihc9hls_floatILi8ELi23ELNS_9fp_config8FP_RoundE0EE12set_exponentE6ac_intILi8ELb0EE(%"class.ihc::hls_float"* %this, i8 %bits.coerce.high) comdat {

define linkonce_odr void @_ZN3ihc9hls_floatILi8ELi23ELNS_9fp_config8FP_RoundE0EE12set_exponentE6ac_intILi8ELb0EE(%"class.ihc::hls_float"* %this, %class.ac_int.13* byval(%class.ac_int.13) align 1 %bits) #2 comdat align 2 {
entry:
  %this.addr = alloca %"class.ihc::hls_float"*, align 8
  store %"class.ihc::hls_float"* %this, %"class.ihc::hls_float"** %this.addr, align 8, !tbaa !9
  call void @llvm.dbg.declare(metadata %"class.ihc::hls_float"** %this.addr, metadata !DIExpression(), metadata !DIExpression())
  call void @llvm.dbg.declare(metadata %class.ac_int.13* %bits, metadata !DIExpression(), metadata !DIExpression())
  %this1 = load %"class.ihc::hls_float"*, %"class.ihc::hls_float"** %this.addr, align 8
  %fp_bit_value = getelementptr inbounds %"class.ihc::hls_float", %"class.ihc::hls_float"* %this1, i32 0, i32 0
  ret void
}

attributes #0 = { nounwind readnone speculatable willreturn }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "not-ocl-dpcpp"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!3}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4, !4}
!opencl.used.optional.core.features = !{!5, !5}
!opencl.compiler.options = !{!6, !4}
!llvm.ident = !{!7, !8}
!opencl.kernels = !{!4}

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
