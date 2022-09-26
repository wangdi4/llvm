; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -internalize -S -dtrans-optbasetest -dtrans-merge-padded-structs=true -dtrans-optbasetest-typelist=struct.test.a 2>&1 | FileCheck %s 
; RUN: opt  < %s -whole-program-assume -S -passes='internalize,dtrans-optbasetest' -dtrans-merge-padded-structs=true -dtrans-optbasetest-typelist=struct.test.a 2>&1 | FileCheck %s 

; This test verifies that type remapping didn't happen because there is a
; base type and a padded type.

; NOTE: This test case might need to be updated in the future. We can enable
; type remapping for this case, but it is not urgent at the moment.

%"struct.test.array" = type { [4 x i32] }
%struct.test.a = type <{ %"struct.test.array" , i32, [4 x i8] }>
%struct.test.a.base = type <{ %"struct.test.array" , i32 }>
%struct.test.b = type { %struct.test.a.base, i64 }

define i32 @test() {
  %bptr = alloca %struct.test.b
  %bcast = bitcast %struct.test.b* %bptr to %struct.test.a*
  %agep = getelementptr %struct.test.a, %struct.test.a* %bcast, i64 0, i32 0
  %boostgep = getelementptr %"struct.test.array", %"struct.test.array"* %agep, i64 0, i32 0
  %arrgep = getelementptr [4 x i32], [4 x i32]* %boostgep, i64 0, i32 0
  %ret = load i32, i32* %arrgep
  ret i32 %ret
}

; Check that %struct.test.b still uses the base class
; CHECK: %struct.test.b = type { %struct.test.a.base, i64 }

; Check that the types weren't combined
; CHECK: %struct.test.a.base = type <{ %struct.test.array, i32 }>
; CHECK: %struct.test.a = type <{ %struct.test.array, i32, [4 x i8] }>

; Check that the type struct.test.a wasn't changed
; CHECK-NOT: %__DTT_struct.test.a = type <{ %struct.test.array, i32, [4 x i8] }>
