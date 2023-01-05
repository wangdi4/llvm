; RUN: opt -dpcpp-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=dpcpp-kernel-rewrite-pipes -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

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
%opencl.pipe_ro_t.9 = type opaque
%struct.ConstantPipeStorage = type { i32, i32, i32 }

@_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI14interH2DPipeIDEEfLi3ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE9m_StorageE = linkonce_odr addrspace(1) constant %structtype { i32 4, i32 4, i32 3, i32 0, i32 1, i8 1, i8 0, i32 0 }, align 4

; Function Attrs: nounwind
declare %opencl.pipe_ro_t.9 addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS419ConstantPipeStorage(%struct.ConstantPipeStorage addrspace(4)*) #0

; Function Attrs: nounwind
define internal float @_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI14interH2DPipeIDEEfLi3ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE4readIJEEEfv() #0 {
entry:
  %TempData = alloca float, align 4
  %TempData.ascast = addrspacecast float* %TempData to float addrspace(4)*
  %call = call %opencl.pipe_ro_t.9 addrspace(1)* @_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS419ConstantPipeStorage(%struct.ConstantPipeStorage addrspace(4)* addrspacecast (%struct.ConstantPipeStorage addrspace(1)* bitcast (%structtype addrspace(1)* @_ZN4sycl3ext5intel9prototype8internal4pipeIN6detail14HostPipePipeIdI14interH2DPipeIDEEfLi3ELi0ELi1ELb1ELb0ELNS3_13protocol_nameE0EE9m_StorageE to %struct.ConstantPipeStorage addrspace(1)*) to %struct.ConstantPipeStorage addrspace(4)*)) #7
  %0 = bitcast float* %TempData to i8*
  %1 = load float, float addrspace(4)* %TempData.ascast, align 4
  %2 = bitcast float* %TempData to i8*
  ret float %1
}

; DEBUGIFY: PASS
