; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-ARG
; RUN: opt -passes='sycl-kernel-add-tls-globals,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-tls-globals,sycl-kernel-resolve-wi-call' -S %s | FileCheck %s --check-prefixes CHECK,CHECK-TLS

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; get_global_id is not handled by the Resolve WI Calls pass
;define i64 @GGI0() {
;  %c = tail call i64 @_Z13get_global_idj(i32 0) nounwind
;  ret i64 %c
;}
;define i64 @GGI1() {
;  %c = tail call i64 @_Z13get_global_idj(i32 1) nounwind
;  ret i64 %c
;}
;define i64 @GGI2() {
;  %c = tail call i64 @_Z13get_global_idj(i32 2) nounwind
;  ret i64 %c
;}


define i64 @GLS0() {
; CHECK-LABEL: @GLS0
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 8, i32 0, i32 0
; CHECK: %InternalLocalSize_0 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalLocalSize_0
  %c = tail call i64 @_Z14get_local_sizej(i32 0) nounwind
  ret i64 %c
}
define i64 @GLS1() {
; CHECK-LABEL: @GLS1
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 1
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 8, i32 0, i32 1
; CHECK: %InternalLocalSize_1 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalLocalSize_1
  %c = tail call i64 @_Z14get_local_sizej(i32 1) nounwind
  ret i64 %c
}
define i64 @GLS2() {
; CHECK-LABEL: @GLS2
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 2
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 8, i32 0, i32 2
; CHECK: %InternalLocalSize_2 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalLocalSize_2
  %c = tail call i64 @_Z14get_local_sizej(i32 2) nounwind
  ret i64 %c
}
define i64 @GLSX(i32 %x) {
; CHECK-LABEL: @GLSX
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 8, i32 0, i32 %x
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 8, i32 0, i32 %x
; CHECK: %InternalLocalSize_var = load i64, ptr [[GEP]]
  %c = tail call i64 @_Z14get_local_sizej(i32 %x) nounwind
  ret i64 %c
}

define i64 @GBGI0() {
; CHECK-LABEL: @GBGI0
; CHECK-ARG: %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load [4 x i64], ptr @__BaseGlbId
; CHECK-TLS: %BaseGlobalID_0 = extractvalue [4 x i64] [[LD]], 0
; CHECK: ret i64 %BaseGlobalID_0
  %c = tail call i64 @get_base_global_id.(i32 0) nounwind
  ret i64 %c
}
define i64 @GBGI1() {
; CHECK-LABEL: @GBGI1
; CHECK-ARG: %BaseGlobalID_1 = extractvalue [4 x i64] %BaseGlbId, 1
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load [4 x i64], ptr @__BaseGlbId
; CHECK-TLS: %BaseGlobalID_1 = extractvalue [4 x i64] [[LD]], 1
; CHECK: ret i64 %BaseGlobalID_1
  %c = tail call i64 @get_base_global_id.(i32 1) nounwind
  ret i64 %c
}
define i64 @GBGI2() {
; CHECK-LABEL: @GBGI2
; CHECK-ARG: %BaseGlobalID_2 = extractvalue [4 x i64] %BaseGlbId, 2
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load [4 x i64], ptr @__BaseGlbId
; CHECK-TLS: %BaseGlobalID_2 = extractvalue [4 x i64] [[LD]], 2
; CHECK: ret i64 %BaseGlobalID_2
  %c = tail call i64 @get_base_global_id.(i32 2) nounwind
  ret i64 %c
}
define i64 @GBGIX(i32 %x) {
; CHECK-LABEL: @GBGIX
  %c = tail call i64 @get_base_global_id.(i32 %x) nounwind
  ret i64 %c
}

; get_local_id is not handled by the Resolve WI Calls pass
;define i64 @GLI0() {
;  %c = tail call i64 @_Z12get_local_idj(i32 0) nounwind
;  ret i64 %c
;}
;define i64 @GLI1() {
;  %c = tail call i64 @_Z12get_local_idj(i32 1) nounwind
;  ret i64 %c
;}
;define i64 @GLI2() {
;  %c = tail call i64 @_Z12get_local_idj(i32 2) nounwind
;  ret i64 %c
;}

define i32 @GWD() {
; CHECK-LABEL: @GWD
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 0
; CHECK: [[LD:%[a-zA-Z0-9]+]] = load i64, ptr [[GEP]]
; CHECK: %WorkDim = trunc i64 [[LD]] to i32
; CHECK: ret i32 %WorkDim
  %c = tail call i32 @_Z12get_work_dimj() nounwind
  ret i32 %c
}

define i64 @GGS0() {
; CHECK-LABEL: @GGS0
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 7, i32 0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 7, i32 0
; CHECK: %InternalGlobalSize_0 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalGlobalSize_0
  %c = tail call i64 @_Z15get_global_sizej(i32 0) nounwind
  ret i64 %c
}
define i64 @GGS1() {
; CHECK-LABEL: @GGS1
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 7, i32 1
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 7, i32 1
; CHECK: %InternalGlobalSize_1 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalGlobalSize_1
  %c = tail call i64 @_Z15get_global_sizej(i32 1) nounwind
  ret i64 %c
}
define i64 @GGS2() {
; CHECK-LABEL: @GGS2
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 7, i32 2
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 7, i32 2
; CHECK: %InternalGlobalSize_2 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalGlobalSize_2
  %c = tail call i64 @_Z15get_global_sizej(i32 2) nounwind
  ret i64 %c
}
define i64 @GGSX(i32 %x) {
; CHECK-LABEL: @GGSX
  %c = tail call i64 @_Z15get_global_sizej(i32 %x) nounwind
  ret i64 %c
}

define i64 @GNG0() {
; CHECK-LABEL: @GNG0
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 9, i32 0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 9, i32 0
; CHECK: %InternalNumGroups_0 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalNumGroups_0
  %c = tail call i64 @_Z14get_num_groupsj(i32 0) nounwind
  ret i64 %c
}
define i64 @GNG1() {
; CHECK-LABEL: @GNG1
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 9, i32 1
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 9, i32 1
; CHECK: %InternalNumGroups_1 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalNumGroups_1
  %c = tail call i64 @_Z14get_num_groupsj(i32 1) nounwind
  ret i64 %c
}
define i64 @GNG2() {
; CHECK-LABEL: @GNG2
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 9, i32 2
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 9, i32 2
; CHECK: %InternalNumGroups_2 = load i64, ptr [[GEP]]
; CHECK: ret i64 %InternalNumGroups_2
  %c = tail call i64 @_Z14get_num_groupsj(i32 2) nounwind
  ret i64 %c
}
define i64 @GNGX(i32 %x) {
; CHECK-LABEL: @GNGX
  %c = tail call i64 @_Z14get_num_groupsj(i32 %x) nounwind
  ret i64 %c
}

define i64 @GGI0() {
; CHECK-LABEL: @GGI0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWGId
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr [[LD]], i32 0
; CHECK-TLS: %GroupID_0 = load i64, ptr [[GEP]]
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr %pWGId, i32 0
; CHECK-ARG: %GroupID_0 = load i64, ptr [[GEP]]
; CHECK: ret i64 %GroupID_0
  %c = tail call i64 @_Z12get_group_idj(i32 0) nounwind
  ret i64 %c
}
define i64 @GGI1() {
; CHECK-LABEL: @GGI1
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr %pWGId, i32 1
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWGId
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr [[LD]], i32 1
; CHECK: %GroupID_1 = load i64, ptr [[GEP]]
; CHECK: ret i64 %GroupID_1
  %c = tail call i64 @_Z12get_group_idj(i32 1) nounwind
  ret i64 %c
}
define i64 @GGI2() {
; CHECK-LABEL: @GGI2
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr %pWGId, i32 2
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWGId
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64, ptr [[LD]], i32 2
; CHECK: %GroupID_2 = load i64, ptr [[GEP]]
; CHECK: ret i64 %GroupID_2
  %c = tail call i64 @_Z12get_group_idj(i32 2) nounwind
  ret i64 %c
}
define i64 @GGIX(i32 %x) {
; CHECK-LABEL: @GGIX
  %c = tail call i64 @_Z12get_group_idj(i32 %x) nounwind
  ret i64 %c
}

define i64 @GGO0() {
; CHECK-LABEL: @GGO0
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 0
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64, ptr [[GEP]]
; CHECK: ret i64 %GlobalOffset_0
  %c = tail call i64 @_Z17get_global_offsetj(i32 0) nounwind
  ret i64 %c
}
define i64 @GGO1() {
; CHECK-LABEL: @GGO1
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 1
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64, ptr [[GEP]]
; CHECK: ret i64 %GlobalOffset_1
  %c = tail call i64 @_Z17get_global_offsetj(i32 1) nounwind
  ret i64 %c
}
define i64 @GGO2() {
; CHECK-LABEL: @GGO2
; CHECK-ARG: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr %pWorkDim, i32 0, i32 1, i32 2
; CHECK-TLS: [[LD:%[a-zA-Z0-9]+]] = load ptr, ptr @__pWorkDim
; CHECK-TLS: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], ptr, ptr, [3 x i64], [2 x [3 x i64]], [3 x i64] }, ptr [[LD]], i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64, ptr [[GEP]]
; CHECK: ret i64 %GlobalOffset_2
  %c = tail call i64 @_Z17get_global_offsetj(i32 2) nounwind
  ret i64 %c
}
define i64 @GGOX(i32 %x) {
; CHECK-LABEL: @GGOX
  %c = tail call i64 @_Z17get_global_offsetj(i32 %x) nounwind
  ret i64 %c
}

declare i32 @_Z12get_work_dimj()
declare i64 @get_base_global_id.(i32)
declare i64 @_Z12get_local_idj(i32)
declare i64 @_Z13get_global_idj(i32)
declare i64 @_Z15get_global_sizej(i32)
declare i64 @_Z14get_local_sizej(i32)
declare i64 @_Z14get_num_groupsj(i32)
declare i64 @_Z12get_group_idj(i32)
declare i64 @_Z17get_global_offsetj(i32)
declare ptr @get_special_buffer.()

; DEBUGIFY-NOT: WARNING
