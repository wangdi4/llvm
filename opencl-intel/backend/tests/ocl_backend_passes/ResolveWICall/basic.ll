; RUN: oclopt -add-implicit-args -resolve-wi-call -S %s -o - | FileCheck %s
; TODO: add checks...
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
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0
; CHECK: %LocalSize_0 = load i64* [[GEP]]
; CHECK: ret i64 %LocalSize_0
  %c = tail call i64 @_Z14get_local_sizej(i32 0) nounwind
  ret i64 %c
}
define i64 @GLS1() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 1
; CHECK: %LocalSize_1 = load i64* [[GEP]]
; CHECK: ret i64 %LocalSize_1
  %c = tail call i64 @_Z14get_local_sizej(i32 1) nounwind
  ret i64 %c
}
define i64 @GLS2() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 2
; CHECK: %LocalSize_2 = load i64* [[GEP]]
; CHECK: ret i64 %LocalSize_2
  %c = tail call i64 @_Z14get_local_sizej(i32 2) nounwind
  ret i64 %c
}
define i64 @GLSX(i32 %x) {
  %c = tail call i64 @_Z14get_local_sizej(i32 %x) nounwind
  ret i64 %c
}

define i64 @GBGI0() {
; CHECK-LABEL: GBGI0
; CHECK: %BaseGlobalID_0 = extractvalue [4 x i64] %BaseGlbId, 0
; CHECK: ret i64 %BaseGlobalID_0
  %c = tail call i64 @get_base_global_id.(i32 0) nounwind
  ret i64 %c
}
define i64 @GBGI1() {
; CHECK: %BaseGlobalID_1 = extractvalue [4 x i64] %BaseGlbId, 1
; CHECK: ret i64 %BaseGlobalID_1
  %c = tail call i64 @get_base_global_id.(i32 1) nounwind
  ret i64 %c
}
define i64 @GBGI2() {
; CHECK: %BaseGlobalID_2 = extractvalue [4 x i64] %BaseGlbId, 2
; CHECK: ret i64 %BaseGlobalID_2
  %c = tail call i64 @get_base_global_id.(i32 2) nounwind
  ret i64 %c
}
define i64 @GBGIX(i32 %x) {
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
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 0
; CHECK: [[LD:%[a-zA-Z0-9]+]] = load i64* [[GEP]]
; CHECK: %WorkDim = trunc i64 [[LD]] to i32
; CHECK: ret i32 %WorkDim
  %c = tail call i32 @_Z12get_work_dimj() nounwind
  ret i32 %c
}

define i64 @GGS0() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 2, i32 0
; CHECK: %GlobalSize_0 = load i64* [[GEP]]
; CHECK: ret i64 %GlobalSize_0
  %c = tail call i64 @_Z15get_global_sizej(i32 0) nounwind
  ret i64 %c
}
define i64 @GGS1() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 2, i32 1
; CHECK: %GlobalSize_1 = load i64* [[GEP]]
; CHECK: ret i64 %GlobalSize_1
  %c = tail call i64 @_Z15get_global_sizej(i32 1) nounwind
  ret i64 %c
}
define i64 @GGS2() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 2, i32 2
; CHECK: %GlobalSize_2 = load i64* [[GEP]]
; CHECK: ret i64 %GlobalSize_2
  %c = tail call i64 @_Z15get_global_sizej(i32 2) nounwind
  ret i64 %c
}
define i64 @GGSX(i32 %x) {
  %c = tail call i64 @_Z15get_global_sizej(i32 %x) nounwind
  ret i64 %c
}

define i64 @GNG0() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 4, i32 0
; CHECK: %NumGroups_0 = load i64* [[GEP]]
; CHECK: ret i64 %NumGroups_0
  %c = tail call i64 @_Z14get_num_groupsj(i32 0) nounwind
  ret i64 %c
}
define i64 @GNG1() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 4, i32 1
; CHECK: %NumGroups_1 = load i64* [[GEP]]
; CHECK: ret i64 %NumGroups_1
  %c = tail call i64 @_Z14get_num_groupsj(i32 1) nounwind
  ret i64 %c
}
define i64 @GNG2() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 4, i32 2
; CHECK: %NumGroups_2 = load i64* [[GEP]]
; CHECK: ret i64 %NumGroups_2
  %c = tail call i64 @_Z14get_num_groupsj(i32 2) nounwind
  ret i64 %c
}
define i64 @GNGX(i32 %x) {
  %c = tail call i64 @_Z14get_num_groupsj(i32 %x) nounwind
  ret i64 %c
}

define i64 @GGI0() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64* %pWGId, i32 0
; CHECK: %GroupID_0 = load i64* [[GEP]]
; CHECK: ret i64 %GroupID_0
  %c = tail call i64 @_Z12get_group_idj(i32 0) nounwind
  ret i64 %c
}
define i64 @GGI1() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64* %pWGId, i32 1
; CHECK: %GroupID_1 = load i64* [[GEP]]
; CHECK: ret i64 %GroupID_1
  %c = tail call i64 @_Z12get_group_idj(i32 1) nounwind
  ret i64 %c
}
define i64 @GGI2() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i64* %pWGId, i32 2
; CHECK: %GroupID_2 = load i64* [[GEP]]
; CHECK: ret i64 %GroupID_2
  %c = tail call i64 @_Z12get_group_idj(i32 2) nounwind
  ret i64 %c
}
define i64 @GGIX(i32 %x) {
  %c = tail call i64 @_Z12get_group_idj(i32 %x) nounwind
  ret i64 %c
}

define i64 @GGO0() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i64* [[GEP]]
; CHECK: ret i64 %GlobalOffset_0
  %c = tail call i64 @_Z17get_global_offsetj(i32 0) nounwind
  ret i64 %c
}
define i64 @GGO1() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i64* [[GEP]]
; CHECK: ret i64 %GlobalOffset_1
  %c = tail call i64 @_Z17get_global_offsetj(i32 1) nounwind
  ret i64 %c
}
define i64 @GGO2() {
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i64, [3 x i64], [3 x i64], [3 x i64], [3 x i64], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i64* [[GEP]]
; CHECK: ret i64 %GlobalOffset_2
  %c = tail call i64 @_Z17get_global_offsetj(i32 2) nounwind
  ret i64 %c
}
define i64 @GGOX(i32 %x) {
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
declare i8* @get_special_buffer.()
