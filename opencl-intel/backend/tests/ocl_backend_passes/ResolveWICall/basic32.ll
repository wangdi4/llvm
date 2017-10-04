; RUN: %oclopt -add-implicit-args -resolve-wi-call -S %s -o - | FileCheck %s
; TODO: add checks...
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i32:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i386-pc-linux"

; get_global_id is not handled by the Resolve WI Calls pass
;define i32 @GGI0() {
;  %c = tail call i32 @_Z13get_global_idj(i32 0) nounwind
;  ret i32 %c
;}
;define i32 @GGI1() {
;  %c = tail call i32 @_Z13get_global_idj(i32 1) nounwind
;  ret i32 %c
;}
;define i32 @GGI2() {
;  %c = tail call i32 @_Z13get_global_idj(i32 2) nounwind
;  ret i32 %c
;}


define i32 @GLS0() {
; CHECK-LABEL: @GLS0
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 0
; CHECK: %LocalSize_0 = load i32, i32* [[GEP]]
; CHECK: ret i32 %LocalSize_0
  %c = tail call i32 @_Z14get_local_sizej(i32 0) nounwind
  ret i32 %c
}
define i32 @GLS1() {
; CHECK-LABEL: @GLS1
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 1
; CHECK: %LocalSize_1 = load i32, i32* [[GEP]]
; CHECK: ret i32 %LocalSize_1
  %c = tail call i32 @_Z14get_local_sizej(i32 1) nounwind
  ret i32 %c
}
define i32 @GLS2() {
; CHECK-LABEL: @GLS2
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 2
; CHECK: %LocalSize_2 = load i32, i32* [[GEP]]
; CHECK: ret i32 %LocalSize_2
  %c = tail call i32 @_Z14get_local_sizej(i32 2) nounwind
  ret i32 %c
}
define i32 @GLSX(i32 %x) {
; CHECK-LABEL: @GLSX
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 3, i32 0, i32 %x
; CHECK: %LocalSize_var = load i32, i32* [[GEP]]
  %c = tail call i32 @_Z14get_local_sizej(i32 %x) nounwind
  ret i32 %c
}

define i32 @GBGI0() {
; CHECK-LABEL: @GBGI0
; CHECK: %BaseGlobalID_0 = extractvalue [4 x i32] %BaseGlbId, 0
; CHECK: ret i32 %BaseGlobalID_0
  %c = tail call i32 @get_base_global_id.(i32 0) nounwind
  ret i32 %c
}
define i32 @GBGI1() {
; CHECK-LABEL: @GBGI1
; CHECK: %BaseGlobalID_1 = extractvalue [4 x i32] %BaseGlbId, 1
; CHECK: ret i32 %BaseGlobalID_1
  %c = tail call i32 @get_base_global_id.(i32 1) nounwind
  ret i32 %c
}
define i32 @GBGI2() {
; CHECK-LABEL: @GBGI2
; CHECK: %BaseGlobalID_2 = extractvalue [4 x i32] %BaseGlbId, 2
; CHECK: ret i32 %BaseGlobalID_2
  %c = tail call i32 @get_base_global_id.(i32 2) nounwind
  ret i32 %c
}
define i32 @GBGIX(i32 %x) {
; CHECK-LABEL: @GBGIX
  %c = tail call i32 @get_base_global_id.(i32 %x) nounwind
  ret i32 %c
}

; get_local_id is not handled by the Resolve WI Calls pass
;define i32 @GLI0() {
;  %c = tail call i32 @_Z12get_local_idj(i32 0) nounwind
;  ret i32 %c
;}
;define i32 @GLI1() {
;  %c = tail call i32 @_Z12get_local_idj(i32 1) nounwind
;  ret i32 %c
;}
;define i32 @GLI2() {
;  %c = tail call i32 @_Z12get_local_idj(i32 2) nounwind
;  ret i32 %c
;}

define i32 @GWD() {
; CHECK-LABEL: @GWD
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 0
; CHECK: %WorkDim = load i32, i32* [[GEP]]
; CHECK: ret i32 %WorkDim
  %c = tail call i32 @_Z12get_work_dimj() nounwind
  ret i32 %c
}

define i32 @GGS0() {
; CHECK-LABEL: @GGS0
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 2, i32 0
; CHECK: %GlobalSize_0 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GlobalSize_0
  %c = tail call i32 @_Z15get_global_sizej(i32 0) nounwind
  ret i32 %c
}
define i32 @GGS1() {
; CHECK-LABEL: @GGS1
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 2, i32 1
; CHECK: %GlobalSize_1 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GlobalSize_1
  %c = tail call i32 @_Z15get_global_sizej(i32 1) nounwind
  ret i32 %c
}
define i32 @GGS2() {
; CHECK-LABEL: @GGS2
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 2, i32 2
; CHECK: %GlobalSize_2 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GlobalSize_2
  %c = tail call i32 @_Z15get_global_sizej(i32 2) nounwind
  ret i32 %c
}
define i32 @GGSX(i32 %x) {
; CHECK-LABEL: @GGSX
  %c = tail call i32 @_Z15get_global_sizej(i32 %x) nounwind
  ret i32 %c
}

define i32 @GNG0() {
; CHECK-LABEL: @GNG0
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 4, i32 0
; CHECK: %NumGroups_0 = load i32, i32* [[GEP]]
; CHECK: ret i32 %NumGroups_0
  %c = tail call i32 @_Z14get_num_groupsj(i32 0) nounwind
  ret i32 %c
}
define i32 @GNG1() {
; CHECK-LABEL: @GNG1
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 4, i32 1
; CHECK: %NumGroups_1 = load i32, i32* [[GEP]]
; CHECK: ret i32 %NumGroups_1
  %c = tail call i32 @_Z14get_num_groupsj(i32 1) nounwind
  ret i32 %c
}
define i32 @GNG2() {
; CHECK-LABEL: @GNG2
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 4, i32 2
; CHECK: %NumGroups_2 = load i32, i32* [[GEP]]
; CHECK: ret i32 %NumGroups_2
  %c = tail call i32 @_Z14get_num_groupsj(i32 2) nounwind
  ret i32 %c
}
define i32 @GNGX(i32 %x) {
; CHECK-LABEL: @GNGX
  %c = tail call i32 @_Z14get_num_groupsj(i32 %x) nounwind
  ret i32 %c
}

define i32 @GGI0() {
; CHECK-LABEL: @GGI0
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i32, i32* %pWGId, i32 0
; CHECK: %GroupID_0 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GroupID_0
  %c = tail call i32 @_Z12get_group_idj(i32 0) nounwind
  ret i32 %c
}
define i32 @GGI1() {
; CHECK-LABEL: @GGI1
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i32, i32* %pWGId, i32 1
; CHECK: %GroupID_1 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GroupID_1
  %c = tail call i32 @_Z12get_group_idj(i32 1) nounwind
  ret i32 %c
}
define i32 @GGI2() {
; CHECK-LABEL: @GGI2
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr i32, i32* %pWGId, i32 2
; CHECK: %GroupID_2 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GroupID_2
  %c = tail call i32 @_Z12get_group_idj(i32 2) nounwind
  ret i32 %c
}
define i32 @GGIX(i32 %x) {
; CHECK-LABEL: @GGIX
  %c = tail call i32 @_Z12get_group_idj(i32 %x) nounwind
  ret i32 %c
}

define i32 @GGO0() {
; CHECK-LABEL: @GGO0
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 0
; CHECK: %GlobalOffset_0 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GlobalOffset_0
  %c = tail call i32 @_Z17get_global_offsetj(i32 0) nounwind
  ret i32 %c
}
define i32 @GGO1() {
; CHECK-LABEL: @GGO1
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 1
; CHECK: %GlobalOffset_1 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GlobalOffset_1
  %c = tail call i32 @_Z17get_global_offsetj(i32 1) nounwind
  ret i32 %c
}
define i32 @GGO2() {
; CHECK-LABEL: @GGO2
; CHECK: [[GEP:%[a-zA-Z0-9]+]] = getelementptr { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }, { i32, [3 x i32], [3 x i32], [2 x [3 x i32]], [3 x i32], {}*, {}* }* %pWorkDim, i32 0, i32 1, i32 2
; CHECK: %GlobalOffset_2 = load i32, i32* [[GEP]]
; CHECK: ret i32 %GlobalOffset_2
  %c = tail call i32 @_Z17get_global_offsetj(i32 2) nounwind
  ret i32 %c
}
define i32 @GGOX(i32 %x) {
; CHECK-LABEL: @GGOX
  %c = tail call i32 @_Z17get_global_offsetj(i32 %x) nounwind
  ret i32 %c
}

declare i32 @_Z12get_work_dimj()
declare i32 @get_base_global_id.(i32)
declare i32 @_Z12get_local_idj(i32)
declare i32 @_Z13get_global_idj(i32)
declare i32 @_Z15get_global_sizej(i32)
declare i32 @_Z14get_local_sizej(i32)
declare i32 @_Z14get_num_groupsj(i32)
declare i32 @_Z12get_group_idj(i32)
declare i32 @_Z17get_global_offsetj(i32)
declare i8* @get_special_buffer.()
