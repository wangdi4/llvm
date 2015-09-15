; RUN: llc -mtriple=i686-pc-windows-msvc < %s | FileCheck %s

; INTEL_CUSTOMIZATION
; In LLVM trunk, this test checks to make sure EH tables are generated and setup
; even when the code that requires them has been removed (as with the invoke of
; a nounwind function that is converted to a simple call).
;
; In xmain, we've taken the approach of instead supressing all EH related code
; and tables when they are not needed, so the test has been updated to check
; for the absence of EH-related symbols.
; END INTEL_CUSTOMIZATION

; Sometimes invokes of nounwind functions make it through to CodeGen, especially
; at -O0, where Clang sometimes optimistically annotates functions as nounwind.
; WinEHPrepare ends up outlining functions, and emitting references to LSDA
; labels. Make sure we emit the LSDA in that case.

declare i32 @__CxxFrameHandler3(...)
declare void @nounwind_func() nounwind
declare void @cleanup()

define void @should_emit_tables() personality i32 (...)* @__CxxFrameHandler3 {
entry:
  invoke void @nounwind_func()
      to label %done unwind label %lpad

done:
  ret void

lpad:
  %vals = landingpad { i8*, i32 }
      cleanup
  call void @cleanup()
  resume { i8*, i32 } %vals
}

; CHECK: _should_emit_tables:
; CHECK: calll _nounwind_func
; CHECK: retl

; INTEL_CUSTOMIZATION
; CHECK-NOT: L__ehtable$should_emit_tables
; CHECK-NOT: ___ehhandler$should_emit_tables
; CHECK-NOT: ___CxxFrameHandler3
; END INTEL_CUSTOMIZATION

; NOT INTEL_CUSTOMIZATION (i.e. the old checks)
; DONT-CHECK: _should_emit_tables:
; DONT-CHECK: calll _nounwind_func
; DONT-CHECK: retl

; DONT-CHECK: L__ehtable$should_emit_tables:

; DONT-CHECK: ___ehhandler$should_emit_tables:
; DONT-CHECK: movl $L__ehtable$should_emit_tables, %eax
; DONT-CHECK: jmp ___CxxFrameHandler3 # TAILCALL
; END NOT INTEL_CUSTOMIZATION
