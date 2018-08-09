; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-outofboundsok=true -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-outofboundsok=true -disable-output 2>&1 | FileCheck %s

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

; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Cast of arbitrary i8* to struct pointer.
%struct.test02.a = type { i32, i8 }
%struct.test02.b = type { i32, i32 }
@p.test2 = internal unnamed_addr global %struct.test02.a zeroinitializer
define void @test2() {
  %s = bitcast i8* getelementptr( %struct.test02.a, %struct.test02.a* @p.test2,
                                  i64 0, i32 1) to %struct.test02.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test02.a = type { i32, i8 }
; CHECK: Safety data: Bad casting | Global instance
; CHECK: LLVMType: %struct.test02.b = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of non-alloc pointer value
%struct.test03 = type { i32, i32 }
define void @test3() {
  %s = bitcast i8* undef to %struct.test03*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test03 = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test04.a = type { i32, i32 }
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct.test04.b = type { i32, i32, %struct.test04.a* }
; CHECK: Safety data: No issues found

; Direct casting to related type.
%struct.test05.a = type { i32, i32 }
%struct.test05.b = type { i32, i32, i32 }
define void @test5( %struct.test05.a* %pa ) {
  %pb = bitcast %struct.test05.a* %pa to %struct.test05.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test05.a = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test06.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test06.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Bad pointer-to-pointer casting.
%struct.test07 = type { i32, i32 }
define void @test7( %struct.test07** %pps ) {
  %ps = bitcast %struct.test07** %pps to %struct.test07*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Safe element zero access through bitcast.
%struct.test08.a = type { i32, i32 }
%struct.test08.b = type { %struct.test08.a, i32, i32 }
define void @test8( %struct.test08.b* %pb ) {
  %pb.a = bitcast %struct.test08.b* %pb to %struct.test08.a*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test08.a = type { i32, i32 }
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.test08.b = type { %struct.test08.a, i32, i32 }
; CHECK: Safety data: Contains nested structure

; Unsafe element zero access through bitcast.
%struct.test09.a = type { i32, i32 }
%struct.test09.b = type { %struct.test09.a, i32, i32 }
%struct.test09.c = type { i32, i32, i32, i32 }
define void @test9( %struct.test09.b* %pb ) {
  %pb.c = bitcast %struct.test09.b* %pb to %struct.test09.c*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test09.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Nested structure
; CHECK: LLVMType: %struct.test09.b = type { %struct.test09.a, i32, i32 }
; CHECK: Safety data: Bad casting | Contains nested structure
; CHECK: LLVMType: %struct.test09.c = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Safe array element zero access through bitcast.
%struct.test10 = type { i32, i32 }
define void @test10( [16 x %struct.test10]* %parr ) {
  %pa0 = bitcast [16 x %struct.test10]* %parr to %struct.test10*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test10 = type { i32, i32 }
; CHECK: Safety data: No issues found
; (checked below): LLVMType: [16 x %struct.test10]

; Unsafe array element zero access through bitcast.
%struct.test11.a = type { i32, i32 }
%struct.test11.b = type { i32, i32, i32, i32 }
define void @test11( [16 x %struct.test11.a]* %parr ) {
  %pb0 = bitcast [16 x %struct.test11.a]* %parr to %struct.test11.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test11.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test11.b = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad casting
; (checked below): LLVMType: [16 x %struct.test11.a]

; Cast of global value through an intermediate i8*
%struct.test12 = type { i32, i32 }
@p.test12 = internal unnamed_addr global %struct.test12 zeroinitializer
define void @test12() {
  %t = bitcast %struct.test12* @p.test12 to i8*
  %t2 = bitcast i8* %t to %struct.test12*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test12 = type { i32, i32 }
; CHECK: Safety data: Global instance

; Bad cast of global value through an intermediate i8*
%struct.test13.a = type { i32, i32 }
%struct.test13.b = type { i32, i32, i32 }
@p.test13 = internal unnamed_addr global %struct.test13.a zeroinitializer
define void @test13() {
  %t = bitcast %struct.test13.a* @p.test13 to i8*
  %t2 = bitcast i8* %t to %struct.test13.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test13.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Global instance
; CHECK: LLVMType: %struct.test13.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of argument through an intermediate i8*
%struct.test14 = type { i32, i32 }
define void @test14(%struct.test14* %p) {
  %t = bitcast %struct.test14* %p to i8*
  %t2 = bitcast i8* %t to %struct.test14*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test14 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Bad cast of argument through an intermediate i8*
%struct.test15.a = type { i32, i32 }
%struct.test15.b = type { i32, i32, i32 }
define void @test15(%struct.test15.a* %p) {
  %t = bitcast %struct.test15.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test15.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test15.a = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test16 = type { i32, i32 }
; CHECK: Safety data: Local instance

; Bad cast of stack pointer through an intermediate i8*
%struct.test17.a = type { i32, i32 }
%struct.test17.b = type { i32, i32, i32 }
define void @test17() {
  %p = alloca %struct.test17.a
  %t = bitcast %struct.test17.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test17.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test17.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Local instance
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

; CHECK-LABEL: LLVMType: %struct.test18 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Bad cast of loaded pointer through an intermediate i8*
%struct.test19.a = type { i32, i32 }
%struct.test19.b = type { i32, i32, i32 }
define void @test19(%struct.test19.a** %mem) {
  %p = load %struct.test19.a*, %struct.test19.a** %mem
  %t = bitcast %struct.test19.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test19.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test19.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test19.b = type { i32, i32, i32 }
; CHECK: Safety data: Bad casting

; Cast of returned pointer through an intermediate i8*
%struct.test20 = type { i32, i32 }
define %struct.test20* @test20Helper() { ret %struct.test20* undef }
define void @test20() {
  %p = call %struct.test20* @test20Helper()
  %t = bitcast %struct.test20* %p to i8*
  %t2 = bitcast i8* %t to %struct.test20*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test20 = type { i32, i32 }
; CHECK: Safety data: No issues found

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

; CHECK-LABEL: LLVMType: %struct.test21.a = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test22.a = type { i32, i32 }
; CHECK: Safety data: Nested structure
; CHECK: LLVMType: %struct.test22.b = type { i32, %struct.test22.a, i32 }
; CHECK: Safety data: Contains nested structure

; Bad cast of GEP-derived pointer through an intermediate i8*
%struct.test23.a = type { i32, i32 }
%struct.test23.b = type { i32, %struct.test23.a, i32 }
define void @test23(%struct.test23.b* %pb) {
  %pa = getelementptr %struct.test23.b, %struct.test23.b* %pb, i64 0, i32 1
  %t = bitcast %struct.test23.a* %pa to i8*
  %t2 = bitcast i8* %t to %struct.test23.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test23.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Nested structure
; CHECK: LLVMType: %struct.test23.b = type { i32, %struct.test23.a, i32 }
; CHECK: Safety data: Bad casting | Contains nested structure

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

; CHECK-LABEL: LLVMType: %struct.test24 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Bad cast of inttoptr value through an intermediate i8*
%struct.test25.a = type { i32, i32 }
%struct.test25.b = type { i32, i32, i32 }
define void @test25() {
  %p = inttoptr i64 undef to %struct.test25.a*
  %t = bitcast %struct.test25.a* %p to i8*
  %t2 = bitcast i8* %t to %struct.test25.b*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test25.a = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test26 = type { i32, i32 }
; CHECK: Safety data: No issues found

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

; CHECK-LABEL: LLVMType: %struct.test27.a = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test28 = type { i32, i32 }
; CHECK: Safety data: No issues found

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

; CHECK-LABEL: LLVMType: %struct.test29.a = type { i32, i32 }
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

; CHECK-LABEL: LLVMType: %struct.test30 = type { i32, i32 }
; CHECK: Safety data: No issues found

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

; CHECK-LABEL: LLVMType: %struct.test31.a = type { i32, i32 }
; CHECK: Safety data: Bad casting
; CHECK: LLVMType: %struct.test31.b = type { i32, i32, i32 }

; Safe element zero access of an aliased value through bitcast.
%struct.test32 = type { i32, i32 }
define void @test32( %struct.test32* %p ) {
  %tmp = bitcast %struct.test32* %p to i8*
  %ps.a = bitcast i8* %tmp to i32*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test32 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Unsafe element zero access of an aliased value through bitcast.
%struct.test33 = type { i32, i32 }
define void @test33( %struct.test33* %p ) {
  %tmp = bitcast %struct.test33* %p to i8*
  %ps.a = bitcast i8* %tmp to i64*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test33 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Multiple cast of allocated pointer-to-pointer-to-pointer.
; The second cast effectively re-interprets %struct.test34*** so that a
; an allocated %struct.test34** pointer can be stored to this location as
; an i8** value.
;
; A typical occurance of the pattern may look like this:
;
;   <BB1>:
;     %7 = call noalias i8* @malloc(i64 8)
;     %8 = bitcast i8* %7 to %struct.a***
;     ...
;   <BB2>:
;     ...
;     %49 = tail call noalias i8* @calloc(i64 %46, i64 8)
;     %50 = bitcast i8* %7 to i8**
;     store i8* %49, i8** %50, align 8, !tbaa !45
;     ...
;     %52 = bitcast i8* %49 to %struct.a**
;     ...
;
%struct.test34 = type { i32, i32 }
define void @test34() {
  %p1 = call noalias i8* @malloc(i64 8)
  %p2 = bitcast i8* %p1 to %struct.test34***
  %p3 = bitcast i8* %p1 to i8**
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test34 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Test that an element zero pointer cannot be cast back to the original type
; after a merge with an unknown pointer value.
;
; Currently, we do not allow element zero to be cast back to the original
; type under any circumstances because it is not an expected IR idiom. However,
; if we decide to allow it in the future, something would need to be done
; to handle this case.
%struct.test35 = type { i32, i32 }
define void @test35(%struct.test35* %p, i32* %p2) {
  %p.a = getelementptr %struct.test35, %struct.test35* %p, i64 0, i32 0
  %p3 = select i1 undef, i32* %p.a, i32* %p2
  %p4 = bitcast i32* %p.a to %struct.test35*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test35 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Test that an element zero pointer cannot be cast back to the original type
; after a merge with a pointer to another element.
%struct.test36 = type { i32, i32 }
define void @test36(%struct.test36* %p) {
  %p.a = getelementptr %struct.test36, %struct.test36* %p, i64 0, i32 0
  %p.b = getelementptr %struct.test36, %struct.test36* %p, i64 0, i32 1
  %p2 = select i1 undef, i32* %p.a, i32* %p.b
  %p3 = bitcast i32* %p2 to %struct.test36*
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test36 = type { i32, i32 }
; CHECK: Safety data: Bad casting

; Test the case of a cyclic dependency among PHI nodes.
; There was a bug where the alias set for one of the PHI nodes wasn't
; being fully resolved because it depended on another PHI node that
; was only partially analyzed when it was needed.
%struct.test37.a = type { i32, i32 }
%struct.test37.b = type { i16, i16, i16, i16 }

define void @test37(%struct.test37.a* %a_in, %struct.test37.b* %b_in) {
entry:
  %tmpA = bitcast %struct.test37.a* %a_in to i8*
  %tmpB = bitcast %struct.test37.b* %b_in to i8*
  br i1 undef, label %block_A, label %block_B

block_C:
  %c = phi i8* [%a, %block_A], [%b, %block_B]
  br label %merge

block_A:
  %a = phi i8* [%c, %merge], [%tmpA, %entry]
  br i1 undef, label %block_C, label %exit_A

block_B:
  %b = phi i8* [%c, %merge], [%tmpB, %entry]
  br i1 undef, label %block_C, label %exit_B

merge:
  br i1 undef, label %block_A, label %block_B

exit_A:
  %badA = bitcast i8* %a to %struct.test37.a*
  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}
  
; CHECK-LABEL: LLVMType: %struct.test37.a = type { i32, i32 }
; CHECK: Safety data: Bad casting | Unsafe pointer merge
; CHECK: LLVMType: %struct.test37.b = type { i16, i16, i16, i16 }
; CHECK: Safety data: Bad casting | Unsafe pointer merge

; Test a cyclic analysis with a dependent bitcast.
; This was an attempt to cause a false positive with an element-zero bitcast
; inside a cycle, but it is handled correctly.
%struct.test38 = type { i32, i32 }

define void @test38(%struct.test38* %p) {
entry:
  %tmpA = bitcast %struct.test38* %p to i8*
  %tmpB = call i8* @malloc(i64 16)
  br i1 undef, label %block_A, label %block_B

block_C:
  %c_a0 = phi i32* [%a0, %block_A], [%b_a0, %block_B]
  %c = phi i8* [%a, %block_A], [%b, %block_B]
  %elem0 = load i32, i32* %c_a0
  br label %merge

block_A:
  %a = phi i8* [%c, %merge], [%tmpA, %entry]
  %a0 = bitcast i8* %a to i32*
  br i1 undef, label %block_C, label %exit

block_B:
  %b = phi i8* [%c, %merge], [%tmpB, %entry]
  %b_a = bitcast i8* %b to %struct.test38*
  %b_a0 = bitcast i8* %b to i32*
  br i1 undef, label %block_C, label %exit

merge:
  br i1 undef, label %block_A, label %block_B

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test38 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Array types get printed last so theese checks aren't with their IR.

; CHECK: LLVMType: [16 x %struct.test10]
; CHECK: Safety data: No issues found

; CHECK: LLVMType: [16 x %struct.test11.a]
; CHECK: Safety data: Bad casting

declare noalias i8* @malloc(i64)
