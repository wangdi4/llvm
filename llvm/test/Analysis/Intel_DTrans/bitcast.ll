; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies the behavior of the DTransAnalysis of bitcast instructions.
; The test cases below present representative examples of bitcasts that
; may occur within a module. Some cases represent behavior that does not
; present any legality concerns for DTrans optimizations, and we check to be
; sure these aren't being incorrectly identified as bad cast. Other cases
; do present legality concerns for DTrans, and in these cases we check to be
; sure the bad casts are properly identified.

; Cast of allocated buffer to a struct pointer.
%struct.test01 = type { i32, i32 }
define void @test1() {
  %p = call noalias i8* @malloc(i64 8)
  %s1 = bitcast i8* %p to %struct.test01*
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Cast of arbitrary i8* to struct pointer.
%struct.test02.a = type { i32, i8 }
%struct.test02.b = type { i32, i32 }
@p.test2 = external global %struct.test02.a
define void @test2() {
  %s = bitcast i8* getelementptr( %struct.test02.a, %struct.test02.a* @p.test2,
                                  i64 0, i32 1) to %struct.test02.b*
  ret void
}

; FIXME: %struct.test02.a should also be marked as bad casting here.
;        That's being missed because the analysis doesn't look for the
;        uses of globals and therefore doesn't see the GEP operator.
; CHECK: LLVMType: %struct.test02.a = type { i32, i8 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test02.b = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of non-alloc pointer value
%struct.test03 = type { i32, i32 }
define void @test3(i8* %p) {
  %s = bitcast i8* %p to %struct.test03*
  ret void
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of pointer-to-pointer-to-struct cast as a pointer-sized integer.
%struct.test04.a = type { i32, i32 }
%struct.test04.b = type { i32, i32, %struct.test04.a* }
define void @test4( %struct.test04.a** %ppa, %struct.test04.b* %pb ) {
  %a.as.pi = bitcast %struct.test04.a** %ppa to i64*
  %pb.a = getelementptr %struct.test04.b, %struct.test04.b* %pb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test04.a** %pb.a to i64*
  ret void
}

; FIXME: These should be "No issues found" but are currently "Unhandled use"
;        because the GEP is not yet handled.
; CHECK: LLVMType: %struct.test04.a = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting
; CHECK: LLVMType: %struct.test04.b = type { i32, i32, %struct.test04.a* }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Direct casting to related type.
%struct.test05.a = type { i32, i32 }
%struct.test05.b = type { i32, i32, i32 }
define void @test5( %struct.test05.a* %pa ) {
  %pb = bitcast %struct.test05.a* %pa to %struct.test05.b*
  ret void
}

; CHECK: LLVMType: %struct.test05.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test05.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Pointer-to-pointer casting to related type.
%struct.test06.a = type { i32, i32 }
%struct.test06.b = type { i32, i32, i32 }
define void @test6( %struct.test06.a** %ppa ) {
  %ppb = bitcast %struct.test06.a** %ppa to %struct.test06.b**
  ret void
}

; CHECK: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test06.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Bad pointer-to-pointer casting.
%struct.test07 = type { i32, i32 }
define void @test7( %struct.test07** %pps ) {
  %ps = bitcast %struct.test07** %pps to %struct.test07*
  ret void
}

; CHECK: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Safe element zero access through bitcast.
%struct.test08.a = type { i32, i32 }
%struct.test08.b = type { %struct.test08.a, i32, i32 }
define void @test8( %struct.test08.b* %pb ) {
  %pb.a = bitcast %struct.test08.b* %pb to %struct.test08.a*
  ret void
}

; FIXME: These should report "no issues found" but they currently report
;        "Unhandled use" because element-zero access via bitcast isn't
;        analyzed. The checks here verify that the bitcast was accepted.
; CHECK: LLVMType: %struct.test08.a = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting
; CHECK: LLVMType: %struct.test08.b = type { %struct.test08.a, i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Unsafe element zero access through bitcast.
%struct.test09.a = type { i32, i32 }
%struct.test09.b = type { %struct.test09.a, i32, i32 }
%struct.test09.c = type { i32, i32, i32, i32 }
define void @test9( %struct.test09.b* %pb ) {
  %pb.c = bitcast %struct.test09.b* %pb to %struct.test09.c*
  ret void
}

; FIXME: Should %struct.test09.a als be flagged with bad casting here?
; CHECK: LLVMType: %struct.test09.b = type { %struct.test09.a, i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test09.c = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Safe array element zero access through bitcast.
%struct.test10 = type { i32, i32 }
define void @test10( [16 x %struct.test10]* %parr ) {
  %pa0 = bitcast [16 x %struct.test10]* %parr to %struct.test10*
  ret void
}

; FIXME: These should report "no issues found" but they currently report
;        "Unhandled use" because element-zero access via bitcast isn't
;        analyzed. The checks here verify that the bitcast was accepted.
; CHECK: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting
; (checked below): LLVMType: [16 x %struct.test10]

; Unsafe array element zero access through bitcast.
%struct.test11.a = type { i32, i32 }
%struct.test11.b = type { i32, i32, i32, i32 }
define void @test11( [16 x %struct.test11.a]* %parr ) {
  %pb0 = bitcast [16 x %struct.test11.a]* %parr to %struct.test11.b*
  ret void
}

; FIXME: Should %struct.test11.a als be flagged with bad casting here?
; CHECK: LLVMType: %struct.test11.b = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad casting
; (checked below): LLVMType: [16 x %struct.test11.a]

; Cast of global value through an intermediate i8*
%struct.test12 = type { i32, i32 }
@p.test12 = external global %struct.test12
define void @test12() {
  %t = bitcast %struct.test12* @p.test12 to i8*
  %t2 = bitcast i8* %t to %struct.test12*
  ret void
}

; CHECK: LLVMType: %struct.test12 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of global value through an intermediate i8*
%struct.test13.a = type { i32, i32 }
%struct.test13.b = type { i32, i32, i32 }
@p.test13 = external global %struct.test13.a
define void @test13() {
  %t = bitcast %struct.test13.a* @p.test13 to i8*
  %t2 = bitcast i8* %t to %struct.test13.b*
  ret void
}

; CHECK: LLVMType: %struct.test13.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test13.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of argument through an intermediate i8*
%struct.test14 = type { i32, i32 }
define void @test14(%struct.test14* %p) {
  %t = bitcast %struct.test14* %p to i8*
  %t2 = bitcast i8* %t to %struct.test14*
  ret void
}

; CHECK: LLVMType: %struct.test14 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of argument through an intermediate i8*
%struct.test15.a = type { i32, i32 }
%struct.test15.b = type { i32, i32, i32 }
define void @test15(%struct.test15.a* %p) {
  %t = bitcast %struct.test15.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test15.b*
  ret void
}

; CHECK: LLVMType: %struct.test15.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test15.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of stack pointer through an intermediate i8*
%struct.test16 = type { i32, i32 }
define void @test16() {
  %p = alloca %struct.test16
  %t = bitcast %struct.test16* %p to i8*
  %t2 = bitcast i8* %t to %struct.test16*
  ret void
}

; CHECK: LLVMType: %struct.test16 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of stack pointer through an intermediate i8*
%struct.test17.a = type { i32, i32 }
%struct.test17.b = type { i32, i32, i32 }
define void @test17() {
  %p = alloca %struct.test17.a
  %t = bitcast %struct.test17.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test17.b*
  ret void
}

; CHECK: LLVMType: %struct.test17.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test17.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of loaded pointer through an intermediate i8*
%struct.test18 = type { i32, i32 }
define void @test18(%struct.test18** %mem) {
  %p = load %struct.test18*, %struct.test18** %mem
  %t = bitcast %struct.test18* %p to i8*
  %t2 = bitcast i8* %t to %struct.test18*
  ret void
}

; CHECK: LLVMType: %struct.test18 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of loaded pointer through an intermediate i8*
%struct.test19.a = type { i32, i32 }
%struct.test19.b = type { i32, i32, i32 }
define void @test19(%struct.test19.a** %mem) {
  %p = load %struct.test19.a*, %struct.test19.a** %mem
  %t = bitcast %struct.test19.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test19.b*
  ret void
}

; CHECK: LLVMType: %struct.test19.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test19.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of returned pointer through an intermediate i8*
%struct.test20 = type { i32, i32 }
declare %struct.test20* @test20Helper()
define void @test20() {
  %p = call %struct.test20* @test20Helper()
  %t = bitcast %struct.test20* %p to i8*
  %t2 = bitcast i8* %t to %struct.test20*
  ret void
}

; CHECK: LLVMType: %struct.test20 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of returned pointer through an intermediate i8*
%struct.test21.a = type { i32, i32 }
%struct.test21.b = type { i32, i32, i32 }
declare %struct.test21.a* @test21Helper()
define void @test21() {
  %p = call %struct.test21.a* @test21Helper()
  %t = bitcast %struct.test21.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test21.b*
  ret void
}

; CHECK: LLVMType: %struct.test21.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test21.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of GEP-derived pointer through an intermediate i8*
%struct.test22.a = type { i32, i32 }
%struct.test22.b = type { i32, %struct.test22.a, i32 }
define void @test22(%struct.test22.b* %pb) {
  %pa = getelementptr %struct.test22.b, %struct.test22.b* %pb, i64 0, i32 1
  %t = bitcast %struct.test22.a* %pa to i8*
  %t2 = bitcast i8* %t to %struct.test22.a*
  ret void
}

; CHECK: LLVMType: %struct.test22.a = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of GEP-derived pointer through an intermediate i8*
%struct.test23.a = type { i32, i32 }
%struct.test23.b = type { i32, %struct.test23.a, i32 }
define void @test23(%struct.test23.b* %pb) {
  %pa = getelementptr %struct.test23.b, %struct.test23.b* %pb, i64 0, i32 1
  %t = bitcast %struct.test23.a* %pa to i8*
  %t2 = bitcast i8* %t to %struct.test23.b*
  ret void
}

; CHECK: LLVMType: %struct.test23.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test23.b = type { i32, %struct.test23.a, i32 }
; CHECK: Safety data: Bad casting

; Cast of inttoptr value through an intermediate i8*
; Note: inttoptr will typically be used when a pointer is loaded
;       as an i64 value from a structure field. In most other cases
;       it will be unsafe. Here 'undef' is used to ignore problems
;       related to where the value came from.
%struct.test24 = type { i32, i32 }
define void @test24() {
  %p = inttoptr i64 undef to %struct.test24*
  %t = bitcast %struct.test24* %p to i8*
  %t2 = bitcast i8* %t to %struct.test24*
  ret void
}

; CHECK: LLVMType: %struct.test24 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Bad cast of inttoptr value through an intermediate i8*
%struct.test25.a = type { i32, i32 }
%struct.test25.b = type { i32, i32, i32 }
define void @test25() {
  %p = inttoptr i64 undef to %struct.test25.a*
  %t = bitcast %struct.test25.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test25.b*
  ret void
}

; CHECK: LLVMType: %struct.test25.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test25.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Follow safe cast through select instruction
%struct.test26 = type { i32, i32 }
define void @test26(%struct.test26* %p1, %struct.test26* %p2) {
  %t1 = bitcast %struct.test26* %p1 to i8*
  %t2 = bitcast %struct.test26* %p2 to i8*
  %t3 = select i1 undef, i8* %t1, i8* %t2
  %t4 = bitcast i8* %t3 to %struct.test26*
  ret void
}

; CHECK: LLVMType: %struct.test26 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Follow unsafe cast through select instruction
%struct.test27.a = type { i32, i32 }
%struct.test27.b = type { i32, i32, i32 }
define void @test27(%struct.test27.a* %pa, %struct.test27.b* %pb) {
  %t1 = bitcast %struct.test27.a* %pa to i8*
  %t2 = bitcast %struct.test27.b* %pb to i8*
  %t3 = select i1 undef, i8* %t1, i8* %t2
  %t4 = bitcast i8* %t3 to %struct.test27.a*
  ret void
}

; CHECK: LLVMType: %struct.test27.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test27.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Follow safe cast through PHI node
%struct.test28 = type { i32, i32 }
define void @test28(%struct.test28* %p1, %struct.test28* %p2) {
  br i1 undef, label %true, label %false

true:
  %t1 = bitcast %struct.test28* %p1 to i8*
  br label %end

false:
  %t2 = bitcast %struct.test28* %p2 to i8*
  br label %end

end:
  %t3 = phi i8* [%t1, %true], [%t2, %false]
  %t4 = bitcast i8* %t3 to %struct.test28*
  ret void
}

; CHECK: LLVMType: %struct.test28 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Follow unsafe cast through PHI node
%struct.test29.a = type { i32, i32 }
%struct.test29.b = type { i32, i32, i32 }
define void @test29(%struct.test29.a* %pa, %struct.test29.b* %pb) {
  br i1 undef, label %true, label %false

true:
  %t1 = bitcast %struct.test29.a* %pa to i8*
  br label %end

false:
  %t2 = bitcast %struct.test29.b* %pb to i8*
  br label %end

end:
  %t3 = phi i8* [%t1, %true], [%t2, %false]
  %t4 = bitcast i8* %t3 to %struct.test29.a*
  ret void
}

; CHECK: LLVMType: %struct.test29.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test29.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Follow safe cast through PHI node with loop.
%struct.test30 = type { i32, i32 }
define void @test30(%struct.test30* %p1, %struct.test30* %p2) {
entry:
  %t1 = bitcast %struct.test30* %p1 to i8*
  br label %loop

loop:
  %t2 = phi i8* [%t1, %entry], [%t4, %back]
  br i1 undef, label %end, label %back

back:
  %t3 = bitcast %struct.test30* %p2 to i8*
  %t4 = select i1 undef, i8* %t2, i8* %t3
  br label %loop

end:
  %t5 = bitcast i8* %t2 to %struct.test30*
  ret void
}

; CHECK: LLVMType: %struct.test30 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Follow unsafe cast through PHI node with loop.
%struct.test31.a = type { i32, i32 }
%struct.test31.b = type { i32, i32, i32 }
define void @test31(%struct.test31.a* %pa, %struct.test31.b* %pb) {
entry:
  %t1 = bitcast %struct.test31.a* %pa to i8*
  br label %loop

loop:
  %t2 = phi i8* [%t1, %entry], [%t4, %back]
  br i1 undef, label %end, label %back

back:
  %t3 = bitcast %struct.test31.b* %pb to i8*
  %t4 = select i1 undef, i8* %t2, i8* %t3
  br label %loop

end:
  %t5 = bitcast i8* %t2 to %struct.test31.a*
  ret void
}

; CHECK: LLVMType: %struct.test31.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test31.b = type { i32, i32, i32 }

; Safe element zero access of an aliased value through bitcast.
%struct.test32 = type { i32, i32 }
define void @test32( %struct.test32* %p ) {
  %tmp = bitcast %struct.test32* %p to i8*
  %ps.a = bitcast i8* %tmp to i32*
  ret void
}

; FIXME: This should report "no issues found" but they currently report
;        "Unhandled use" because element-zero access via bitcast isn't
;        analyzed. The checks here verify that the bitcast was accepted.
; CHECK: LLVMType: %struct.test32 = type { i32, i32 }
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; Unsafe element zero access of an aliased value through bitcast.
%struct.test33 = type { i32, i32 }
define void @test33( %struct.test33* %p ) {
  %tmp = bitcast %struct.test33* %p to i8*
  %ps.a = bitcast i8* %tmp to i64*
  ret void
}

; CHECK: LLVMType: %struct.test33 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Array types get printed last so theese checks aren't with their IR.

; CHECK: LLVMType: [16 x %struct.test10]
; CHECK: Safety data:
; CHECK-NOT: Bad casting

; CHECK: LLVMType: [16 x %struct.test11.a]
; CHECK: Safety data: Bad casting

declare noalias i8* @malloc(i64)
