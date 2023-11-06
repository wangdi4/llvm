;RUN: opt -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -debug-only=dtrans-pta-verbose -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that the pointer type analyzer detects dest of InsertValue
; is only used by ResumeInst via Phi instruction.

%struct.test01 = type { i64, i64 }

define i32 @test1() personality ptr null {
entry:
  unreachable

lpad:                                             ; No predecessors!
  %0 = landingpad { ptr, i32 }
          cleanup
  %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1
  br label %common.resume

common.resume:                                    ; preds = %lpad.i.i, %lpad
  %common.resume.op = phi { ptr, i32 } [ %lpad.val66, %lpad ], [ zeroinitializer, %lpad.i.i ]
  resume { ptr, i32 } %common.resume.op

lpad.i.i:                                         ; No predecessors!
  %1 = landingpad { ptr, i32 }
          cleanup
  br label %common.resume
}
; CHECK: Added alias [DECL]: [test1] %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1 -- { i8*, i32 } = type { i8*, i32 }
; CHECK: Added alias [USE]: [test1] %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1 -- { i8*, i32 } = type { i8*, i32 }
; CHECK-NOT: Marked as unhandled: [test1]   %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1
define i32 @test2() personality ptr null {
entry:
  unreachable

lpad:                                             ; No predecessors!
  %0 = landingpad { ptr, i32 }
          cleanup
  %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1
  br label %common.loop

common.loop:                                    ; preds = %lpad.i.i, %lpad
  %common.resume.pre = phi { ptr, i32 } [ %lpad.val66, %lpad ], [ %common.resume.pre, %common.loop ]
  br i1 undef, label %common.resume, label %common.loop

common.resume:
  %common.resume.op = phi { ptr, i32 } [ %common.resume.pre, %common.loop], [ zeroinitializer, %lpad.i.i ]
  resume { ptr, i32 } %common.resume.op

lpad.i.i:                                         ; No predecessors!
  %1 = landingpad { ptr, i32 }
          cleanup
  br label %common.resume

}
; CHECK: Added alias [DECL]: [test2] %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1 -- { i8*, i32 } = type { i8*, i32 }
; CHECK: Added alias [USE]: [test2] %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1 -- { i8*, i32 } = type { i8*, i32 }
; CHECK-NOT: Marked as unhandled: [test2]   %lpad.val66 = insertvalue { ptr, i32 } zeroinitializer, i32 0, 1

; uselistorder directives

!1 = !{i64 0, i32 0}  ; i64
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }

!intel.dtrans.types = !{!2}

