; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

;; This is Win32 target, pointer size is 4 bytes
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{ <4 x i32> , i8}>
%struct.struct2 = type <{ i32 ,i32 ,i32 }>

; CHECK: @t1
define void @t1(%struct.struct1* %arg0, %struct.struct2* %arg1, <4 x i32>* byval(<4 x i32>) %arg2) {
entry:
  ret void
}

; CHECK: %explicit_2.ptr = alloca <4 x i32>, align 16

;; struct1* arg0 - 4 bytes - expected alignment: 0
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: [[ARG0_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1**
; CHECK-NEXT: %explicit_0 = load %struct.struct1*, %struct.struct1** [[ARG0_TYPECAST]], align 4
;; struct2* arg1 - 4 bytes - expected alignment: 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 4
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to %struct.struct2**
; CHECK-NEXT: %explicit_1 = load %struct.struct2*, %struct.struct2** [[ARG1_TYPECAST]], align 4
;; int4 byvalue arg2, size is the actual size not the pointer size - expected alignment:16 (4+12 = 16 so it's aligned to 16)
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK-NEXT: %explicit_2 = bitcast i8* [[ARG2_BUFF_INDEX]] to <4 x i32>*
;;implicit args

;; copy of byval arg
; CHECK: [[EXPLICIT2PtrBC:%[0-9]+]] = bitcast <4 x i32>* %explicit_2.ptr to i8*
; CHECK-NEXT: [[EXPLICIT2BC:%[0-9]+]] = bitcast <4 x i32>* %explicit_2 to i8*
; CHECK-NEXT: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[EXPLICIT2PtrBC]], i8* align 1 [[EXPLICIT2BC]], i64 16, i1 false)
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{void (%struct.struct1*, %struct.struct2*, <4 x i32>*)* @t1}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} bitcast
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} load
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} add
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} insertvalue
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} mul
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function t1 {{.*}} alloca
; DEBUGIFY-NOT: WARNING
