; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtrans-elim-ro-field-access -S 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes=dtrans-elim-ro-field-access -S 2>&1 | FileCheck %s

; This test verifies the DTrans eliminate read-only field access pass.

%struct.lzma_allocator = type { i8* (i8*, i64, i64)*, void (i8*, i8*)*, i8* }

define i8* @lzma_alloc(i64 %size, %struct.lzma_allocator* %allocator) {
entry:
  %cmp = icmp eq i64 %size, 0
  %spec.select = select i1 %cmp, i64 1, i64 %size
  %cmp1 = icmp eq %struct.lzma_allocator* %allocator, null
  br i1 %cmp1, label %if.else, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %alloc = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %allocator, i64 0, i32 0
  %0 = load i8* (i8*, i64, i64)*, i8* (i8*, i64, i64)** %alloc, align 8
  %cmp2 = icmp eq i8* (i8*, i64, i64)* %0, null
  br i1 %cmp2, label %if.else, label %if.then3

if.then3:                                         ; preds = %land.lhs.true
  %opaque = getelementptr inbounds %struct.lzma_allocator, %struct.lzma_allocator* %allocator, i64 0, i32 2
  %1 = load i8*, i8** %opaque, align 8
  %call = call i8* %0(i8* %1, i64 1, i64 %spec.select)
  br label %if.end6

if.else:                                          ; preds = %land.lhs.true, %entry
  %call5 = call noalias i8* @malloc(i64 %spec.select)
  br label %if.end6

if.end6:                                          ; preds = %if.else, %if.then3
  %ptr.0 = phi i8* [ %call, %if.then3 ], [ %call5, %if.else ]
  ret i8* %ptr.0
}

declare i8* @malloc(i64)


; CHECK-LABEL: @lzma_alloc(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i64 [[SIZE:%.*]], 0
; CHECK-NEXT:    [[SPEC_SELECT:%.*]] = select i1 [[CMP]], i64 1, i64 [[SIZE]]
; CHECK-NEXT:    br label [[IF_ELSE:%.*]]
; CHECK:       if.else:
; CHECK-NEXT:    [[CALL5:%.*]] = call noalias i8* @malloc(i64 [[SPEC_SELECT]])
; CHECK-NEXT:    br label [[IF_END6:%.*]]
; CHECK:       if.end6:
; CHECK-NEXT:    ret i8* [[CALL5]]
;
