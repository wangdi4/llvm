; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{ <4 x i32> , i8}>

; CHECK: @t1
define void @t1(%struct.struct1* byval(%struct.struct1) %arg0, %struct.struct1* %arg1) {
entry:
  ret void
}

;; struct1 arg0 - packed struct - 17 bytes - byval (no load)
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1*
;; struct1* arg1 - 4 bytes - expected alignment: 20
; CHECK: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 20
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to %struct.struct1**
; CHECK-NEXT: %explicit_1 = load %struct.struct1*, %struct.struct1** [[ARG1_TYPECAST]], align 4
;;implicit args

;; copy of byval arg
; CHECK: [[EXPLICIT0PtrBC:%[0-9]+]] = bitcast %struct.struct1* %explicit_0.ptr to i8*
; CHECK-NEXT: [[EXPLICIT0BC:%[0-9]+]] = bitcast %struct.struct1* %explicit_0 to i8*
; CHECK-NEXT: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 [[EXPLICIT0PtrBC]], i8* align 1 [[EXPLICIT0BC]], i64 17, i1 false)
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{void (%struct.struct1*, %struct.struct1*)* @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-39: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
