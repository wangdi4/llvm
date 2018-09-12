; RUN: opt < %s -whole-program-assume -passes="dtrans-deletefield" -S 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-deletefield -S 2>&1 | FileCheck %s

; Check that the unused second field (%y) is deleted

; CHECK: %__DFT_struct.A = type { i32 }

%struct.A = type { i32, i32 }

; void foo(struct A* a) {
;   a->x++;
;   a->y++;
; }
define void @foo(%struct.A* %a) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %0 = load i32, i32* %x, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %x, align 4
  %y = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 1
  %1 = load i32, i32* %y, align 4
  %inc1 = add nsw i32 %1, 1
  store i32 %inc1, i32* %y, align 4
  ret void
}


;void bar(int *p, struct A* a) {
;  *p = a->x;
;}
define void @bar(i32* %p, %struct.A* %a) {
entry:
  %x = getelementptr inbounds %struct.A, %struct.A* %a, i64 0, i32 0
  %0 = load i32, i32* %x, align 4
  store i32 %0, i32* %p, align 4
  ret void
}

