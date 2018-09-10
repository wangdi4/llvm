; REQUIRES: asserts
; RUN: opt  < %s -whole-program-assume -dtrans-elim-ro-field-access -debug-only=elim-ro-field-access -disable-output 2>&1 | FileCheck --check-prefix=DBG %s
; RUN: opt  < %s -whole-program-assume -dtrans-elim-ro-field-access -dtrans-print-types -disable-output 2>&1 | FileCheck --check-prefix=SAFETY %s
; RUN: opt  < %s -whole-program-assume -passes=dtrans-elim-ro-field-access -debug-only=elim-ro-field-access -disable-output 2>&1 | FileCheck --check-prefix=DBG %s
; RUN: opt  < %s -whole-program-assume -passes=dtrans-elim-ro-field-access -dtrans-print-types -disable-output 2>&1 | FileCheck --check-prefix=SAFETY %s

; This test verifies the dtrans eliminate read-only field access pass.
; lzma_allocator type has safety check violations due to bad casting.

%struct.lzma_allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }
%struct.str = type { i32, i32 }

define i8* @lzma_alloc(i64 %size, %struct.lzma_allocator* %allocator) {
entry:
  %cmp = icmp eq i64 %size, 0
  %spec.select = select i1 %cmp, i64 1, i64 %size
  %cmp1 = icmp eq %struct.lzma_allocator* %allocator, null
  br i1 %cmp1, label %if.else, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %alloc = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %allocator, i64 0, i32 0
  %func = load i8* (i8*, i64, i64)*, i8* (i8*, i64, i64)** %alloc, align 8
  %cmp2 = icmp eq i8* (i8*, i64, i64)* %func, null
  br i1 %cmp2, label %if.else, label %if.then3

if.then3:                                         ; preds = %land.lhs.true
  %opaque = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %allocator, i64 0, i32 2
  %arg = load i8*, i8** %opaque, align 8
  %call_if = call i8* %func(i8* %arg, i64 1, i64 %spec.select)
  br label %if.end6

if.else:                                          ; preds = %land.lhs.true, %entry
  %call_else = call noalias i8* @malloc(i64 %spec.select)
  br label %if.end6

if.end6:                                          ; preds = %if.else, %if.then3
  %ptr = phi i8* [ %call_if, %if.then3 ], [ %call_else, %if.else ]
  %tmp = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %allocator, i64 0, i32 2
  %ptr2 = bitcast i8** %tmp to %struct.str*
  ret i8* %ptr
}

declare i8* @malloc(i64)

; On Windows the stdout and stderr streams don't mix cleanly, so we check them
; separately.
; DBG: DTRANS-ELIM-RO-FIELD-ACCESS: Safety check failed - skipping.
; SAFETY: Safety data: Bad casting
; SAFETY: Safety data: Bad casting
