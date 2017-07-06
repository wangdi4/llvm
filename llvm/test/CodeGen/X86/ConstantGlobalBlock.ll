; TODO: upstream to llvm.org.
; REQUIRES: intel_opencl
; RUN: llc < %s -filetype=asm -mtriple x86_64

; Check if AsmPrinter doesn't produce llvm error
; while emiting global constant with addrspacecast

%struct.__block_descriptor = type { i64, i64 }

@__block_literal_global = external addrspace(1) constant { i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* }, align 4
@globalBlock = addrspace(2) constant i32 () addrspace(4)* addrspacecast (i32 () addrspace(1)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } addrspace(1)* @__block_literal_global to i32 () addrspace(1)*) to i32 () addrspace(4)*), align 4

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.1"}
