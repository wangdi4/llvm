; RUN: opt -passes=sycl-kernel-vec-clone -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-vec-clone,vplan-vec -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-vect-info=%p/../Inputs/VectInfo64.gen %s -S -o - | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

define spir_kernel void @a(ptr addrspace(1) nocapture readonly %a, ptr addrspace(1) nocapture %b) local_unnamed_addr #0 !recommended_vector_length !5 !kernel_arg_base_type !6 !arg_type_null_val !7 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #0
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %a, i64 %call
  %0 = load i32, ptr addrspace(1) %arrayidx, align 4, !tbaa !0
  %1 = trunc i32 %0 to i16
; CHECK: [[WIDE_LOAD_TRUNCi16:%.*]] = trunc <4 x i32> %wide.load to <4 x i16>
  %2 = trunc i32 %0 to i8
; CHECK: [[WIDE_LOAD_TRUNCi8:%.*]] = trunc <4 x i32> %wide.load to <4 x i8>
  %3 = zext i32 %0 to i64
; CHECK: [[WIDE_LOAD_TRUNCi64:%.*]] = zext <4 x i32> %wide.load to <4 x i64>
  %4 = uitofp i32 %0 to float
; CHECK: [[WIDE_LOAD_TRUNCf32:%.*]] = uitofp <4 x i32> %wide.load to <4 x float>
  %5 = uitofp i32 %0 to double
; CHECK: [[WIDE_LOAD_TRUNCf64:%.*]] = uitofp <4 x i32> %wide.load to <4 x double>

  %call0 = tail call spir_func float @_Z4acosf(float %4) #0
  %call1 = tail call spir_func double @_Z4acosd(double %5) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z4acosDv4_f(<4 x float> [[WIDE_LOAD_TRUNCf32]])
; CHECK: call {{(spir_func )?}}<4 x double> @_Z4acosDv4_d(<4 x double> [[WIDE_LOAD_TRUNCf64]])

  %call2 = tail call spir_func float @_Z4fmaxff(float %4, float %4) #0
  %call3 = tail call spir_func double @_Z4fmaxdd(double %5, double %5) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z4fmaxDv4_fS_(<4 x float> [[WIDE_LOAD_TRUNCf32]], <4 x float> [[WIDE_LOAD_TRUNCf32]])
; CHECK: call {{(spir_func )?}}<4 x double> @_Z4fmaxDv4_dS_(<4 x double> [[WIDE_LOAD_TRUNCf64]], <4 x double> [[WIDE_LOAD_TRUNCf64]])

  %call4 = tail call spir_func float @_Z8half_cosf(float %4) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z8half_cosDv4_f(<4 x float> [[WIDE_LOAD_TRUNCf32]])

  %call6 = tail call spir_func float @_Z10native_cosf(float %4) #0
  %call7 = tail call spir_func double @_Z10native_cosd(double %5) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z10native_cosDv4_f(<4 x float> [[WIDE_LOAD_TRUNCf32]])
; CHECK: call {{(spir_func )?}}<4 x double> @_Z10native_cosDv4_d(<4 x double> [[WIDE_LOAD_TRUNCf64]])

  %call8 = tail call spir_func float @_Z11native_fmaxff(float %4, float %4) #0
  %call9 = tail call spir_func double @_Z11native_fmaxdd(double %5, double %5) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z11native_fmaxDv4_fS_(<4 x float> [[WIDE_LOAD_TRUNCf32]], <4 x float> [[WIDE_LOAD_TRUNCf32]])
; CHECK: call {{(spir_func )?}}<4 x double> @_Z11native_fmaxDv4_dS_(<4 x double> [[WIDE_LOAD_TRUNCf64]], <4 x double> [[WIDE_LOAD_TRUNCf64]])

  %call10 = tail call spir_func i64 @_Z3absl(i64 %3) #0
  %call11 = tail call spir_func i32 @_Z3absi(i32 %0) #0
  %call12 = tail call spir_func i16 @_Z3abss(i16 %1) #0
  %call13 = tail call spir_func i8 @_Z3absc(i8 %2) #0
; CHECK: call {{(spir_func )?}}<4 x i64> @_Z3absDv4_l(<4 x i64> [[WIDE_LOAD_TRUNCi64]])
; CHECK: call {{(spir_func )?}}<4 x i32> @_Z3absDv4_i(<4 x i32> %wide.load)
; CHECK: call {{(spir_func )?}}<4 x i16> @_Z3absDv4_s(<4 x i16> [[WIDE_LOAD_TRUNCi16]])
; CHECK: call {{(spir_func )?}}<4 x i8> @_Z3absDv4_c(<4 x i8> [[WIDE_LOAD_TRUNCi8]])

  %call14 = tail call spir_func i8 @_Z8upsamplech(i8 %2, i8 %2) #0
; CHECK: call {{(spir_func )?}}<4 x i8> @_Z8upsampleDv4_cDv4_h(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[WIDE_LOAD_TRUNCi8]])

  %call15 = tail call spir_func i8 @_Z5clampccc(i8 %2, i8 %2, i8 %2) #0
; CHECK: call {{(spir_func )?}}<4 x i8> @_Z5clampDv4_cS_S_(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[WIDE_LOAD_TRUNCi8]])

  %call16 = tail call spir_func float @_Z7degreesf(float %4) #0
  %call17 = tail call spir_func double @_Z7degreesd(double %5) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z7degreesDv4_f(<4 x float> [[WIDE_LOAD_TRUNCf32]])
; CHECK: call {{(spir_func )?}}<4 x double> @_Z7degreesDv4_d(<4 x double> [[WIDE_LOAD_TRUNCf64]])

  %call18 = tail call spir_func float @_Z3mixfff(float %4, float %4, float %4) #0
  %call19 = tail call spir_func double @_Z3mixddd(double %5, double %5, double %5) #0
; CHECK: call {{(spir_func )?}}<4 x float> @_Z3mixDv4_fS_S_(<4 x float> [[WIDE_LOAD_TRUNCf32]], <4 x float> [[WIDE_LOAD_TRUNCf32]], <4 x float> [[WIDE_LOAD_TRUNCf32]])
; CHECK: call {{(spir_func )?}}<4 x double> @_Z3mixDv4_dS_S_(<4 x double> [[WIDE_LOAD_TRUNCf64]], <4 x double> [[WIDE_LOAD_TRUNCf64]], <4 x double> [[WIDE_LOAD_TRUNCf64]])

  %call22 = tail call spir_func i8 @_Z9bitselectccc(i8 %2, i8 %2, i8 %2) #0
; CHECK: call {{(spir_func )?}}<4 x i8> @_Z9bitselectDv4_cS_S_(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[WIDE_LOAD_TRUNCi8]])

  %call23 = tail call spir_func i8 @_Z6selectccc(i8 %2, i8 %2, i8 %2) #0
; CHECK: [[SIGNED_ZEROINIT:%.*]] = icmp ne <4 x i8> [[WIDE_LOAD_TRUNCi8]], zeroinitializer
; CHECK-NEXT: [[SIGNEDi8:%.*]] = sext <4 x i1> [[SIGNED_ZEROINIT]] to <4 x i8>
; CHECK-NEXT: call {{(spir_func )?}}<4 x i8> @_Z6selectDv4_cS_S_(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> [[SIGNEDi8]])

  %call24 = tail call spir_func i32 @_Z4idivii(i32 %0, i32 %0) #0
  %call25 = tail call spir_func float @_Z4udivjj(float %4, float %4) #0
; CHECK: call {{(spir_func )?}}<4 x i32> @_Z4idivDv4_iS_(<4 x i32> %wide.load, <4 x i32> %wide.load)
; CHECK: call {{(spir_func )?}}<4 x float> @_Z4udivDv4_jS_(<4 x float> [[WIDE_LOAD_TRUNCf32]], <4 x float> [[WIDE_LOAD_TRUNCf32]])

  ret void
}

declare spir_func float @_Z4acosf(float) #0
declare spir_func double @_Z4acosd(double) #0
declare spir_func float @_Z4fmaxff(float, float) #0
declare spir_func double @_Z4fmaxdd(double, double) #0
declare spir_func float @_Z8half_cosf(float) #0
declare spir_func float @_Z10native_cosf(float) #0
declare spir_func double @_Z10native_cosd(double) #0
declare spir_func float @_Z11native_fmaxff(float, float) #0
declare spir_func double @_Z11native_fmaxdd(double, double) #0
declare spir_func i64 @_Z3absl(i64) #0
declare spir_func i32 @_Z3absi(i32) #0
declare spir_func i16 @_Z3abss(i16) #0
declare spir_func i8 @_Z3absc(i8) #0
declare spir_func i8 @_Z8upsamplech(i8, i8) #0
declare spir_func i8 @_Z5clampccc(i8, i8, i8) #0
declare spir_func float @_Z7degreesf(float) #0
declare spir_func double @_Z7degreesd(double) #0
declare spir_func float @_Z3mixfff(float, float, float) #0
declare spir_func double @_Z3mixddd(double, double, double) #0
declare spir_func i8 @_Z9bitselectccc(i8, i8, i8) #0
declare spir_func i8 @_Z6selectccc(i8, i8, i8) #0
declare spir_func i32 @_Z4idivii(i32, i32) #0
declare spir_func float @_Z4udivjj(float, float) #0

declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #0

attributes #0 = { convergent nounwind readnone }

!sycl.kernels = !{!4}

!0 = !{!1, !1, i64 0}
!1 = !{!"int", !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{ptr @a}
!5 = !{i32 4}
!6 = !{!"int*", !"int*"}
!7 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVeN4uu_a {{.*}} br
; DEBUGIFY-NOT: WARNING
