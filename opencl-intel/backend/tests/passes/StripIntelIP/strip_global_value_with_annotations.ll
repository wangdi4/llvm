;; Compiled from:
;; --------------
;; __attribute__((__numreadports__(2))) constant int global_const7 = 1;
;;
;; where numreadports() is FPGA attribute (no-op for emulator)
;;
;; Purpose of the test to check if we nuke not only unused GV from a module, but
;; @llvm.global.annotations appended to them as well.

; RUN: %oclopt -strip-intel-ip -verify -S %s | FileCheck %s

; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@global_const7 = addrspace(2) constant i32 1, align 4
; CHECK-NOT: @global_const7
@.str = internal unnamed_addr constant [33 x i8] c"{memory:DEFAULT}{numreadports:2}\00", section "llvm.metadata"
; CHECK-NOT: @.str
@.str.1 = internal unnamed_addr constant [2 x i8] c"1\00", section "llvm.metadata"
; CHECK-NOT: @.str.1
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i32 } { i8* addrspacecast (i8 addrspace(2)* bitcast (i32 addrspace(2)* @global_const7 to i8 addrspace(2)*) to i8*), i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([2 x i8], [2 x i8]* @.str.1, i32 0, i32 0), i32 2 }], section "llvm.metadata"
; CHECK-NOT: @llvm.global.annotations

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!sycl.kernels = !{!2}
!opencl.global_variable_total_size = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{i32 0}
