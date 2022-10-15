; REQUIRES: asserts

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-fieldmodrefop-analysis>' -debug-only=dtrans-fmr-candidates-post -disable-output 2>&1 | FileCheck %s

; These tests are to check whether escapes are detected for field based
; Mod/Ref analysis.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Test simple allocations of memory for field members that do NOT escape.
%struct.test01 = type { i32, i32* }
define internal void @test01() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i32* null, i32** %f1
  br label %done

good1:
  call void @llvm.memset.p0i8.i64(i8* %ar1_mem, i8 0, i64 64, i1 false)

  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  store i32* %ar1_mem2, i32** %f1
  br label %done

done:
  ret void
}
; CHECK: ModRef candidate structures after analysis:
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test01


; Test allocating storage for a field that does escape. This should disqualify
; the pointer field.
%struct.test02 = type { i32, i32* }
@gAr02 = internal global i32* zeroinitializer, !intel_dtrans_type !2

define internal void @test02() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test02*

  %f0 = getelementptr %struct.test02, %struct.test02* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %f1 = getelementptr %struct.test02, %struct.test02* %st, i64 0, i32 1
  %f1_i8 = bitcast i32** %f1 to i8**

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i8* null, i8** %f1_i8
  br label %done

good1:
  store i8* %ar1_mem, i8** %f1_i8
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*

  ; Escape the allocated memory
  store i32* %ar1_mem2, i32** @gAr02
  br label %done

done:
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: bottom
; CHECK: End LLVMType: %struct.test02


; Test allocating storage with alignment adjustments that does not escape. This
; stores the pointer into the allocated region.
%struct.test03 = type { i32, i32* }
define internal void @test03() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test03*

  %f0 = getelementptr %struct.test03, %struct.test03* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %f1 = getelementptr %struct.test03, %struct.test03* %st, i64 0, i32 1
  %f1_i8 = bitcast i32** %f1 to i8**

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i8* null, i8** %f1_i8
  br label %done

good1:
  %pti = ptrtoint i8* %ar1_mem to i64
  %pti_offset = add i64 %pti, 71
  %pti_align = and i64 %pti_offset, -64
  %field_val = inttoptr i64 %pti_align to i8*
  %ar1_offset = inttoptr i64 %pti_align to i8**
  %ar1_internal = getelementptr inbounds i8*, i8** %ar1_offset, i64 -1
  store i8* %ar1_mem, i8** %ar1_internal, align 8

  store i8* %field_val, i8** %f1_i8
  br label %done

done:
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test03
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test03


; Test allocating storage with alignment adjustments that are not supported. This
; should cause the pointer field to be set to bottom.
%struct.test04 = type { i32, i32* }
define internal void @test04() {
  %st_mem = call i8* @malloc(i64 16)
  %st = bitcast i8* %st_mem to %struct.test04*

  %f0 = getelementptr %struct.test04, %struct.test04* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %f1 = getelementptr %struct.test04, %struct.test04* %st, i64 0, i32 1
  %f1_i8 = bitcast i32** %f1 to i8**

  %ar1_mem = call i8* @malloc(i64 64)
  %cmp1 = icmp eq i8* %ar1_mem, null
  br i1 %cmp1, label %no_mem1, label %good1

no_mem1:
  store i8* null, i8** %f1_i8
  br label %done

good1:
  %pti = ptrtoint i8* %ar1_mem to i64
  %pti_offset = add i64 %pti, 71
  ; Unsupported operation
  %pti_offset2 = or i64 %pti_offset, 7
  %pti_align = and i64 %pti_offset2, -64
  %field_val = inttoptr i64 %pti_align to i8*
  %ar1_offset = inttoptr i64 %pti_align to i8**
  %ar1_internal = getelementptr inbounds i8*, i8** %ar1_offset, i64 -1
  store i8* %ar1_mem, i8** %ar1_internal, align 8

  store i8* %field_val, i8** %f1_i8
  br label %done

done:
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: bottom
; CHECK: End LLVMType: %struct.test04


; Test with GEP of non-pointer field that escapes
%struct.test05 = type { i32, i32* }
define internal void @test05(%struct.test05* "intel_dtrans_func_index"="1" %in, i32** "intel_dtrans_func_index"="2" %out) !intel.dtrans.func.type !5 {
  %f0 = getelementptr %struct.test05, %struct.test05* %in, i64 0, i32 0
  store i32* %f0, i32** %out
  store i32 0, i32* %f0
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test05
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test05


; Test with GEP of pointer field that escapes
%struct.test06 = type { i32, i32* }
define internal void @test06(%struct.test06* "intel_dtrans_func_index"="1" %in, i32*** "intel_dtrans_func_index"="2" %out) !intel.dtrans.func.type !8 {
  %f1 = getelementptr %struct.test06, %struct.test06* %in, i64 0, i32 1
  store i32** %f1, i32*** %out
  store i32* null, i32** %f1
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test06
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: bottom
; CHECK: End LLVMType: %struct.test06


; Test with byte-flattened GEP forms that do NOT escape
%struct.test07 = type { i32, i32* }
define internal void @test07(%struct.test07* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !10 {
  %in_i8 = bitcast %struct.test07* %in to i8*
  %f0 = getelementptr i8, i8* %in_i8, i64 0
  %f0_p32 = bitcast i8* %f0 to i32*
  %v0 = load i32, i32* %f0_p32
  %v0i = add i32 %v0, 1
  store i32 %v0i, i32* %f0_p32

  %f1 = getelementptr i8, i8* %in_i8, i64 8
  %f1_pp = bitcast i8* %f1 to i8**
  store i8* null, i8** %f1_pp
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test07
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test07


; Test use of unsupported field types
%struct.test08 = type { [4 x i32] , %struct.test08**, i32 }
define internal void @test08(%struct.test08* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !14 {
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test08
; CHECK: 0)Field DTrans Type: [4 x i32]
; CHECK: RWState: bottom
; CHECK: 1)Field DTrans Type: %struct.test08**
; CHECK: RWState: bottom
; CHECK: 2)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test08


%struct.test09 = type { i32, [2 x %struct.test09b] }
%struct.test09b = type { i32, i32 }
define internal void @test09(%struct.test09* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !18 {
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test09
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: [2 x %struct.test09b]
; CHECK: RWState: bottom
; CHECK: End LLVMType: %struct.test09

; %struct.test09b should have been disqualified due to array within
; %struct.test09.
; CHECK-NOT: LLVMType: %struct.test09b


; Test that a field address used in a PHI node is supported.
%struct.test10 = type { i32, i32* }
define internal void @test10(%struct.test10* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !20 {
test10_entry:
  %f1 = getelementptr %struct.test10, %struct.test10* %in, i64 0, i32 1
  %ptr_val = load i32*, i32** %f1
  br label %test10_b1
test10_b1:
  %ptr_val1 = phi i32* [ %ptr_val, %test10_entry ]
  %ar_addr = getelementptr i32, i32* %ptr_val1, i64 1
  %val = load i32, i32* %ar_addr

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test10
; CHECK: 0)Field DTrans Type: i32
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test10


; Test that a field address used in a SelectInst is suported.
%struct.test11 = type { i32*, i32* }
define internal void @test11(%struct.test11* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !22 {
test11_entry:
  %f0 = getelementptr %struct.test11, %struct.test11* %in, i64 0, i32 0
  %f1 = getelementptr %struct.test11, %struct.test11* %in, i64 0, i32 1
  %ptr_val0 = load i32*, i32** %f0
  %ptr_val1 = load i32*, i32** %f1
  %ar_addr0 = getelementptr i32, i32* %ptr_val0, i64 1
  %val = load i32, i32* %ar_addr0
  %val1 = add i32 1, %val

  %ptr_valX = select i1 undef, i32* %ptr_val0, i32* %ptr_val1
  %ar_addrX = getelementptr i32, i32* %ptr_valX, i64 1
  store i32 %val1, i32* %ar_addrX
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test11
; CHECK: 0)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: 1)Field DTrans Type: i32*
; CHECK: RWState: computed
; CHECK: End LLVMType: %struct.test11


declare !intel.dtrans.func.type !24 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !25 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!4 = !{i32 0, i32 2}  ; i32**
!5 = distinct !{!3, !4}
!6 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!7 = !{i32 0, i32 3}  ; i32***
!8 = distinct !{!6, !7}
!9 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!10 = distinct !{!9}
!11 = !{!"A", i32 4, !1}  ; [4 x i32]
!12 = !{%struct.test08 zeroinitializer, i32 2}  ; %struct.test08**
!13 = !{%struct.test08 zeroinitializer, i32 1}  ; %struct.test08*
!14 = distinct !{!13}
!15 = !{!"A", i32 2, !16}  ; [2 x %struct.test09b]
!16 = !{%struct.test09b zeroinitializer, i32 0}  ; %struct.test09b
!17 = !{%struct.test09 zeroinitializer, i32 1}  ; %struct.test09*
!18 = distinct !{!17}
!19 = !{%struct.test10 zeroinitializer, i32 1}  ; %struct.test10*
!20 = distinct !{!19}
!21 = !{%struct.test11 zeroinitializer, i32 1}  ; %struct.test11*
!22 = distinct !{!21}
!23 = !{i8 0, i32 1}  ; i8*
!24 = distinct !{!23}
!25 = distinct !{!23}
!26 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!27 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!28 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!29 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!30 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!31 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!32 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!33 = !{!"S", %struct.test08 zeroinitializer, i32 3, !11, !12, !1} ; { [4 x i32] , %struct.test08**, i32 }
!34 = !{!"S", %struct.test09 zeroinitializer, i32 2, !1, !15} ; { i32, [2 x %struct.test09b] }
!35 = !{!"S", %struct.test09b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!36 = !{!"S", %struct.test10 zeroinitializer, i32 2, !1, !2} ; { i32, i32* }
!37 = !{!"S", %struct.test11 zeroinitializer, i32 2, !2, !2} ; { i32*, i32* }

!intel.dtrans.types = !{!26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37}
