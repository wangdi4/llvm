; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-rewrite-pipes -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

;; This test is used to check whether `ReplaceChecker` and other assertions work well 
;; when `ConstantPipeStorage` is extended to `ConstantPipeStorageExp`.

; // experimental struct to hold properties replacing ConstantPipeStorage
; struct ConstantPipeStorageExp : ConstantPipeStorage {
;   int32_t _ReadyLatency;
;   int32_t _BitsPerSymbol;
;   bool _UsesValid;
;   bool _FirstSymInHighOrderBits;
;   protocol_name _Protocol;
; };
%structtype = type { i32, i32, i32, i32, i32, i8, i8, i32 }
%struct.ConstantPipeStorage = type { i32, i32, i32 }

@_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI14interH2DPipeIDEEfLi3ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE9m_StorageE = linkonce_odr addrspace(1) constant %structtype { i32 4, i32 4, i32 3, i32 0, i32 1, i8 1, i8 0, i32 0 }, align 4

; Function Attrs: nounwind
declare ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS419ConstantPipeStorage(ptr addrspace(4)) #0

; Function Attrs: nounwind
define internal float @_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI14interH2DPipeIDEEfLi3ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE4readIJEEEfv() #0 {
entry:
  %TempData = alloca float, align 4
  %TempData.ascast = addrspacecast ptr %TempData to ptr addrspace(4)
  %call = call ptr addrspace(1) @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS419ConstantPipeStorage(ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI14interH2DPipeIDEEfLi3ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE9m_StorageE to ptr addrspace(4))) #7
  %0 = load float, ptr addrspace(4) %TempData.ascast, align 4
  ret float %0
}

; DEBUGIFY: PASS
