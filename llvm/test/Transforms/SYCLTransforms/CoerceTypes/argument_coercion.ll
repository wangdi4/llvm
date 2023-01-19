; RUN: opt -passes=dpcpp-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s --check-prefixes=X64-LINUX,X64-LINUX-NONOPAQUE
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s --check-prefixes=X64-WIN,X64-WIN-NONOPAQUE

; RUN: opt -passes=dpcpp-kernel-coerce-types -opaque-pointers -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-types -opaque-pointers -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s --check-prefixes=X64-LINUX,X64-LINUX-OPAQUE
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -opaque-pointers -mtriple x86_64-w64-mingw32 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-win64-types -opaque-pointers -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s --check-prefixes=X64-WIN,X64-WIN-OPAQUE

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
  %0 = bitcast %struct.SingleInt* %SI to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  call void @singleInt(%struct.SingleInt* byval(%struct.SingleInt) align 4 %SI) #4
  %1 = bitcast %struct.SingleFloat* %SF to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  call void @singleFloat(%struct.SingleFloat* byval(%struct.SingleFloat) align 4 %SF) #4
  %2 = bitcast %struct.IntAndSSE* %IAS to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #3
  call void @intAndSSE(%struct.IntAndSSE* byval(%struct.IntAndSSE) align 4 %IAS) #4
  %3 = bitcast %struct.TwoFloats* %TF to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #3
  call void @twoFloats(%struct.TwoFloats* byval(%struct.TwoFloats) align 4 %TF) #4
  %4 = bitcast %struct.TwoDoubles* %TD to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %4) #3
  call void @twoDoubles(%struct.TwoDoubles* byval(%struct.TwoDoubles) align 8 %TD) #4
  %5 = bitcast %struct.TwoInts* %TI to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %5) #3
  call void @twoInts(%struct.TwoInts* byval(%struct.TwoInts) align 4 %TI) #4
  %6 = bitcast %struct.TwoLongs* %TL to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %6) #3
  call void @twoLongs(%struct.TwoLongs* byval(%struct.TwoLongs) align 8 %TL) #4
  %7 = bitcast %struct.TwoDifferentWords* %TDW to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %7) #3
  call void @twoDifferentWords(%struct.TwoDifferentWords* byval(%struct.TwoDifferentWords) align 8 %TDW) #4
  %8 = bitcast %struct.TwoWordWithArray* %TWA to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %8) #3
  call void @twoWordWithArray(%struct.TwoWordWithArray* byval(%struct.TwoWordWithArray) align 4 %TWA) #4
  %bc.tim = bitcast %struct.ThreeIntegerMember* %TIM to i8*
  call void @llvm.lifetime.start.p0i8(i64 12, i8* %bc.tim) #3
  call void @threeIntegerMember(%struct.ThreeIntegerMember* byval(%struct.ThreeIntegerMember) align 4 %TIM) #4
  %9 = bitcast %struct.NestedStruct* %NS to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %9) #3
  call void @nestedStruct(%struct.NestedStruct* byval(%struct.NestedStruct) align 4 %NS) #4
  %10 = bitcast %struct.OneElementFloatArray* %OEFA to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #3
  call void @oneElementFLoatArray(%struct.OneElementFloatArray* byval(%struct.OneElementFloatArray) align 4 %OEFA) #4
  call void @outOfIntRegisters(%struct.TwoLongs* byval(%struct.TwoLongs) align 8 %TL, %struct.TwoLongs* byval(%struct.TwoLongs) align 8 %TL, %struct.SingleInt* byval(%struct.SingleInt) align 4 %SI, %struct.TwoLongs* byval(%struct.TwoLongs) align 8 %TL, %struct.SingleInt* byval(%struct.SingleInt) align 4 %SI, %struct.SingleInt* byval(%struct.SingleInt) align 4 %SI) #4
  call void @outOfSSERegisters(%struct.TwoDoubles* byval(%struct.TwoDoubles) align 8 %TD, %struct.TwoDoubles* byval(%struct.TwoDoubles) align 8 %TD, %struct.TwoDoubles* byval(%struct.TwoDoubles) align 8 %TD, %struct.SingleFloat* byval(%struct.SingleFloat) align 4 %SF, %struct.TwoDoubles* byval(%struct.TwoDoubles) align 8 %TD, %struct.SingleFloat* byval(%struct.SingleFloat) align 4 %SF, %struct.SingleFloat* byval(%struct.SingleFloat) align 4 %SF) #4
  %11 = bitcast %struct.OneElementFloatArray* %OEFA to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #3
  %12 = bitcast %struct.NestedStruct* %NS to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %12) #3
  %13 = bitcast %struct.TwoWordWithArray* %TWA to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %13) #3
  %14 = bitcast %struct.TwoDifferentWords* %TDW to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %14) #3
  %15 = bitcast %struct.TwoLongs* %TL to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %15) #3
  %16 = bitcast %struct.TwoInts* %TI to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %16) #3
  %17 = bitcast %struct.TwoDoubles* %TD to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %17) #3
  %18 = bitcast %struct.TwoFloats* %TF to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %18) #3
  %19 = bitcast %struct.IntAndSSE* %IAS to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %19) #3
  %20 = bitcast %struct.SingleFloat* %SF to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #3
  %21 = bitcast %struct.SingleInt* %SI to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: convergent
declare void @singleInt(%struct.SingleInt* byval(%struct.SingleInt) align 4) #2
; X64-LINUX: declare void @singleInt(i32)
; X64-WIN:   declare void @singleInt(i32)

; Function Attrs: convergent
declare void @singleFloat(%struct.SingleFloat* byval(%struct.SingleFloat) align 4) #2
; X64-LINUX: declare void @singleFloat(float)
; X64-WIN:   declare void @singleFloat(i32)

; Function Attrs: convergent
declare void @intAndSSE(%struct.IntAndSSE* byval(%struct.IntAndSSE) align 4) #2
; X64-LINUX: declare void @intAndSSE(i64)
; X64-WIN:   declare void @intAndSSE(i64)

; Function Attrs: convergent
declare void @twoFloats(%struct.TwoFloats* byval(%struct.TwoFloats) align 4) #2
; X64-LINUX: declare void @twoFloats(<2 x float>)
; X64-WIN:   declare void @twoFloats(i64)

; Function Attrs: convergent
declare void @twoDoubles(%struct.TwoDoubles* byval(%struct.TwoDoubles) align 8) #2
; X64-LINUX: declare void @twoDoubles(double, double)
; X64-WIN-NONOPAQUE:   declare void @twoDoubles(%struct.TwoDoubles*)
; X64-WIN-OPAQUE:   declare void @twoDoubles(ptr)

; Function Attrs: convergent
declare void @twoInts(%struct.TwoInts* byval(%struct.TwoInts) align 4) #2
; X64-LINUX: declare void @twoInts(i64)
; X64-WIN:   declare void @twoInts(i64)

; Function Attrs: convergent
declare void @twoLongs(%struct.TwoLongs* byval(%struct.TwoLongs) align 8) #2
; X64-LINUX: declare void @twoLongs(i64, i64)
; X64-WIN-NONOPAQUE:   declare void @twoLongs(%struct.TwoLongs*)
; X64-WIN-OPAQUE:   declare void @twoLongs(ptr)

; Function Attrs: convergent
declare void @twoDifferentWords(%struct.TwoDifferentWords* byval(%struct.TwoDifferentWords) align 8) #2
; X64-LINUX: declare void @twoDifferentWords(double, i64)
; X64-WIN-NONOPAQUE:   declare void @twoDifferentWords(%struct.TwoDifferentWords*)
; X64-WIN-OPAQUE:   declare void @twoDifferentWords(ptr)

; Function Attrs: convergent
declare void @twoWordWithArray(%struct.TwoWordWithArray* byval(%struct.TwoWordWithArray) align 4) #2
; X64-LINUX: declare void @twoWordWithArray(i64, i64)
; X64-WIN-NONOPAQUE:   declare void @twoWordWithArray(%struct.TwoWordWithArray*)
; X64-WIN-OPAQUE:   declare void @twoWordWithArray(ptr)

; Function Attrs: convergent
declare void @threeIntegerMember(%struct.ThreeIntegerMember* byval(%struct.ThreeIntegerMember) align 4) #2
; X64-LINUX: declare void @threeIntegerMember(i64, i32)
; X64-WIN-NONOPAQUE:   declare void @threeIntegerMember(%struct.ThreeIntegerMember*)
; X64-WIN-OPAQUE:   declare void @threeIntegerMember(ptr)

; Function Attrs: convergent
declare void @nestedStruct(%struct.NestedStruct* byval(%struct.NestedStruct) align 4) #2
; X64-LINUX: declare void @nestedStruct(i64, i64)
; X64-WIN-NONOPAQUE:   declare void @nestedStruct(%struct.NestedStruct*)
; X64-WIN-OPAQUE:   declare void @nestedStruct(ptr)

; Function Attrs: convergent
declare void @oneElementFLoatArray(%struct.OneElementFloatArray* byval(%struct.OneElementFloatArray) align 4) #2
; X64-LINUX: declare void @oneElementFLoatArray(float)
; X64-WIN:   declare void @oneElementFLoatArray(i32)

; Function Attrs: convergent
declare void @outOfIntRegisters(%struct.TwoLongs* byval(%struct.TwoLongs) align 8, %struct.TwoLongs* byval(%struct.TwoLongs) align 8, %struct.SingleInt* byval(%struct.SingleInt) align 4, %struct.TwoLongs* byval(%struct.TwoLongs) align 8, %struct.SingleInt* byval(%struct.SingleInt) align 4, %struct.SingleInt* byval(%struct.SingleInt) align 4) #2
; X64-LINUX-NONOPAQUE: declare void @outOfIntRegisters(i64, i64, i64, i64, i32, %struct.TwoLongs*, i32, %struct.SingleInt*)
; X64-LINUX-OPAQUE: declare void @outOfIntRegisters(i64, i64, i64, i64, i32, ptr, i32, ptr)
; X64-WIN-NONOPAQUE: declare void @outOfIntRegisters(%struct.TwoLongs*, %struct.TwoLongs*, i32, %struct.TwoLongs*, i32, i32)
; X64-WIN-OPAQUE: declare void @outOfIntRegisters(ptr, ptr, i32, ptr, i32, i32)

; Function Attrs: convergent
declare void @outOfSSERegisters(%struct.TwoDoubles* byval(%struct.TwoDoubles) align 8, %struct.TwoDoubles* byval(%struct.TwoDoubles) align 8, %struct.TwoDoubles* byval(%struct.TwoDoubles) align 8, %struct.SingleFloat* byval(%struct.SingleFloat) align 4, %struct.TwoDoubles* byval(%struct.TwoDoubles) align 8, %struct.SingleFloat* byval(%struct.SingleFloat) align 4, %struct.SingleFloat* byval(%struct.SingleFloat) align 4) #2
; X64-LINUX-NONOPAQUE: declare void @outOfSSERegisters(double, double, double, double, double, double, float, %struct.TwoDoubles*, float, %struct.SingleFloat*)
; X64-LINUX-OPAQUE: declare void @outOfSSERegisters(double, double, double, double, double, double, float, ptr, float, ptr)
; X64-WIN-NONOPAQUE: declare void @outOfSSERegisters(%struct.TwoDoubles*, %struct.TwoDoubles*, %struct.TwoDoubles*, i32, %struct.TwoDoubles*, i32, i32)
; X64-WIN-OPAQUE: declare void @outOfSSERegisters(ptr, ptr, ptr, i32, ptr, i32, i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

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
!5 = !{void ()* @test}


; DEBUGIFY-NOT: WARNING
