; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s

; Make sure the pass doesn't crash when alignment is not specified for byval args.
; In the code, 1 byte alignment is set in such cases.

; CHECK: declare void @__intel_indirect_call.6(ptr addrspace(4), ptr addrspace(4) sret(%"class.sycl::_V1::vec"), ptr)

%"class.sycl::_V1::vec" = type { <2 x i8> }

declare void @__intel_indirect_call.6(ptr addrspace(4) %0, ptr addrspace(4) sret(%"class.sycl::_V1::vec") %1, ptr byval(%"class.sycl::_V1::vec") %2, ...)
