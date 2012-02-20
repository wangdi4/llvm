; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s
;

target datalayout = "e-p:64:64"

declare <16 x float> @foo()

define <16 x float> @test1() nounwind alwaysinline {
; KNF: test1:
; KNF: callq foo
; KNF: callq foo
; KNF: vaddps
;
; KNC: test1:
; KNC: callq foo
; KNC: callq foo
; KNC: vaddps
start:
        %val1 = call <16 x float> @foo()
        %val2 = call <16 x float> @foo()
        %val = fadd <16 x float> %val1, %val2
        ret <16 x float> %val
}


define i32 @Triad(i32 %v, float %s) nounwind {
entry:
; CHECK: call get_global_id
  %call = tail call i32 @get_global_id(i32 0, float %s) nounwind
  %0 = add i32 %v, %call
  ret i32 %0
}

declare void @caleeee1(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %g, i32 %h, i32 %i, i32 %j, i32 %k, i32 %l) nounwind


define void @caleeee(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %g, i32 %h, i32 %i, i32 %j, i32 %k, i32 %l) nounwind
{
; CHECK: subq      ${{[0-9]+}}, %rsp
; CHECK: movl      %r{{[0-9a-z]+}}, {{[0-9]+}}(%rsp)
; CHECK: call      caleeee1
; CHECK: addq      ${{[0-9]+}}, %rsp
  call void @caleeee1(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %g, i32 %h, i32 %i, i32 %j, i32 %k, i32 %l);
  ret void
}


define void @callerrrr() nounwind {
entry:
; CHECK: subq      ${{[0-9]+}}, %rsp
; CHECK: movl      $0, {{[0-9]+}}(%rsp)
; CHECK: call      caleeee
; CHECK: addq      ${{[0-9]+}}, %rsp
  call void @caleeee(i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0)
  ret void
}

declare i32 @get_global_id(i32, float)

