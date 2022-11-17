; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

; CHECK: @t1
define void @t1() {
entry:
  ret void
}

;;implicit args

; CHECK: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: %pWorkDim = bitcast i8* [[GEP0]] to { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }*
; CHECK-NEXT: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK-NEXT: %InternalLocalSize_0 = load i32, i32* [[GEP1]]
; CHECK-NEXT: [[GEP2:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK-NEXT: %InternalLocalSize_1 = load i32, i32* [[GEP2]]
; CHECK-NEXT: [[GEP3:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK-NEXT: %InternalLocalSize_2 = load i32, i32* [[GEP3]]
; CHECK-NEXT: [[GEP4:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK-NEXT: %GlobalOffset_0 = load i32, i32* [[GEP4]]
; CHECK-NEXT: [[GEP5:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK-NEXT: %GlobalOffset_1 = load i32, i32* [[GEP5]]
; CHECK-NEXT: [[GEP6:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}*, [3 x i32], [2 x [3 x i32]], [3 x i32] }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK-NEXT: %GlobalOffset_2 = load i32, i32* [[GEP6]]
; CHECK-NEXT: [[GEP7:%[a-zA-Z0-9]+]] = getelementptr i32, i32* %pWGId, i32 0
; CHECK-NEXT: %GroupID_0 = load i32, i32* [[GEP7]]
; CHECK-NEXT: [[GEP8:%[a-zA-Z0-9]+]] = getelementptr i32, i32* %pWGId, i32 1
; CHECK-NEXT: %GroupID_1 = load i32, i32* [[GEP8]]
; CHECK-NEXT: [[GEP9:%[a-zA-Z0-9]+]] = getelementptr i32, i32* %pWGId, i32 2
; CHECK-NEXT: %GroupID_2 = load i32, i32* [[GEP9]]
; CHECK-NEXT: [[MUL0:%[a-zA-Z0-9]+]] = mul i32 %InternalLocalSize_0, %GroupID_0
; CHECK-NEXT: [[ADD0:%[a-zA-Z0-9]+]] = add i32 [[MUL0]], %GlobalOffset_0
; CHECK-NEXT: [[MUL1:%[a-zA-Z0-9]+]] = mul i32 %InternalLocalSize_1, %GroupID_1
; CHECK-NEXT: [[ADD1:%[a-zA-Z0-9]+]] = add i32 [[MUL1]], %GlobalOffset_1
; CHECK-NEXT: [[MUL2:%[a-zA-Z0-9]+]] = mul i32 %InternalLocalSize_2, %GroupID_2
; CHECK-NEXT: [[ADD2:%[a-zA-Z0-9]+]] = add i32 [[MUL2]], %GlobalOffset_2
; CHECK-NEXT: [[IV0:%[a-zA-Z0-9]+]] = insertvalue [4 x i32] undef, i32 [[ADD0]], 0
; CHECK-NEXT: [[IV1:%[a-zA-Z0-9]+]] = insertvalue [4 x i32] [[IV0]], i32 [[ADD1]], 1
; CHECK-NEXT: %BaseGlbId = insertvalue [4 x i32] [[IV1]], i32 [[ADD2]], 2
; CHECK-NEXT: [[MUL01:%[0-9]+]] = mul nuw nsw i32 %InternalLocalSize_0, %InternalLocalSize_1
; CHECK-NEXT: %LocalSizeProd = mul nuw nsw i32 [[MUL01]], %InternalLocalSize_2
; CHECK-NEXT: %BarrierBufferSize = mul nuw nsw i32 0, %LocalSizeProd
; CHECK-NEXT: %pSpecialBuf = alloca i8, i32 %BarrierBufferSize, align 128
; CHECK-NEXT: ret void

!sycl.kernels = !{!0}
!0 = !{void ()* @t1}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-33: WARNING: Instruction with empty DebugLoc in function {{.*}}
; DEBUGIFY-NOT: WARNING
