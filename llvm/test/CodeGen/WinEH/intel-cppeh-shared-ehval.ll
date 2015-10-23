; RUN: opt -mtriple=x86_64-pc-windows-msvc -winehprepare -S -o - < %s | FileCheck %s

; This test case is based on the following code with some simplificiations made
; for test readability.
;
; void test() {
;   try {
;     may_throw();
;   } catch (...) {
;     int i;
;     i = get_val();
;     try {
;       may_throw();
;     } catch (...) {
;       use_val(i);
;     }
;   }
; }

; This tests that a value defined in one exception handler and used in another
; is properly demoted to the stack so that both handlers are accessing the same
; value.

define void @test() personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
entry:
  invoke void @may_throw()
          to label %try.cont unwind label %lpad

lpad:
  %lp = landingpad { i8*, i32 }
          catch i8* null
  %exn = extractvalue { i8*, i32 } %lp, 0
  call void @llvm.eh.begincatch(i8* %exn, i8* null)
  %i = call i32 @get_val()
  invoke void @may_throw()
          to label %try.cont.inner unwind label %lpad2

lpad2:
  %lp2 = landingpad { i8*, i32 }
          catch i8* null
  %exn2 = extractvalue { i8*, i32 } %lp2, 0
  call void @llvm.eh.begincatch(i8* %exn2, i8* null)
  call void @use_val(i32 %i)
  call void @llvm.eh.endcatch()
  br label %try.cont.inner

try.cont.inner:
  call void @llvm.eh.endcatch()
  br label %try.cont

try.cont:
  ret void
}

; CHECK: define void @test()
; CHECK: entry:
; CHECK:   [[I_REG2MEM:\%.+]] = alloca i32
; CHECK:   call void (...) @llvm.localescape(i32* [[I_REG2MEM]])
; CHECK: }

; CHECK-LABEL: define internal i8* @test.catch(i8*, i8*)
; CHECK: entry:
; CHECK:   [[I_REG2MEM_I8PTR:\%.+]] = call i8* @llvm.localrecover(i8* bitcast (void ()* @test to i8*), i8* [[FP:\%.+]], i32 0)
; CHECK:   [[I_REG2MEM_I32PTR:\%.+]] = bitcast i8* [[I_REG2MEM_I8PTR]] to i32*
; CHECK:   [[I_RELOAD:\%.+]] = load i32, i32* [[I_REG2MEM_I32PTR]]
; CHECK:   call void @use_val(i32 [[I_RELOAD]])
; CHECK: }

; CHECK-LABEL: define internal i8* @test.catch.1(i8*, i8*)
; CHECK: entry:
; CHECK:   [[I_REG2MEM_I8PTR:\%.+]] = call i8* @llvm.localrecover(i8* bitcast (void ()* @test to i8*), i8* [[FP:\%.+]], i32 0)
; CHECK:   [[I_REG2MEM_I32PTR:\%.+]] = bitcast i8* [[I_REG2MEM_I8PTR]] to i32*
; CHECK:   [[I:\%.+]] = call i32 @get_val()
; CHECK:   store i32 [[I]], i32* [[I_REG2MEM_I32PTR]]
; CHECK: }

declare i32 @get_val()
declare void @use_val(i32)
declare void @may_throw()
declare i32 @__CxxFrameHandler3(...)
declare void @llvm.eh.begincatch(i8* nocapture, i8* nocapture)
declare void @llvm.eh.endcatch()