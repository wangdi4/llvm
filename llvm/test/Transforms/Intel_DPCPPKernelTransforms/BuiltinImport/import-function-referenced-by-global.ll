; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -dpcpp-kernel-builtin-import %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -passes=dpcpp-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-builtin-lib=%t.rtl.bc -passes=dpcpp-kernel-builtin-import %s -S | FileCheck %s

;*********************************************************************************
; When importing we need to fully explore call tree for functions referenced
; in global variables.
;*********************************************************************************

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-gnu-elf"

%opencl.image2d_ro_t = type opaque

; CHECK: @coord_translate_i_callback = constant [2 x <4 x i32> (i8*, <4 x i32>)*] [<4 x i32> (i8*, <4 x i32>)* @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i], align 16

; Function Attrs: nounwind readonly
declare i8* @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t addrspace(1)*, i32, <2 x i32>)


; Function Attrs: nounwind
define i8* @test_bgra8888(%opencl.image2d_ro_t addrspace(1)* %srcimg, i32 %sampler, <2 x i32> %vecinit4) {
entry:
  %call = tail call i8* @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t addrspace(1)* %srcimg, i32 %sampler, <2 x i32> %vecinit4)
  ret i8*  %call
}

; CHECK: define i8* @test_bgra8888
; CHECK: define internal <4 x i32> @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i
; CHECK: define internal <4 x i32> @_Z25trans_coord_int_UNDEFINEDPvDv4_i
; CHECK: define internal <4 x i32> @_Z3minDv4_iS_
; CHECK: define internal <4 x i32> @_Z3maxDv4_iS_
; CHECK: define internal i8* @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i
; CHECK: define internal i8* @call_coord_translate_i_callback

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc in function _Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z25trans_coord_int_UNDEFINEDPvDv4_i
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function _Z3minDv4_iS_
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function _Z3maxDv4_iS_
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function _Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i
; DEBUGIFY-COUNT-5: WARNING: Instruction with empty DebugLoc in function call_coord_translate_i_callback
; DEBUGIFY-NOT: WARNING
