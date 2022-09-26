; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtrans-elim-ro-field-access -debug-only=elim-ro-field-access -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes=dtrans-elim-ro-field-access -debug-only=elim-ro-field-access -disable-output 2>&1 | FileCheck %s

; This test verifies the DTrans eliminate read-only field access pass.
; First IF basic block doesn't meet the criteria.

%struct.lzma_allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }

define i8* @lzma_alloc(i64 %size, %struct.lzma_allocator* %allocator) {
entry:
  %cmp = icmp eq i64 %size, 0
  %spec.select = select i1 %cmp, i64 1, i64 %size
  %cmp1 = icmp ne %struct.lzma_allocator* %allocator, null
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
  ret i8* %ptr
}

declare i8* @malloc(i64)

; CHECK-LABEL: DTRANS-ELIM-RO-FIELD-ACCESS: Analysing lzma_alloc
; CHECK-NOT: DTRANS-ELIM-RO-FIELD-ACCESS: First IF BB is proven
