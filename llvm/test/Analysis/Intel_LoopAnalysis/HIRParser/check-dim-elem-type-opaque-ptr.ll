; RUN: opt < %s -opaque-pointers -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework  | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s
; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that parser works for the following llvm input. Notice the type difference between %class.FastState and %class.KoState

; CHECK: + DO i1 = 0, -2, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, (-1236 * i1 + -1236 * %sub.ptr.div.i.i + -1 * ptrtoint.ptr.i64(%base) + ptrtoint.ptr.i64(%this) + -1028)/u8, 1   <DO_LOOP>  <MAX_TC_EST = 24>
; CHECK: |   |   (i32*)(%cur)[0].9.0[i2] = 0;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %cur = &((%base)[i1 + %sub.ptr.div.i.i + 1]);
; CHECK: + END LOOP

;RUN: opt < %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-cg -force-hir-cg -S | FileCheck -check-prefix=CHECK-CG %s
;RUN: opt %s -passes="hir-ssa-deconstruction,hir-cg" -hir-cost-model-throttling=0 -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG
;RUN: opt < %s -opaque-pointers -enable-new-pm=0 -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-cg -force-hir-cg -S | FileCheck -check-prefix=CHECK-CG %s
;RUN: opt %s -opaque-pointers -passes="hir-ssa-deconstruction,hir-cg" -hir-cost-model-throttling=0 -force-hir-cg -S 2>&1 | FileCheck %s -check-prefix=CHECK-CG

; CHECK-CG: [[TEMP:%.*]] = alloca ptr, align 8
; CHECK-CG: [[TEMPLOAD:%.*]] = load ptr, ptr [[TEMP]], align 8
;
; CHECK-CG: [[STOREID:%.*]] = getelementptr inbounds %class.FastState, ptr [[TEMPLOAD]], i64 0, i32 9, i32 0, i64 %9
; CHECK-CG: store i32 0, ptr [[STOREID]], align 4
;
; CHECK-CG: [[PTRSTOREID:%.*]]  = getelementptr inbounds %class.KoState, ptr %base, i64 %14
; CHECK-CG: store ptr [[PTRSTOREID]], ptr [[TEMP]], align 8

%class.KoState = type { %class.FastState.base, i64, i64 }
%class.FastState.base = type <{ %class.FullBoard, float, i32, i32, i32, i32, i32, i32, %"class.boost::array.5", %"class.boost::array.6" }>
%class.FullBoard = type { %class.FastBoard.base, i64, i64 }
%class.FastBoard.base = type <{ %"class.boost::array", i32 }>
%"class.boost::array" = type { [441 x i16] }
%"class.boost::array.5" = type { [24 x i32] }
%"class.boost::array.6" = type { [24 x %"struct.std::pair"] }
%"struct.std::pair" = type { i32, i32 }
%class.Time = type { i32 }
%class.FastState = type <{ %class.FullBoard, float, i32, i32, i32, i32, i32, i32, %"class.boost::array.5", %"class.boost::array.6", [4 x i8] }>

define void @_ZN9GameState9play_moveEii(ptr %this, i64 %sub.ptr.div.i.i, ptr %base) #0 {

exit2: ; preds = %exit.i
  %add.ptr = getelementptr inbounds %class.KoState, ptr %base, i64 %sub.ptr.div.i.i
  br label %for.body

for.body:                             ; preds = %for.inc, %exit2
  %addr = phi i64 [ %dec, %for.inc ], [ 1, %exit2 ]
  %cur = phi ptr [ %incdec.ptr.i.i.i.i29.i, %for.inc ], [ %add.ptr, %exit2 ]
  %array.begin = getelementptr inbounds %class.FastState, ptr %cur, i64 0, i32 9, i32 0, i64 0
  br label %arrayctor.loop

arrayctor.loop:                 ; preds = %arrayctor.loop, %for.body
  %arrayctor.cur = phi ptr [ %array.begin, %for.body ], [ %arrayctor.next, %arrayctor.loop ]
  store i32 0, ptr %arrayctor.cur, align 4
  %arrayctor.next = getelementptr inbounds %"struct.std::pair", ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.cur, %this
  br i1 %arrayctor.done, label %for.inc, label %arrayctor.loop

for.inc:                                ; preds = %arrayctor.loop
  %dec = add i64 %addr, 1
  %incdec.ptr.i.i.i.i29.i = getelementptr inbounds %class.KoState, ptr %cur, i64 1
  %cmp.not = icmp eq i64 %dec, 0
  br i1 %cmp.not, label %try.cont.i.i, label %for.body

try.cont.i.i:                                     ; preds = %for.inc
  ret void
}

attributes #0 = { "loopopt-pipeline"="light" }
