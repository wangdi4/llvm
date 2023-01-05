; RUN: opt -passes="instcombine" < %s -S | FileCheck %s

; Verify that instcombine does not simplify %gep1 to this by eliminating zero
; struct offset with "pre_loopopt" attribute on the function-
; %gep1 = getelementptr inbounds %struct, ptr %p, i64 %iv

; CHECK-LABEL: foo
; CHECK: %gep1 = getelementptr inbounds %struct, ptr %p, i64 %iv, i32 0


%struct = type { i16, i16 }

define double @foo(ptr %p, i64 %iv) "pre_loopopt" {
entry:
  %struct.gep = getelementptr inbounds %struct, ptr %p, i64 %iv
  %gep = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 1 
  %ld = load double, ptr %gep, align 8
  %gep1 = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 0
  %ld1 = load double, ptr %gep1, align 8
  %add = fadd double %ld, %ld1
  ret double %add
}

; CHECK-LABEL: bar
; CHECK: %gep1 = getelementptr inbounds %struct, ptr %p, i64 %iv{{[[:space:]]}}

define double @bar(ptr %p, i64 %iv) {
entry:
  %struct.gep = getelementptr inbounds %struct, ptr %p, i64 %iv
  %gep = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 1 
  %ld = load double, ptr %gep, align 8
  %gep1 = getelementptr inbounds %struct, ptr %struct.gep, i64 0, i32 0
  %ld1 = load double, ptr %gep1, align 8
  %add = fadd double %ld, %ld1
  ret double %add
}
