; ASI can convert the addrspacecast i16 into a plain bitcast.
; This test checks that if ASC has the same element type as NewV, ASC is
; replaced with NewV instead of new bitcast from i16 pointer.

; RUN: opt -passes="infer-address-spaces" -S -override-flat-addr-space=4 -o - %s | FileCheck %s
; CHECK: asi_to_bitcast
; CHECK-NOT: bitcast

define void @asi_to_bitcast(ptr addrspace(4) %ptr) {
  %asc0 = addrspacecast ptr addrspace(4) %ptr to ptr
  %gep0 = getelementptr i32, ptr %asc0, i64 9
  %asc1 = addrspacecast ptr %gep0 to ptr addrspace(4)
  %asc3 = addrspacecast ptr addrspace(4) %asc1 to ptr
  %asc2 = addrspacecast ptr addrspace(4) %asc1 to ptr
  %asc4 = addrspacecast ptr addrspace(4) %asc1 to ptr
  store i16 8, ptr %asc2, align 8
  store i32 8, ptr addrspace(4) %asc1, align 8
  store i32 8, ptr %asc3, align 8
  store i32 8, ptr %asc4, align 8
  ret void
}
