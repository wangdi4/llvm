; XFAIL: x86_64-pc-win32, i686-pc-win32
; RUN: llc < %s -march=x86-64 | grep {movq	8(%rdi), %rdx}
; RUN: llc < %s -march=x86-64 | grep {movq	(%rdi), %rax}

define i128 @test(i128 *%P) {
        %A = load i128* %P
        ret i128 %A
}

