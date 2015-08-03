// RUN: llvm-mc -x86-asm-syntax=intel -output-asm-variant=1 < %s | FileCheck %s

		bts	ax,4
//CHECK:	bts	ax, 4
		bts	ax,dx
//CHECK:	bts	ax, dx
		bts	eax,4
//CHECK:	bts	eax, 4
		bts	eax,ecx
//CHECK:	bts	eax, ecx
		bts	rax,4
//CHECK:	bts	rax, 4
		bts	rax,rcx
//CHECK:	bts	rax, rcx
		bts	QWORD PTR test_mem,4
//CHECK:	bts	qword ptr [test_mem], 4
		bts	DWORD PTR test_mem,4
//CHECK:	bts	dword ptr [test_mem], 4
		bts	WORD PTR test_mem,4
//CHECK:	bts	word ptr [test_mem], 4
		bts	WORD PTR test_mem,ax
//CHECK:	bts	word ptr [test_mem], ax
		bts	DWORD PTR test_mem,eax
//CHECK:	bts	dword ptr [test_mem], eax
		bts	QWORD PTR test_mem,rax
//CHECK:	bts	qword ptr [test_mem], rax
		bts	XMMWORD PTR test_mem,4
//CHECK:	bts	dword ptr [test_mem], 4
		bts	TBYTE PTR test_mem,4
//CHECK:	bts	dword ptr [test_mem], 4
		bts	BYTE PTR test_mem,4
//CHECK:	bts	dword ptr [test_mem], 4
		bts	XMMWORD PTR test_mem,ax
//CHECK:	bts	word ptr [test_mem], ax
		bts	QWORD PTR test_mem,ax
//CHECK:	bts	word ptr [test_mem], ax
		bts	TBYTE PTR test_mem,ax
//CHECK:	bts	word ptr [test_mem], ax
		bts	DWORD PTR test_mem,ax
//CHECK:	bts	word ptr [test_mem], ax
		bts	BYTE PTR test_mem,ax
//CHECK:	bts	word ptr [test_mem], ax
		bts	XMMWORD PTR test_mem,eax
//CHECK:	bts	dword ptr [test_mem], eax
		bts	QWORD PTR test_mem,eax
//CHECK:	bts	dword ptr [test_mem], eax
		bts	TBYTE PTR test_mem,eax
//CHECK:	bts	dword ptr [test_mem], eax
		bts	WORD PTR test_mem,eax
//CHECK:	bts	dword ptr [test_mem], eax
		bts	BYTE PTR test_mem,eax
//CHECK:	bts	dword ptr [test_mem], eax
		bts	XMMWORD PTR test_mem,rax
//CHECK:	bts	qword ptr [test_mem], rax
		bts	TBYTE PTR test_mem,rax
//CHECK:	bts	qword ptr [test_mem], rax
		bts	DWORD PTR test_mem,rax
//CHECK:	bts	qword ptr [test_mem], rax
		bts	WORD PTR test_mem,rax
//CHECK:	bts	qword ptr [test_mem], rax
		bts	BYTE PTR test_mem,rax
//CHECK:	bts	qword ptr [test_mem], rax
