; RUN: opt -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-outofboundsok=true -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-outofboundsok=true -disable-output %s 2>&1 | FileCheck %s

; This test verifies that constantexpr bitcasts involving global variables
; are properly analyzed.

%test01.union.u = type { double }
%test01.struct.anon = type { i32, i32 }

@g_u01 = dso_local local_unnamed_addr global %test01.union.u zeroinitializer

define i32 @test01() {
  %ret = load i32, i32* getelementptr (%test01.struct.anon,
                                       %test01.struct.anon*
                                         bitcast (%test01.union.u* @g_u01 to
                                                  %test01.struct.anon*),
                                       i64 0, i32 1)
  ret i32 %ret
}

; CHECK-LABEL: LLVMType: %test01.struct.anon = type { i32, i32 }
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:
; CHECK-NOT: Read
; CHECK: 1)Field LLVM Type: i32
; CHECK-NEXT: Field info: Read
; CHECK: Safety data: Bad casting | Ambiguous GEP

; CHECK-LABEL: LLVMType: %test01.union.u = type { double }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Global instance


%test02.union.u = type { double }
%test02.struct.anon = type { i32, i32 }
%test02.struct.outer = type { i32, i32, %test02.union.u }

@g_s02 = dso_local local_unnamed_addr global %test02.struct.outer
             zeroinitializer

define i32 @test02() {
  %ret = load i32, i32* getelementptr
                            (%test02.struct.anon, %test02.struct.anon*
                             bitcast (%test02.union.u*
                                          getelementptr inbounds
                                                 (%test02.struct.outer,
                                                  %test02.struct.outer* @g_s02,
                                                  i64 0, i32 2)
                                 to %test02.struct.anon*),
                             i64 0, i32 1)
  ret i32 %ret
}

; CHECK-LABEL: LLVMType: %test02.struct.anon = type { i32, i32 }
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:
; CHECK-NOT: Read
; CHECK: 1)Field LLVM Type: i32
; CHECK-NEXT: Field info: Read
; CHECK: Safety data: Bad casting | Ambiguous GEP

; CHECK-LABEL: LLVMType: %test02.union.u = type { double }
; CHECK: Safety data: Bad casting | Ambiguous GEP | Global instance | Nested structure
