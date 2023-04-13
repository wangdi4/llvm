; ASI can convert the addrspacecast i16 into a plain bitcast.
; This test checks that if ASC has the same element type as NewV, ASC is
; replaced with NewV instead of new bitcast from i16 pointer.

; RUN: opt -opaque-pointers=0 -passes="infer-address-spaces" -S -override-flat-addr-space=4 -o - %s | FileCheck %s
; CHECK: bitcast i32* %gep0 to i16*
; CHECK-NOT: bitcast

define void @asi_to_bitcast(i32 addrspace(4)* %ptr) {
  %asc0 = addrspacecast i32 addrspace(4)* %ptr to i32*
  %gep0 = getelementptr i32, i32* %asc0, i64 9
  %asc1 = addrspacecast i32* %gep0 to i32 addrspace(4)*
  %asc3 = addrspacecast i32 addrspace(4)* %asc1 to i32*
  %asc2 = addrspacecast i32 addrspace(4)* %asc1 to i16*
  %asc4 = addrspacecast i32 addrspace(4)* %asc1 to i32*
  store i16 8, i16* %asc2, align 8
  store i32 8, i32 addrspace(4)* %asc1, align 8
  store i32 8, i32 * %asc3, align 8
  store i32 8, i32 * %asc4, align 8
  ret void
}
