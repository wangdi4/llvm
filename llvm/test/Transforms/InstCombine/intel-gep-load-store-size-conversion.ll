; RUN: opt < %s -passes=instcombine -S | FileCheck %s

; Verify that %gep's src element type is changed from i8 to i32 which is also
; the load/store type.

; CHECK: %gep = getelementptr inbounds i32, ptr %p, i64 %s

define void @foo(ptr %p, i64 %s) {
  %idx = shl nuw nsw i64 %s, 2
  %gep = getelementptr inbounds i8, ptr %p, i64 %idx
  %ld = load i32, ptr %gep
  %add = add i32 %ld, 5
  store i32 %add, ptr %gep
  ret void
}

; Verify that GEP's source element type is not changed into ptr.

; CHECK:  %gep = getelementptr inbounds double, ptr %call, i64 400000

define internal fastcc void @bar(ptr nocapture noundef writeonly %ptr, ptr %call)  {
  %gep = getelementptr inbounds double, ptr %call, i64 400000
  store ptr %gep, ptr %ptr, align 8
  ret void
}

