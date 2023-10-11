	.text
	.file	"movmsk-cmp.ll"
	.globl	allones_v16i8_sign              # -- Begin function allones_v16i8_sign
	.p2align	4, 0x90
	.type	allones_v16i8_sign,@function
allones_v16i8_sign:                     # @allones_v16i8_sign
	.cfi_startproc
# %bb.0:
	vpmovmskb	%xmm0, %eax
	cmpl	$65535, %eax                    # imm = 0xFFFF
	sete	%al
	retq
.Lfunc_end0:
	.size	allones_v16i8_sign, .Lfunc_end0-allones_v16i8_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v16i8_sign             # -- Begin function allzeros_v16i8_sign
	.p2align	4, 0x90
	.type	allzeros_v16i8_sign,@function
allzeros_v16i8_sign:                    # @allzeros_v16i8_sign
	.cfi_startproc
# %bb.0:
	vpmovmskb	%xmm0, %eax
	testl	%eax, %eax
	sete	%al
	retq
.Lfunc_end1:
	.size	allzeros_v16i8_sign, .Lfunc_end1-allzeros_v16i8_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v32i8_sign              # -- Begin function allones_v32i8_sign
	.p2align	4, 0x90
	.type	allones_v32i8_sign,@function
allones_v32i8_sign:                     # @allones_v32i8_sign
	.cfi_startproc
# %bb.0:
	vpmovmskb	%ymm0, %eax
	cmpl	$-1, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end2:
	.size	allones_v32i8_sign, .Lfunc_end2-allones_v32i8_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v32i8_sign             # -- Begin function allzeros_v32i8_sign
	.p2align	4, 0x90
	.type	allzeros_v32i8_sign,@function
allzeros_v32i8_sign:                    # @allzeros_v32i8_sign
	.cfi_startproc
# %bb.0:
	vpmovmskb	%ymm0, %eax
	testl	%eax, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end3:
	.size	allzeros_v32i8_sign, .Lfunc_end3-allzeros_v32i8_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v64i8_sign              # -- Begin function allones_v64i8_sign
	.p2align	4, 0x90
	.type	allones_v64i8_sign,@function
allones_v64i8_sign:                     # @allones_v64i8_sign
	.cfi_startproc
# %bb.0:
	vextracti64x4	$1, %zmm0, %ymm1
	vpand	%ymm0, %ymm1, %ymm0
	vpmovmskb	%ymm0, %eax
	cmpl	$-1, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end4:
	.size	allones_v64i8_sign, .Lfunc_end4-allones_v64i8_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v64i8_sign             # -- Begin function allzeros_v64i8_sign
	.p2align	4, 0x90
	.type	allzeros_v64i8_sign,@function
allzeros_v64i8_sign:                    # @allzeros_v64i8_sign
	.cfi_startproc
# %bb.0:
	vextracti64x4	$1, %zmm0, %ymm1
	vpor	%ymm1, %ymm0, %ymm0
	vpmovmskb	%ymm0, %eax
	testl	%eax, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end5:
	.size	allzeros_v64i8_sign, .Lfunc_end5-allzeros_v64i8_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v8i16_sign              # -- Begin function allones_v8i16_sign
	.p2align	4, 0x90
	.type	allones_v8i16_sign,@function
allones_v8i16_sign:                     # @allones_v8i16_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtw	%xmm0, %xmm1, %xmm0
	vpmovsxwq	%xmm0, %zmm0
	vptestmq	%zmm0, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end6:
	.size	allones_v8i16_sign, .Lfunc_end6-allones_v8i16_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v8i16_sign             # -- Begin function allzeros_v8i16_sign
	.p2align	4, 0x90
	.type	allzeros_v8i16_sign,@function
allzeros_v8i16_sign:                    # @allzeros_v8i16_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtw	%xmm0, %xmm1, %xmm0
	vpmovsxwq	%xmm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end7:
	.size	allzeros_v8i16_sign, .Lfunc_end7-allzeros_v8i16_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v16i16_sign             # -- Begin function allones_v16i16_sign
	.p2align	4, 0x90
	.type	allones_v16i16_sign,@function
allones_v16i16_sign:                    # @allones_v16i16_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtw	%ymm0, %ymm1, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	setb	%al
	vzeroupper
	retq
.Lfunc_end8:
	.size	allones_v16i16_sign, .Lfunc_end8-allones_v16i16_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v16i16_sign            # -- Begin function allzeros_v16i16_sign
	.p2align	4, 0x90
	.type	allzeros_v16i16_sign,@function
allzeros_v16i16_sign:                   # @allzeros_v16i16_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtw	%ymm0, %ymm1, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end9:
	.size	allzeros_v16i16_sign, .Lfunc_end9-allzeros_v16i16_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v32i16_sign             # -- Begin function allones_v32i16_sign
	.p2align	4, 0x90
	.type	allones_v32i16_sign,@function
allones_v32i16_sign:                    # @allones_v32i16_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtw	%ymm0, %ymm1, %ymm2
	vpmovsxwd	%ymm2, %zmm2
	vptestmd	%zmm2, %zmm2, %k0
	kmovw	%k0, %eax
	vextracti64x4	$1, %zmm0, %ymm0
	vpcmpgtw	%ymm0, %ymm1, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kmovw	%k0, %ecx
	andl	%eax, %ecx
	cmpl	$65535, %ecx                    # imm = 0xFFFF
	sete	%al
	vzeroupper
	retq
.Lfunc_end10:
	.size	allones_v32i16_sign, .Lfunc_end10-allones_v32i16_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v32i16_sign            # -- Begin function allzeros_v32i16_sign
	.p2align	4, 0x90
	.type	allzeros_v32i16_sign,@function
allzeros_v32i16_sign:                   # @allzeros_v32i16_sign
	.cfi_startproc
# %bb.0:
	vextracti64x4	$1, %zmm0, %ymm1
	vpxor	%xmm2, %xmm2, %xmm2
	vpcmpgtw	%ymm1, %ymm2, %ymm1
	vpcmpgtw	%ymm0, %ymm2, %ymm0
	vpor	%ymm1, %ymm0, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end11:
	.size	allzeros_v32i16_sign, .Lfunc_end11-allzeros_v32i16_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v4i32_sign              # -- Begin function allones_v4i32_sign
	.p2align	4, 0x90
	.type	allones_v4i32_sign,@function
allones_v4i32_sign:                     # @allones_v4i32_sign
	.cfi_startproc
# %bb.0:
	vpcmpeqd	%xmm1, %xmm1, %xmm1
	vtestps	%xmm1, %xmm0
	setb	%al
	retq
.Lfunc_end12:
	.size	allones_v4i32_sign, .Lfunc_end12-allones_v4i32_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v4i32_sign             # -- Begin function allzeros_v4i32_sign
	.p2align	4, 0x90
	.type	allzeros_v4i32_sign,@function
allzeros_v4i32_sign:                    # @allzeros_v4i32_sign
	.cfi_startproc
# %bb.0:
	vtestps	%xmm0, %xmm0
	sete	%al
	retq
.Lfunc_end13:
	.size	allzeros_v4i32_sign, .Lfunc_end13-allzeros_v4i32_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v8i32_sign              # -- Begin function allones_v8i32_sign
	.p2align	4, 0x90
	.type	allones_v8i32_sign,@function
allones_v8i32_sign:                     # @allones_v8i32_sign
	.cfi_startproc
# %bb.0:
	vpcmpeqd	%ymm1, %ymm1, %ymm1
	vtestps	%ymm1, %ymm0
	setb	%al
	vzeroupper
	retq
.Lfunc_end14:
	.size	allones_v8i32_sign, .Lfunc_end14-allones_v8i32_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v8i32_sign             # -- Begin function allzeros_v8i32_sign
	.p2align	4, 0x90
	.type	allzeros_v8i32_sign,@function
allzeros_v8i32_sign:                    # @allzeros_v8i32_sign
	.cfi_startproc
# %bb.0:
	vtestps	%ymm0, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end15:
	.size	allzeros_v8i32_sign, .Lfunc_end15-allzeros_v8i32_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v16i32_sign             # -- Begin function allones_v16i32_sign
	.p2align	4, 0x90
	.type	allones_v16i32_sign,@function
allones_v16i32_sign:                    # @allones_v16i32_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtd	%zmm0, %zmm1, %k0
	kortestw	%k0, %k0
	setb	%al
	vzeroupper
	retq
.Lfunc_end16:
	.size	allones_v16i32_sign, .Lfunc_end16-allones_v16i32_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v16i32_sign            # -- Begin function allzeros_v16i32_sign
	.p2align	4, 0x90
	.type	allzeros_v16i32_sign,@function
allzeros_v16i32_sign:                   # @allzeros_v16i32_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtd	%zmm0, %zmm1, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end17:
	.size	allzeros_v16i32_sign, .Lfunc_end17-allzeros_v16i32_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v4i64_sign              # -- Begin function allones_v4i64_sign
	.p2align	4, 0x90
	.type	allones_v4i64_sign,@function
allones_v4i64_sign:                     # @allones_v4i64_sign
	.cfi_startproc
# %bb.0:
	vpcmpeqd	%ymm1, %ymm1, %ymm1
	vtestpd	%ymm1, %ymm0
	setb	%al
	vzeroupper
	retq
.Lfunc_end18:
	.size	allones_v4i64_sign, .Lfunc_end18-allones_v4i64_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v4i64_sign             # -- Begin function allzeros_v4i64_sign
	.p2align	4, 0x90
	.type	allzeros_v4i64_sign,@function
allzeros_v4i64_sign:                    # @allzeros_v4i64_sign
	.cfi_startproc
# %bb.0:
	vtestpd	%ymm0, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end19:
	.size	allzeros_v4i64_sign, .Lfunc_end19-allzeros_v4i64_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v8i64_sign              # -- Begin function allones_v8i64_sign
	.p2align	4, 0x90
	.type	allones_v8i64_sign,@function
allones_v8i64_sign:                     # @allones_v8i64_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtq	%zmm0, %zmm1, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end20:
	.size	allones_v8i64_sign, .Lfunc_end20-allones_v8i64_sign
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v8i64_sign             # -- Begin function allzeros_v8i64_sign
	.p2align	4, 0x90
	.type	allzeros_v8i64_sign,@function
allzeros_v8i64_sign:                    # @allzeros_v8i64_sign
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm1, %xmm1
	vpcmpgtq	%zmm0, %zmm1, %k0
	kmovw	%k0, %eax
	testb	%al, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end21:
	.size	allzeros_v8i64_sign, .Lfunc_end21-allzeros_v8i64_sign
	.cfi_endproc
                                        # -- End function
	.globl	allones_v16i8_and1              # -- Begin function allones_v16i8_and1
	.p2align	4, 0x90
	.type	allones_v16i8_and1,@function
allones_v16i8_and1:                     # @allones_v16i8_and1
	.cfi_startproc
# %bb.0:
	vpsllw	$7, %xmm0, %xmm0
	vpmovmskb	%xmm0, %eax
	cmpl	$65535, %eax                    # imm = 0xFFFF
	sete	%al
	retq
.Lfunc_end22:
	.size	allones_v16i8_and1, .Lfunc_end22-allones_v16i8_and1
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v16i8_not              # -- Begin function allzeros_v16i8_not
	.p2align	4, 0x90
	.type	allzeros_v16i8_not,@function
allzeros_v16i8_not:                     # @allzeros_v16i8_not
	.cfi_startproc
# %bb.0:
	vptest	%xmm0, %xmm0
	setne	%al
	retq
.Lfunc_end23:
	.size	allzeros_v16i8_not, .Lfunc_end23-allzeros_v16i8_not
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v2i64_not              # -- Begin function allzeros_v2i64_not
	.p2align	4, 0x90
	.type	allzeros_v2i64_not,@function
allzeros_v2i64_not:                     # @allzeros_v2i64_not
	.cfi_startproc
# %bb.0:
	vptest	%xmm0, %xmm0
	setne	%al
	retq
.Lfunc_end24:
	.size	allzeros_v2i64_not, .Lfunc_end24-allzeros_v2i64_not
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v8i32_not              # -- Begin function allzeros_v8i32_not
	.p2align	4, 0x90
	.type	allzeros_v8i32_not,@function
allzeros_v8i32_not:                     # @allzeros_v8i32_not
	.cfi_startproc
# %bb.0:
	vptest	%ymm0, %ymm0
	setne	%al
	vzeroupper
	retq
.Lfunc_end25:
	.size	allzeros_v8i32_not, .Lfunc_end25-allzeros_v8i32_not
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v8i64_not              # -- Begin function allzeros_v8i64_not
	.p2align	4, 0x90
	.type	allzeros_v8i64_not,@function
allzeros_v8i64_not:                     # @allzeros_v8i64_not
	.cfi_startproc
# %bb.0:
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	setne	%al
	vzeroupper
	retq
.Lfunc_end26:
	.size	allzeros_v8i64_not, .Lfunc_end26-allzeros_v8i64_not
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v16i8_and1
.LCPI27_0:
	.zero	16,1
	.text
	.globl	allzeros_v16i8_and1
	.p2align	4, 0x90
	.type	allzeros_v16i8_and1,@function
allzeros_v16i8_and1:                    # @allzeros_v16i8_and1
	.cfi_startproc
# %bb.0:
	vptest	.LCPI27_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end27:
	.size	allzeros_v16i8_and1, .Lfunc_end27-allzeros_v16i8_and1
	.cfi_endproc
                                        # -- End function
	.globl	allones_v32i8_and1              # -- Begin function allones_v32i8_and1
	.p2align	4, 0x90
	.type	allones_v32i8_and1,@function
allones_v32i8_and1:                     # @allones_v32i8_and1
	.cfi_startproc
# %bb.0:
	vpsllw	$7, %ymm0, %ymm0
	vpmovmskb	%ymm0, %eax
	cmpl	$-1, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end28:
	.size	allones_v32i8_and1, .Lfunc_end28-allones_v32i8_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v32i8_and1
.LCPI29_0:
	.quad	72340172838076673               # 0x101010101010101
	.text
	.globl	allzeros_v32i8_and1
	.p2align	4, 0x90
	.type	allzeros_v32i8_and1,@function
allzeros_v32i8_and1:                    # @allzeros_v32i8_and1
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI29_0(%rip), %ymm1  # ymm1 = [72340172838076673,72340172838076673,72340172838076673,72340172838076673]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end29:
	.size	allzeros_v32i8_and1, .Lfunc_end29-allzeros_v32i8_and1
	.cfi_endproc
                                        # -- End function
	.globl	allones_v64i8_and1              # -- Begin function allones_v64i8_and1
	.p2align	4, 0x90
	.type	allones_v64i8_and1,@function
allones_v64i8_and1:                     # @allones_v64i8_and1
	.cfi_startproc
# %bb.0:
	vextracti64x4	$1, %zmm0, %ymm1
	vpand	%ymm0, %ymm1, %ymm0
	vpsllw	$7, %ymm0, %ymm0
	vpmovmskb	%ymm0, %eax
	cmpl	$-1, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end30:
	.size	allones_v64i8_and1, .Lfunc_end30-allones_v64i8_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0                          # -- Begin function allzeros_v64i8_and1
.LCPI31_0:
	.zero	64,1
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI31_1:
	.zero	4,1
	.text
	.globl	allzeros_v64i8_and1
	.p2align	4, 0x90
	.type	allzeros_v64i8_and1,@function
allzeros_v64i8_and1:                    # @allzeros_v64i8_and1
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI31_1(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end31:
	.size	allzeros_v64i8_and1, .Lfunc_end31-allzeros_v64i8_and1
	.cfi_endproc
                                        # -- End function
	.globl	allones_v8i16_and1              # -- Begin function allones_v8i16_and1
	.p2align	4, 0x90
	.type	allones_v8i16_and1,@function
allones_v8i16_and1:                     # @allones_v8i16_and1
	.cfi_startproc
# %bb.0:
	vpsllw	$15, %xmm0, %xmm0
	vpsraw	$15, %xmm0, %xmm0
	vpmovsxwq	%xmm0, %zmm0
	vptestmq	%zmm0, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end32:
	.size	allones_v8i16_and1, .Lfunc_end32-allones_v8i16_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v8i16_and1
.LCPI33_0:
	.quad	281479271743489                 # 0x1000100010001
	.quad	281479271743489                 # 0x1000100010001
	.text
	.globl	allzeros_v8i16_and1
	.p2align	4, 0x90
	.type	allzeros_v8i16_and1,@function
allzeros_v8i16_and1:                    # @allzeros_v8i16_and1
	.cfi_startproc
# %bb.0:
	vptest	.LCPI33_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end33:
	.size	allzeros_v8i16_and1, .Lfunc_end33-allzeros_v8i16_and1
	.cfi_endproc
                                        # -- End function
	.globl	allones_v16i16_and1             # -- Begin function allones_v16i16_and1
	.p2align	4, 0x90
	.type	allones_v16i16_and1,@function
allones_v16i16_and1:                    # @allones_v16i16_and1
	.cfi_startproc
# %bb.0:
	vpsllw	$15, %ymm0, %ymm0
	vpsraw	$15, %ymm0, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	setb	%al
	vzeroupper
	retq
.Lfunc_end34:
	.size	allones_v16i16_and1, .Lfunc_end34-allones_v16i16_and1
	.cfi_endproc
                                        # -- End function
	.globl	allones_v32i16_and1             # -- Begin function allones_v32i16_and1
	.p2align	4, 0x90
	.type	allones_v32i16_and1,@function
allones_v32i16_and1:                    # @allones_v32i16_and1
	.cfi_startproc
# %bb.0:
	vpsllw	$15, %ymm0, %ymm1
	vpsraw	$15, %ymm1, %ymm1
	vpmovsxwd	%ymm1, %zmm1
	vptestmd	%zmm1, %zmm1, %k0
	kmovw	%k0, %eax
	vextracti64x4	$1, %zmm0, %ymm0
	vpsllw	$15, %ymm0, %ymm0
	vpsraw	$15, %ymm0, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kmovw	%k0, %ecx
	andl	%eax, %ecx
	cmpl	$65535, %ecx                    # imm = 0xFFFF
	sete	%al
	vzeroupper
	retq
.Lfunc_end35:
	.size	allones_v32i16_and1, .Lfunc_end35-allones_v32i16_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0                          # -- Begin function allzeros_v32i16_and1
.LCPI36_0:
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.short	1                               # 0x1
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI36_1:
	.short	1                               # 0x1
	.short	1                               # 0x1
	.text
	.globl	allzeros_v32i16_and1
	.p2align	4, 0x90
	.type	allzeros_v32i16_and1,@function
allzeros_v32i16_and1:                   # @allzeros_v32i16_and1
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI36_1(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end36:
	.size	allzeros_v32i16_and1, .Lfunc_end36-allzeros_v32i16_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v16i16_and1
.LCPI37_0:
	.quad	281479271743489                 # 0x1000100010001
	.text
	.globl	allzeros_v16i16_and1
	.p2align	4, 0x90
	.type	allzeros_v16i16_and1,@function
allzeros_v16i16_and1:                   # @allzeros_v16i16_and1
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI37_0(%rip), %ymm1  # ymm1 = [281479271743489,281479271743489,281479271743489,281479271743489]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end37:
	.size	allzeros_v16i16_and1, .Lfunc_end37-allzeros_v16i16_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allones_v4i32_and1
.LCPI38_0:
	.long	1                               # 0x1
	.text
	.globl	allones_v4i32_and1
	.p2align	4, 0x90
	.type	allones_v4i32_and1,@function
allones_v4i32_and1:                     # @allones_v4i32_and1
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vptestnmd	.LCPI38_0(%rip){1to16}, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$15, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end38:
	.size	allones_v4i32_and1, .Lfunc_end38-allones_v4i32_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v4i32_and1
.LCPI39_0:
	.quad	4294967297                      # 0x100000001
	.quad	4294967297                      # 0x100000001
	.text
	.globl	allzeros_v4i32_and1
	.p2align	4, 0x90
	.type	allzeros_v4i32_and1,@function
allzeros_v4i32_and1:                    # @allzeros_v4i32_and1
	.cfi_startproc
# %bb.0:
	vptest	.LCPI39_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end39:
	.size	allzeros_v4i32_and1, .Lfunc_end39-allzeros_v4i32_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allones_v8i32_and1
.LCPI40_0:
	.long	1                               # 0x1
	.text
	.globl	allones_v8i32_and1
	.p2align	4, 0x90
	.type	allones_v8i32_and1,@function
allones_v8i32_and1:                     # @allones_v8i32_and1
	.cfi_startproc
# %bb.0:
                                        # kill: def $ymm0 killed $ymm0 def $zmm0
	vptestmd	.LCPI40_0(%rip){1to16}, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end40:
	.size	allones_v8i32_and1, .Lfunc_end40-allones_v8i32_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v8i32_and1
.LCPI41_0:
	.quad	4294967297                      # 0x100000001
	.text
	.globl	allzeros_v8i32_and1
	.p2align	4, 0x90
	.type	allzeros_v8i32_and1,@function
allzeros_v8i32_and1:                    # @allzeros_v8i32_and1
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI41_0(%rip), %ymm1  # ymm1 = [4294967297,4294967297,4294967297,4294967297]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end41:
	.size	allzeros_v8i32_and1, .Lfunc_end41-allzeros_v8i32_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allones_v16i32_and1
.LCPI42_0:
	.long	1                               # 0x1
	.text
	.globl	allones_v16i32_and1
	.p2align	4, 0x90
	.type	allones_v16i32_and1,@function
allones_v16i32_and1:                    # @allones_v16i32_and1
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI42_0(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	setb	%al
	vzeroupper
	retq
.Lfunc_end42:
	.size	allones_v16i32_and1, .Lfunc_end42-allones_v16i32_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allzeros_v16i32_and1
.LCPI43_0:
	.long	1                               # 0x1
	.text
	.globl	allzeros_v16i32_and1
	.p2align	4, 0x90
	.type	allzeros_v16i32_and1,@function
allzeros_v16i32_and1:                   # @allzeros_v16i32_and1
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI43_0(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end43:
	.size	allzeros_v16i32_and1, .Lfunc_end43-allzeros_v16i32_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allones_v2i64_and1
.LCPI44_0:
	.quad	1                               # 0x1
	.quad	1                               # 0x1
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0
.LCPI44_1:
	.quad	1                               # 0x1
	.text
	.globl	allones_v2i64_and1
	.p2align	4, 0x90
	.type	allones_v2i64_and1,@function
allones_v2i64_and1:                     # @allones_v2i64_and1
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vpbroadcastq	.LCPI44_1(%rip), %xmm1  # xmm1 = [1,1]
	vptestnmq	%zmm1, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end44:
	.size	allones_v2i64_and1, .Lfunc_end44-allones_v2i64_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v2i64_and1
.LCPI45_0:
	.quad	1                               # 0x1
	.quad	1                               # 0x1
	.text
	.globl	allzeros_v2i64_and1
	.p2align	4, 0x90
	.type	allzeros_v2i64_and1,@function
allzeros_v2i64_and1:                    # @allzeros_v2i64_and1
	.cfi_startproc
# %bb.0:
	vptest	.LCPI45_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end45:
	.size	allzeros_v2i64_and1, .Lfunc_end45-allzeros_v2i64_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allones_v4i64_and1
.LCPI46_0:
	.quad	1                               # 0x1
	.text
	.globl	allones_v4i64_and1
	.p2align	4, 0x90
	.type	allones_v4i64_and1,@function
allones_v4i64_and1:                     # @allones_v4i64_and1
	.cfi_startproc
# %bb.0:
                                        # kill: def $ymm0 killed $ymm0 def $zmm0
	vptestnmq	.LCPI46_0(%rip){1to8}, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$15, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end46:
	.size	allones_v4i64_and1, .Lfunc_end46-allones_v4i64_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v4i64_and1
.LCPI47_0:
	.quad	1                               # 0x1
	.text
	.globl	allzeros_v4i64_and1
	.p2align	4, 0x90
	.type	allzeros_v4i64_and1,@function
allzeros_v4i64_and1:                    # @allzeros_v4i64_and1
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI47_0(%rip), %ymm1  # ymm1 = [1,1,1,1]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end47:
	.size	allzeros_v4i64_and1, .Lfunc_end47-allzeros_v4i64_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allones_v8i64_and1
.LCPI48_0:
	.quad	1                               # 0x1
	.text
	.globl	allones_v8i64_and1
	.p2align	4, 0x90
	.type	allones_v8i64_and1,@function
allones_v8i64_and1:                     # @allones_v8i64_and1
	.cfi_startproc
# %bb.0:
	vptestmq	.LCPI48_0(%rip){1to8}, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end48:
	.size	allones_v8i64_and1, .Lfunc_end48-allones_v8i64_and1
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v8i64_and1
.LCPI49_0:
	.quad	1                               # 0x1
	.text
	.globl	allzeros_v8i64_and1
	.p2align	4, 0x90
	.type	allzeros_v8i64_and1,@function
allzeros_v8i64_and1:                    # @allzeros_v8i64_and1
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI49_0(%rip), %zmm1  # zmm1 = [1,1,1,1,1,1,1,1]
	vptestmd	%zmm1, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end49:
	.size	allzeros_v8i64_and1, .Lfunc_end49-allzeros_v8i64_and1
	.cfi_endproc
                                        # -- End function
	.globl	allones_v16i8_and4              # -- Begin function allones_v16i8_and4
	.p2align	4, 0x90
	.type	allones_v16i8_and4,@function
allones_v16i8_and4:                     # @allones_v16i8_and4
	.cfi_startproc
# %bb.0:
	vpsllw	$5, %xmm0, %xmm0
	vpmovmskb	%xmm0, %eax
	cmpl	$65535, %eax                    # imm = 0xFFFF
	sete	%al
	retq
.Lfunc_end50:
	.size	allones_v16i8_and4, .Lfunc_end50-allones_v16i8_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v16i8_and4
.LCPI51_0:
	.zero	16,4
	.text
	.globl	allzeros_v16i8_and4
	.p2align	4, 0x90
	.type	allzeros_v16i8_and4,@function
allzeros_v16i8_and4:                    # @allzeros_v16i8_and4
	.cfi_startproc
# %bb.0:
	vptest	.LCPI51_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end51:
	.size	allzeros_v16i8_and4, .Lfunc_end51-allzeros_v16i8_and4
	.cfi_endproc
                                        # -- End function
	.globl	allones_v32i8_and4              # -- Begin function allones_v32i8_and4
	.p2align	4, 0x90
	.type	allones_v32i8_and4,@function
allones_v32i8_and4:                     # @allones_v32i8_and4
	.cfi_startproc
# %bb.0:
	vpsllw	$5, %ymm0, %ymm0
	vpmovmskb	%ymm0, %eax
	cmpl	$-1, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end52:
	.size	allones_v32i8_and4, .Lfunc_end52-allones_v32i8_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v32i8_and4
.LCPI53_0:
	.quad	289360691352306692              # 0x404040404040404
	.text
	.globl	allzeros_v32i8_and4
	.p2align	4, 0x90
	.type	allzeros_v32i8_and4,@function
allzeros_v32i8_and4:                    # @allzeros_v32i8_and4
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI53_0(%rip), %ymm1  # ymm1 = [289360691352306692,289360691352306692,289360691352306692,289360691352306692]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end53:
	.size	allzeros_v32i8_and4, .Lfunc_end53-allzeros_v32i8_and4
	.cfi_endproc
                                        # -- End function
	.globl	allones_v64i8_and4              # -- Begin function allones_v64i8_and4
	.p2align	4, 0x90
	.type	allones_v64i8_and4,@function
allones_v64i8_and4:                     # @allones_v64i8_and4
	.cfi_startproc
# %bb.0:
	vextracti64x4	$1, %zmm0, %ymm1
	vpand	%ymm0, %ymm1, %ymm0
	vpsllw	$5, %ymm0, %ymm0
	vpmovmskb	%ymm0, %eax
	cmpl	$-1, %eax
	sete	%al
	vzeroupper
	retq
.Lfunc_end54:
	.size	allones_v64i8_and4, .Lfunc_end54-allones_v64i8_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0                          # -- Begin function allzeros_v64i8_and4
.LCPI55_0:
	.zero	64,4
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI55_1:
	.zero	4,4
	.text
	.globl	allzeros_v64i8_and4
	.p2align	4, 0x90
	.type	allzeros_v64i8_and4,@function
allzeros_v64i8_and4:                    # @allzeros_v64i8_and4
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI55_1(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end55:
	.size	allzeros_v64i8_and4, .Lfunc_end55-allzeros_v64i8_and4
	.cfi_endproc
                                        # -- End function
	.globl	allones_v8i16_and4              # -- Begin function allones_v8i16_and4
	.p2align	4, 0x90
	.type	allones_v8i16_and4,@function
allones_v8i16_and4:                     # @allones_v8i16_and4
	.cfi_startproc
# %bb.0:
	vpsllw	$13, %xmm0, %xmm0
	vpsraw	$15, %xmm0, %xmm0
	vpmovsxwq	%xmm0, %zmm0
	vptestmq	%zmm0, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end56:
	.size	allones_v8i16_and4, .Lfunc_end56-allones_v8i16_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v8i16_and4
.LCPI57_0:
	.quad	1125917086973956                # 0x4000400040004
	.quad	1125917086973956                # 0x4000400040004
	.text
	.globl	allzeros_v8i16_and4
	.p2align	4, 0x90
	.type	allzeros_v8i16_and4,@function
allzeros_v8i16_and4:                    # @allzeros_v8i16_and4
	.cfi_startproc
# %bb.0:
	vptest	.LCPI57_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end57:
	.size	allzeros_v8i16_and4, .Lfunc_end57-allzeros_v8i16_and4
	.cfi_endproc
                                        # -- End function
	.globl	allones_v16i16_and4             # -- Begin function allones_v16i16_and4
	.p2align	4, 0x90
	.type	allones_v16i16_and4,@function
allones_v16i16_and4:                    # @allones_v16i16_and4
	.cfi_startproc
# %bb.0:
	vpsllw	$13, %ymm0, %ymm0
	vpsraw	$15, %ymm0, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kortestw	%k0, %k0
	setb	%al
	vzeroupper
	retq
.Lfunc_end58:
	.size	allones_v16i16_and4, .Lfunc_end58-allones_v16i16_and4
	.cfi_endproc
                                        # -- End function
	.globl	allones_v32i16_and4             # -- Begin function allones_v32i16_and4
	.p2align	4, 0x90
	.type	allones_v32i16_and4,@function
allones_v32i16_and4:                    # @allones_v32i16_and4
	.cfi_startproc
# %bb.0:
	vpsllw	$13, %ymm0, %ymm1
	vpsraw	$15, %ymm1, %ymm1
	vpmovsxwd	%ymm1, %zmm1
	vptestmd	%zmm1, %zmm1, %k0
	kmovw	%k0, %eax
	vextracti64x4	$1, %zmm0, %ymm0
	vpsllw	$13, %ymm0, %ymm0
	vpsraw	$15, %ymm0, %ymm0
	vpmovsxwd	%ymm0, %zmm0
	vptestmd	%zmm0, %zmm0, %k0
	kmovw	%k0, %ecx
	andl	%eax, %ecx
	cmpl	$65535, %ecx                    # imm = 0xFFFF
	sete	%al
	vzeroupper
	retq
.Lfunc_end59:
	.size	allones_v32i16_and4, .Lfunc_end59-allones_v32i16_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata,"a",@progbits
	.p2align	6, 0x0                          # -- Begin function allzeros_v32i16_and4
.LCPI60_0:
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.short	4                               # 0x4
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0
.LCPI60_1:
	.short	4                               # 0x4
	.short	4                               # 0x4
	.text
	.globl	allzeros_v32i16_and4
	.p2align	4, 0x90
	.type	allzeros_v32i16_and4,@function
allzeros_v32i16_and4:                   # @allzeros_v32i16_and4
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI60_1(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end60:
	.size	allzeros_v32i16_and4, .Lfunc_end60-allzeros_v32i16_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v16i16_and4
.LCPI61_0:
	.quad	1125917086973956                # 0x4000400040004
	.text
	.globl	allzeros_v16i16_and4
	.p2align	4, 0x90
	.type	allzeros_v16i16_and4,@function
allzeros_v16i16_and4:                   # @allzeros_v16i16_and4
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI61_0(%rip), %ymm1  # ymm1 = [1125917086973956,1125917086973956,1125917086973956,1125917086973956]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end61:
	.size	allzeros_v16i16_and4, .Lfunc_end61-allzeros_v16i16_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allones_v4i32_and4
.LCPI62_0:
	.long	4                               # 0x4
	.text
	.globl	allones_v4i32_and4
	.p2align	4, 0x90
	.type	allones_v4i32_and4,@function
allones_v4i32_and4:                     # @allones_v4i32_and4
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vptestnmd	.LCPI62_0(%rip){1to16}, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$15, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end62:
	.size	allones_v4i32_and4, .Lfunc_end62-allones_v4i32_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v4i32_and4
.LCPI63_0:
	.quad	17179869188                     # 0x400000004
	.quad	17179869188                     # 0x400000004
	.text
	.globl	allzeros_v4i32_and4
	.p2align	4, 0x90
	.type	allzeros_v4i32_and4,@function
allzeros_v4i32_and4:                    # @allzeros_v4i32_and4
	.cfi_startproc
# %bb.0:
	vptest	.LCPI63_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end63:
	.size	allzeros_v4i32_and4, .Lfunc_end63-allzeros_v4i32_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allones_v8i32_and4
.LCPI64_0:
	.long	4                               # 0x4
	.text
	.globl	allones_v8i32_and4
	.p2align	4, 0x90
	.type	allones_v8i32_and4,@function
allones_v8i32_and4:                     # @allones_v8i32_and4
	.cfi_startproc
# %bb.0:
                                        # kill: def $ymm0 killed $ymm0 def $zmm0
	vptestmd	.LCPI64_0(%rip){1to16}, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end64:
	.size	allones_v8i32_and4, .Lfunc_end64-allones_v8i32_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v8i32_and4
.LCPI65_0:
	.quad	17179869188                     # 0x400000004
	.text
	.globl	allzeros_v8i32_and4
	.p2align	4, 0x90
	.type	allzeros_v8i32_and4,@function
allzeros_v8i32_and4:                    # @allzeros_v8i32_and4
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI65_0(%rip), %ymm1  # ymm1 = [17179869188,17179869188,17179869188,17179869188]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end65:
	.size	allzeros_v8i32_and4, .Lfunc_end65-allzeros_v8i32_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allones_v16i32_and4
.LCPI66_0:
	.long	4                               # 0x4
	.text
	.globl	allones_v16i32_and4
	.p2align	4, 0x90
	.type	allones_v16i32_and4,@function
allones_v16i32_and4:                    # @allones_v16i32_and4
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI66_0(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	setb	%al
	vzeroupper
	retq
.Lfunc_end66:
	.size	allones_v16i32_and4, .Lfunc_end66-allones_v16i32_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst4,"aM",@progbits,4
	.p2align	2, 0x0                          # -- Begin function allzeros_v16i32_and4
.LCPI67_0:
	.long	4                               # 0x4
	.text
	.globl	allzeros_v16i32_and4
	.p2align	4, 0x90
	.type	allzeros_v16i32_and4,@function
allzeros_v16i32_and4:                   # @allzeros_v16i32_and4
	.cfi_startproc
# %bb.0:
	vptestmd	.LCPI67_0(%rip){1to16}, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end67:
	.size	allzeros_v16i32_and4, .Lfunc_end67-allzeros_v16i32_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allones_v2i64_and4
.LCPI68_0:
	.quad	4                               # 0x4
	.quad	4                               # 0x4
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0
.LCPI68_1:
	.quad	4                               # 0x4
	.text
	.globl	allones_v2i64_and4
	.p2align	4, 0x90
	.type	allones_v2i64_and4,@function
allones_v2i64_and4:                     # @allones_v2i64_and4
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vpbroadcastq	.LCPI68_1(%rip), %xmm1  # xmm1 = [4,4]
	vptestnmq	%zmm1, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end68:
	.size	allones_v2i64_and4, .Lfunc_end68-allones_v2i64_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst16,"aM",@progbits,16
	.p2align	4, 0x0                          # -- Begin function allzeros_v2i64_and4
.LCPI69_0:
	.quad	4                               # 0x4
	.quad	4                               # 0x4
	.text
	.globl	allzeros_v2i64_and4
	.p2align	4, 0x90
	.type	allzeros_v2i64_and4,@function
allzeros_v2i64_and4:                    # @allzeros_v2i64_and4
	.cfi_startproc
# %bb.0:
	vptest	.LCPI69_0(%rip), %xmm0
	sete	%al
	retq
.Lfunc_end69:
	.size	allzeros_v2i64_and4, .Lfunc_end69-allzeros_v2i64_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allones_v4i64_and4
.LCPI70_0:
	.quad	4                               # 0x4
	.text
	.globl	allones_v4i64_and4
	.p2align	4, 0x90
	.type	allones_v4i64_and4,@function
allones_v4i64_and4:                     # @allones_v4i64_and4
	.cfi_startproc
# %bb.0:
                                        # kill: def $ymm0 killed $ymm0 def $zmm0
	vptestnmq	.LCPI70_0(%rip){1to8}, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$15, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end70:
	.size	allones_v4i64_and4, .Lfunc_end70-allones_v4i64_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v4i64_and4
.LCPI71_0:
	.quad	4                               # 0x4
	.text
	.globl	allzeros_v4i64_and4
	.p2align	4, 0x90
	.type	allzeros_v4i64_and4,@function
allzeros_v4i64_and4:                    # @allzeros_v4i64_and4
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI71_0(%rip), %ymm1  # ymm1 = [4,4,4,4]
	vptest	%ymm1, %ymm0
	sete	%al
	vzeroupper
	retq
.Lfunc_end71:
	.size	allzeros_v4i64_and4, .Lfunc_end71-allzeros_v4i64_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allones_v8i64_and4
.LCPI72_0:
	.quad	4                               # 0x4
	.text
	.globl	allones_v8i64_and4
	.p2align	4, 0x90
	.type	allones_v8i64_and4,@function
allones_v8i64_and4:                     # @allones_v8i64_and4
	.cfi_startproc
# %bb.0:
	vptestmq	.LCPI72_0(%rip){1to8}, %zmm0, %k0
	kmovw	%k0, %eax
	cmpb	$-1, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end72:
	.size	allones_v8i64_and4, .Lfunc_end72-allones_v8i64_and4
	.cfi_endproc
                                        # -- End function
	.section	.rodata.cst8,"aM",@progbits,8
	.p2align	3, 0x0                          # -- Begin function allzeros_v8i64_and4
.LCPI73_0:
	.quad	4                               # 0x4
	.text
	.globl	allzeros_v8i64_and4
	.p2align	4, 0x90
	.type	allzeros_v8i64_and4,@function
allzeros_v8i64_and4:                    # @allzeros_v8i64_and4
	.cfi_startproc
# %bb.0:
	vpbroadcastq	.LCPI73_0(%rip), %zmm1  # zmm1 = [4,4,4,4,4,4,4,4]
	vptestmd	%zmm1, %zmm0, %k0
	kortestw	%k0, %k0
	sete	%al
	vzeroupper
	retq
.Lfunc_end73:
	.size	allzeros_v8i64_and4, .Lfunc_end73-allzeros_v8i64_and4
	.cfi_endproc
                                        # -- End function
	.globl	allzeros_v8f32_nnan             # -- Begin function allzeros_v8f32_nnan
	.p2align	4, 0x90
	.type	allzeros_v8f32_nnan,@function
allzeros_v8f32_nnan:                    # @allzeros_v8f32_nnan
	.cfi_startproc
# %bb.0:
                                        # kill: def $ymm0 killed $ymm0 def $zmm0
	vxorps	%xmm1, %xmm1, %xmm1
	vcmpneqps	%zmm1, %zmm0, %k0
	kmovw	%k0, %eax
	testb	%al, %al
	setne	%al
	vzeroupper
	retq
.Lfunc_end74:
	.size	allzeros_v8f32_nnan, .Lfunc_end74-allzeros_v8f32_nnan
	.cfi_endproc
                                        # -- End function
	.globl	movmskpd                        # -- Begin function movmskpd
	.p2align	4, 0x90
	.type	movmskpd,@function
movmskpd:                               # @movmskpd
	.cfi_startproc
# %bb.0:
	vmovmskpd	%xmm0, %eax
	retq
.Lfunc_end75:
	.size	movmskpd, .Lfunc_end75-movmskpd
	.cfi_endproc
                                        # -- End function
	.globl	movmskps                        # -- Begin function movmskps
	.p2align	4, 0x90
	.type	movmskps,@function
movmskps:                               # @movmskps
	.cfi_startproc
# %bb.0:
	vmovmskps	%xmm0, %eax
	retq
.Lfunc_end76:
	.size	movmskps, .Lfunc_end76-movmskps
	.cfi_endproc
                                        # -- End function
	.globl	movmskpd256                     # -- Begin function movmskpd256
	.p2align	4, 0x90
	.type	movmskpd256,@function
movmskpd256:                            # @movmskpd256
	.cfi_startproc
# %bb.0:
	vmovmskpd	%ymm0, %eax
	vzeroupper
	retq
.Lfunc_end77:
	.size	movmskpd256, .Lfunc_end77-movmskpd256
	.cfi_endproc
                                        # -- End function
	.globl	movmskps256                     # -- Begin function movmskps256
	.p2align	4, 0x90
	.type	movmskps256,@function
movmskps256:                            # @movmskps256
	.cfi_startproc
# %bb.0:
	vmovmskps	%ymm0, %eax
	vzeroupper
	retq
.Lfunc_end78:
	.size	movmskps256, .Lfunc_end78-movmskps256
	.cfi_endproc
                                        # -- End function
	.globl	movmskb                         # -- Begin function movmskb
	.p2align	4, 0x90
	.type	movmskb,@function
movmskb:                                # @movmskb
	.cfi_startproc
# %bb.0:
	vpmovmskb	%xmm0, %eax
	retq
.Lfunc_end79:
	.size	movmskb, .Lfunc_end79-movmskb
	.cfi_endproc
                                        # -- End function
	.globl	movmskb256                      # -- Begin function movmskb256
	.p2align	4, 0x90
	.type	movmskb256,@function
movmskb256:                             # @movmskb256
	.cfi_startproc
# %bb.0:
	vpmovmskb	%ymm0, %eax
	vzeroupper
	retq
.Lfunc_end80:
	.size	movmskb256, .Lfunc_end80-movmskb256
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v16i8                    # -- Begin function movmsk_v16i8
	.p2align	4, 0x90
	.type	movmsk_v16i8,@function
movmsk_v16i8:                           # @movmsk_v16i8
	.cfi_startproc
# %bb.0:
	vpcmpeqb	%xmm1, %xmm0, %xmm0
	vpmovmskb	%xmm0, %eax
	movl	%eax, %ecx
	shrl	$15, %ecx
	movl	%eax, %edx
	shrl	$8, %edx
	andl	$1, %edx
	andl	$8, %eax
	shrl	$3, %eax
	xorl	%edx, %eax
	andl	%ecx, %eax
                                        # kill: def $al killed $al killed $eax
	retq
.Lfunc_end81:
	.size	movmsk_v16i8, .Lfunc_end81-movmsk_v16i8
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v8i16                    # -- Begin function movmsk_v8i16
	.p2align	4, 0x90
	.type	movmsk_v8i16,@function
movmsk_v8i16:                           # @movmsk_v8i16
	.cfi_startproc
# %bb.0:
	vpcmpgtw	%xmm1, %xmm0, %xmm0
	vpmovsxwq	%xmm0, %zmm0
	vptestnmq	%zmm0, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$-109, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end82:
	.size	movmsk_v8i16, .Lfunc_end82-movmsk_v8i16
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v4i32                    # -- Begin function movmsk_v4i32
	.p2align	4, 0x90
	.type	movmsk_v4i32,@function
movmsk_v4i32:                           # @movmsk_v4i32
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vpcmpgtd	%zmm0, %zmm1, %k0
	kshiftrw	$3, %k0, %k1
	kmovw	%k1, %ecx
	kshiftrw	$2, %k0, %k0
	kmovw	%k0, %eax
	xorb	%cl, %al
                                        # kill: def $al killed $al killed $eax
	vzeroupper
	retq
.Lfunc_end83:
	.size	movmsk_v4i32, .Lfunc_end83-movmsk_v4i32
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_and_v2i64                # -- Begin function movmsk_and_v2i64
	.p2align	4, 0x90
	.type	movmsk_and_v2i64,@function
movmsk_and_v2i64:                       # @movmsk_and_v2i64
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vpcmpeqq	%zmm1, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end84:
	.size	movmsk_and_v2i64, .Lfunc_end84-movmsk_and_v2i64
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_or_v2i64                 # -- Begin function movmsk_or_v2i64
	.p2align	4, 0x90
	.type	movmsk_or_v2i64,@function
movmsk_or_v2i64:                        # @movmsk_or_v2i64
	.cfi_startproc
# %bb.0:
	vpxor	%xmm1, %xmm0, %xmm0
	vptest	%xmm0, %xmm0
	setne	%al
	retq
.Lfunc_end85:
	.size	movmsk_or_v2i64, .Lfunc_end85-movmsk_or_v2i64
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v4f32                    # -- Begin function movmsk_v4f32
	.p2align	4, 0x90
	.type	movmsk_v4f32,@function
movmsk_v4f32:                           # @movmsk_v4f32
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmpeq_uqps	%zmm1, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$14, %al
	setne	%al
	vzeroupper
	retq
.Lfunc_end86:
	.size	movmsk_v4f32, .Lfunc_end86-movmsk_v4f32
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_and_v2f64                # -- Begin function movmsk_and_v2f64
	.p2align	4, 0x90
	.type	movmsk_and_v2f64,@function
movmsk_and_v2f64:                       # @movmsk_and_v2f64
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmplepd	%zmm0, %zmm1, %k0
	knotw	%k0, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	sete	%al
	vzeroupper
	retq
.Lfunc_end87:
	.size	movmsk_and_v2f64, .Lfunc_end87-movmsk_and_v2f64
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_or_v2f64                 # -- Begin function movmsk_or_v2f64
	.p2align	4, 0x90
	.type	movmsk_or_v2f64,@function
movmsk_or_v2f64:                        # @movmsk_or_v2f64
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmplepd	%zmm0, %zmm1, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	setne	%al
	vzeroupper
	retq
.Lfunc_end88:
	.size	movmsk_or_v2f64, .Lfunc_end88-movmsk_or_v2f64
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v16i8_var                # -- Begin function movmsk_v16i8_var
	.p2align	4, 0x90
	.type	movmsk_v16i8_var,@function
movmsk_v16i8_var:                       # @movmsk_v16i8_var
	.cfi_startproc
# %bb.0:
	vpcmpeqb	%xmm1, %xmm0, %xmm0
	vpmovmskb	%xmm0, %eax
	btl	%edi, %eax
	setb	%al
	retq
.Lfunc_end89:
	.size	movmsk_v16i8_var, .Lfunc_end89-movmsk_v16i8_var
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v8i16_var                # -- Begin function movmsk_v8i16_var
	.p2align	4, 0x90
	.type	movmsk_v8i16_var,@function
movmsk_v8i16_var:                       # @movmsk_v8i16_var
	.cfi_startproc
# %bb.0:
                                        # kill: def $edi killed $edi def $rdi
	vpcmpgtw	%xmm1, %xmm0, %xmm0
	vpmovsxwq	%xmm0, %zmm0
	vptestmq	%zmm0, %zmm0, %k1
	vpternlogd	$255, %zmm0, %zmm0, %zmm0 {%k1} {z}
	vpmovdw	%zmm0, %ymm0
	vmovdqa	%xmm0, -24(%rsp)
	andl	$7, %edi
	movzbl	-24(%rsp,%rdi,2), %eax
	vzeroupper
	retq
.Lfunc_end90:
	.size	movmsk_v8i16_var, .Lfunc_end90-movmsk_v8i16_var
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v4i32_var                # -- Begin function movmsk_v4i32_var
	.p2align	4, 0x90
	.type	movmsk_v4i32_var,@function
movmsk_v4i32_var:                       # @movmsk_v4i32_var
	.cfi_startproc
# %bb.0:
                                        # kill: def $edi killed $edi def $rdi
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vpcmpgtd	%zmm0, %zmm1, %k1
	vpternlogd	$255, %zmm0, %zmm0, %zmm0 {%k1} {z}
	vmovdqa	%xmm0, -24(%rsp)
	andl	$3, %edi
	movzbl	-24(%rsp,%rdi,4), %eax
	vzeroupper
	retq
.Lfunc_end91:
	.size	movmsk_v4i32_var, .Lfunc_end91-movmsk_v4i32_var
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v2i64_var                # -- Begin function movmsk_v2i64_var
	.p2align	4, 0x90
	.type	movmsk_v2i64_var,@function
movmsk_v2i64_var:                       # @movmsk_v2i64_var
	.cfi_startproc
# %bb.0:
                                        # kill: def $edi killed $edi def $rdi
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vpcmpneqq	%zmm1, %zmm0, %k1
	vpternlogq	$255, %zmm0, %zmm0, %zmm0 {%k1} {z}
	vmovdqa	%xmm0, -24(%rsp)
	andl	$1, %edi
	movzbl	-24(%rsp,%rdi,8), %eax
	vzeroupper
	retq
.Lfunc_end92:
	.size	movmsk_v2i64_var, .Lfunc_end92-movmsk_v2i64_var
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v4f32_var                # -- Begin function movmsk_v4f32_var
	.p2align	4, 0x90
	.type	movmsk_v4f32_var,@function
movmsk_v4f32_var:                       # @movmsk_v4f32_var
	.cfi_startproc
# %bb.0:
                                        # kill: def $edi killed $edi def $rdi
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmpeq_uqps	%zmm1, %zmm0, %k1
	vpternlogd	$255, %zmm0, %zmm0, %zmm0 {%k1} {z}
	vmovdqa	%xmm0, -24(%rsp)
	andl	$3, %edi
	movzbl	-24(%rsp,%rdi,4), %eax
	vzeroupper
	retq
.Lfunc_end93:
	.size	movmsk_v4f32_var, .Lfunc_end93-movmsk_v4f32_var
	.cfi_endproc
                                        # -- End function
	.globl	movmsk_v2f64_var                # -- Begin function movmsk_v2f64_var
	.p2align	4, 0x90
	.type	movmsk_v2f64_var,@function
movmsk_v2f64_var:                       # @movmsk_v2f64_var
	.cfi_startproc
# %bb.0:
                                        # kill: def $edi killed $edi def $rdi
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmplepd	%zmm0, %zmm1, %k1
	vpternlogq	$255, %zmm0, %zmm0, %zmm0 {%k1} {z}
	vmovdqa	%xmm0, -24(%rsp)
	andl	$1, %edi
	movzbl	-24(%rsp,%rdi,8), %eax
	vzeroupper
	retq
.Lfunc_end94:
	.size	movmsk_v2f64_var, .Lfunc_end94-movmsk_v2f64_var
	.cfi_endproc
                                        # -- End function
	.globl	PR39665_c_ray                   # -- Begin function PR39665_c_ray
	.p2align	4, 0x90
	.type	PR39665_c_ray,@function
PR39665_c_ray:                          # @PR39665_c_ray
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmpltpd	%zmm0, %zmm1, %k0
	kmovw	%k0, %ecx
	kmovw	%k0, %eax
	testb	$2, %al
	movl	$42, %eax
	movl	$99, %edx
	cmovel	%edx, %eax
	testb	$1, %cl
	cmovel	%edx, %eax
	vzeroupper
	retq
.Lfunc_end95:
	.size	PR39665_c_ray, .Lfunc_end95-PR39665_c_ray
	.cfi_endproc
                                        # -- End function
	.globl	PR39665_c_ray_opt               # -- Begin function PR39665_c_ray_opt
	.p2align	4, 0x90
	.type	PR39665_c_ray_opt,@function
PR39665_c_ray_opt:                      # @PR39665_c_ray_opt
	.cfi_startproc
# %bb.0:
                                        # kill: def $xmm1 killed $xmm1 def $zmm1
                                        # kill: def $xmm0 killed $xmm0 def $zmm0
	vcmpltpd	%zmm0, %zmm1, %k0
	knotw	%k0, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	movl	$42, %ecx
	movl	$99, %eax
	cmovel	%ecx, %eax
	vzeroupper
	retq
.Lfunc_end96:
	.size	PR39665_c_ray_opt, .Lfunc_end96-PR39665_c_ray_opt
	.cfi_endproc
                                        # -- End function
	.globl	pr67287                         # -- Begin function pr67287
	.p2align	4, 0x90
	.type	pr67287,@function
pr67287:                                # @pr67287
	.cfi_startproc
# %bb.0:                                # %entry
	vpxor	%xmm1, %xmm1, %xmm1
	vpblendd	$10, %xmm1, %xmm0, %xmm0        # xmm0 = xmm0[0],xmm1[1],xmm0[2],xmm1[3]
	vptestnmq	%zmm0, %zmm0, %k0
	kmovw	%k0, %eax
	testb	$3, %al
	jne	.LBB97_2
# %bb.1:                                # %entry
	testb	$1, %al
	jne	.LBB97_2
# %bb.3:                                # %middle.block
	xorl	%eax, %eax
	vzeroupper
	retq
.LBB97_2:
	movw	$0, 0
	xorl	%eax, %eax
	vzeroupper
	retq
.Lfunc_end97:
	.size	pr67287, .Lfunc_end97-pr67287
	.cfi_endproc
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
