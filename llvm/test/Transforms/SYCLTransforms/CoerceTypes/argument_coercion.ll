; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s

; This test checks function argument type coercion

%struct.SingleInt = type { i32 }
%struct.SingleFloat = type { float }
%struct.IntAndSSE = type { i8, float }
%struct.TwoFloats = type { float, float }
%struct.TwoDoubles = type { double, double }
%struct.TwoInts = type { i32, i32 }
%struct.TwoLongs = type { i64, i64 }
%struct.TwoDifferentWords = type { double, i64 }
%struct.TwoWordWithArray = type { float, [2 x i32], float }
%struct.ThreeIntegerMember = type { i8, i32, i32 }
%struct.NestedStruct = type { float, %struct.TwoInts, float }
%struct.OneElementFloatArray = type { [1 x float] }

; Function Attrs: convergent nounwind
define void @test() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 {
entry:
  %SI = alloca %struct.SingleInt, align 4
  %SF = alloca %struct.SingleFloat, align 4
  %IAS = alloca %struct.IntAndSSE, align 4
  %TF = alloca %struct.TwoFloats, align 4
  %TD = alloca %struct.TwoDoubles, align 8
  %TI = alloca %struct.TwoInts, align 4
  %TL = alloca %struct.TwoLongs, align 8
  %TDW = alloca %struct.TwoDifferentWords, align 8
  %TWA = alloca %struct.TwoWordWithArray, align 4
  %TIM = alloca %struct.ThreeIntegerMember, align 4
  %NS = alloca %struct.NestedStruct, align 4
  %OEFA = alloca %struct.OneElementFloatArray, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %SI) #3
  call void @singleInt(ptr byval(%struct.SingleInt) align 4 %SI) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %SF) #3
  call void @singleFloat(ptr byval(%struct.SingleFloat) align 4 %SF) #4
  call void @llvm.lifetime.start.p0(i64 8, ptr %IAS) #3
  call void @intAndSSE(ptr byval(%struct.IntAndSSE) align 4 %IAS) #4
  call void @llvm.lifetime.start.p0(i64 8, ptr %TF) #3
  call void @twoFloats(ptr byval(%struct.TwoFloats) align 4 %TF) #4
  call void @llvm.lifetime.start.p0(i64 16, ptr %TD) #3
  call void @twoDoubles(ptr byval(%struct.TwoDoubles) align 8 %TD) #4
  call void @llvm.lifetime.start.p0(i64 8, ptr %TI) #3
  call void @twoInts(ptr byval(%struct.TwoInts) align 4 %TI) #4
  call void @llvm.lifetime.start.p0(i64 16, ptr %TL) #3
  call void @twoLongs(ptr byval(%struct.TwoLongs) align 8 %TL) #4
  call void @llvm.lifetime.start.p0(i64 16, ptr %TDW) #3
  call void @twoDifferentWords(ptr byval(%struct.TwoDifferentWords) align 8 %TDW) #4
  call void @llvm.lifetime.start.p0(i64 16, ptr %TWA) #3
  call void @twoWordWithArray(ptr byval(%struct.TwoWordWithArray) align 4 %TWA) #4
  call void @llvm.lifetime.start.p0(i64 12, ptr %TIM) #3
  call void @threeIntegerMember(ptr byval(%struct.ThreeIntegerMember) align 4 %TIM) #4
  call void @llvm.lifetime.start.p0(i64 16, ptr %NS) #3
  call void @nestedStruct(ptr byval(%struct.NestedStruct) align 4 %NS) #4
  call void @llvm.lifetime.start.p0(i64 4, ptr %OEFA) #3
  call void @oneElementFLoatArray(ptr byval(%struct.OneElementFloatArray) align 4 %OEFA) #4
  call void @outOfIntRegisters(ptr byval(%struct.TwoLongs) align 8 %TL, ptr byval(%struct.TwoLongs) align 8 %TL, ptr byval(%struct.SingleInt) align 4 %SI, ptr byval(%struct.TwoLongs) align 8 %TL, ptr byval(%struct.SingleInt) align 4 %SI, ptr byval(%struct.SingleInt) align 4 %SI) #4
  call void @outOfSSERegisters(ptr byval(%struct.TwoDoubles) align 8 %TD, ptr byval(%struct.TwoDoubles) align 8 %TD, ptr byval(%struct.TwoDoubles) align 8 %TD, ptr byval(%struct.SingleFloat) align 4 %SF, ptr byval(%struct.TwoDoubles) align 8 %TD, ptr byval(%struct.SingleFloat) align 4 %SF, ptr byval(%struct.SingleFloat) align 4 %SF) #4
  call void @llvm.lifetime.end.p0(i64 4, ptr %OEFA) #3
  call void @llvm.lifetime.end.p0(i64 16, ptr %NS) #3
  call void @llvm.lifetime.end.p0(i64 16, ptr %TWA) #3
  call void @llvm.lifetime.end.p0(i64 16, ptr %TDW) #3
  call void @llvm.lifetime.end.p0(i64 16, ptr %TL) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %TI) #3
  call void @llvm.lifetime.end.p0(i64 16, ptr %TD) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %TF) #3
  call void @llvm.lifetime.end.p0(i64 8, ptr %IAS) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %SF) #3
  call void @llvm.lifetime.end.p0(i64 4, ptr %SI) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: convergent
declare void @singleInt(ptr byval(%struct.SingleInt) align 4) #2
; CHECK: declare void @singleInt(i32)

; Function Attrs: convergent
declare void @singleFloat(ptr byval(%struct.SingleFloat) align 4) #2
; CHECK: declare void @singleFloat(float)

; Function Attrs: convergent
declare void @intAndSSE(ptr byval(%struct.IntAndSSE) align 4) #2
; CHECK: declare void @intAndSSE(i64)

; Function Attrs: convergent
declare void @twoFloats(ptr byval(%struct.TwoFloats) align 4) #2
; CHECK: declare void @twoFloats(<2 x float>)

; Function Attrs: convergent
declare void @twoDoubles(ptr byval(%struct.TwoDoubles) align 8) #2
; CHECK: declare void @twoDoubles(double, double)

; Function Attrs: convergent
declare void @twoInts(ptr byval(%struct.TwoInts) align 4) #2
; CHECK: declare void @twoInts(i64)

; Function Attrs: convergent
declare void @twoLongs(ptr byval(%struct.TwoLongs) align 8) #2
; CHECK: declare void @twoLongs(i64, i64)

; Function Attrs: convergent
declare void @twoDifferentWords(ptr byval(%struct.TwoDifferentWords) align 8) #2
; CHECK: declare void @twoDifferentWords(double, i64)

; Function Attrs: convergent
declare void @twoWordWithArray(ptr byval(%struct.TwoWordWithArray) align 4) #2
; CHECK: declare void @twoWordWithArray(i64, i64)

; Function Attrs: convergent
declare void @threeIntegerMember(ptr byval(%struct.ThreeIntegerMember) align 4) #2
; CHECK: declare void @threeIntegerMember(i64, i32)

; Function Attrs: convergent
declare void @nestedStruct(ptr byval(%struct.NestedStruct) align 4) #2
; CHECK: declare void @nestedStruct(i64, i64)

; Function Attrs: convergent
declare void @oneElementFLoatArray(ptr byval(%struct.OneElementFloatArray) align 4) #2
; CHECK: declare void @oneElementFLoatArray(float)

; Function Attrs: convergent
declare void @outOfIntRegisters(ptr byval(%struct.TwoLongs) align 8, ptr byval(%struct.TwoLongs) align 8, ptr byval(%struct.SingleInt) align 4, ptr byval(%struct.TwoLongs) align 8, ptr byval(%struct.SingleInt) align 4, ptr byval(%struct.SingleInt) align 4) #2
; CHECK: declare void @outOfIntRegisters(i64, i64, i64, i64, i32, ptr, i32, ptr)

; Function Attrs: convergent
declare void @outOfSSERegisters(ptr byval(%struct.TwoDoubles) align 8, ptr byval(%struct.TwoDoubles) align 8, ptr byval(%struct.TwoDoubles) align 8, ptr byval(%struct.SingleFloat) align 4, ptr byval(%struct.TwoDoubles) align 8, ptr byval(%struct.SingleFloat) align 4, ptr byval(%struct.SingleFloat) align 4) #2
; CHECK: declare void @outOfSSERegisters(double, double, double, double, double, double, float, ptr, float, ptr)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"icx (ICX) dev.8.x.0"}
!5 = !{ptr @test}


; DEBUGIFY-NOT: WARNING
