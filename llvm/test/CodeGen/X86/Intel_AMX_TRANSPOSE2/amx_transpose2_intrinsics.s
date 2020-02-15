	.text
	.file	"amx_transpose2_intrinsics.ll"
	.globl	test_amx                # -- Begin function test_amx
	.p2align	4, 0x90
	.type	test_amx,@function
test_amx:                               # @test_amx
	.cfi_startproc
# %bb.0:
	subq	$56, %rsp
	.cfi_def_cfa_offset 64
	movl	$3, %eax
	movq	%rdi, 48(%rsp)          # 8-byte Spill
	movl	%eax, %edi
	movq	%rsi, 40(%rsp)          # 8-byte Spill
	movq	%r9, %rsi
	movl	%edx, 36(%rsp)          # 4-byte Spill
	movq	%rcx, %rdx
	movq	%rcx, 24(%rsp)          # 8-byte Spill
	movq	%r8, %rcx
	movq	%r8, 16(%rsp)           # 8-byte Spill
	movq	%r9, 8(%rsp)            # 8-byte Spill
	callq	llvm.x86.t2transposew
	movl	$4, %edi
	movq	8(%rsp), %rsi           # 8-byte Reload
	movq	24(%rsp), %rdx          # 8-byte Reload
	movq	16(%rsp), %rcx          # 8-byte Reload
	callq	llvm.x86.t2transposewt1
	addq	$56, %rsp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	test_amx, .Lfunc_end0-test_amx
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
