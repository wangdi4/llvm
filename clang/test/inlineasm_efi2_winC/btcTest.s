// RUN: llvm-mc -x86-asm-syntax=intel -output-asm-variant=1 < %s | FileCheck %s

		btc	ax,4
//CHECK:	btc	ax, 4
		btc	ax,dx
//CHECK:	btc	ax, dx
		btc	eax,4
//CHECK:	btc	eax, 4
		btc	eax,ecx
//CHECK:	btc	eax, ecx
		btc	rax,4
//CHECK:	btc	rax, 4
		btc	rax,rcx
//CHECK:	btc	rax, rcx
		btc	QWORD PTR test_mem,4
//CHECK:	btc	qword ptr [test_mem], 4
		btc	DWORD PTR test_mem,4
//CHECK:	btc	dword ptr [test_mem], 4
		btc	WORD PTR test_mem,4
//CHECK:	btc	word ptr [test_mem], 4
		btc	WORD PTR test_mem,ax
//CHECK:	btc	word ptr [test_mem], ax
		btc	DWORD PTR test_mem,eax
//CHECK:	btc	dword ptr [test_mem], eax
		btc	QWORD PTR test_mem,rax
//CHECK:	btc	qword ptr [test_mem], rax
		btc	XMMWORD PTR test_mem,4
//CHECK:	btc	dword ptr [test_mem], 4
		btc	TBYTE PTR test_mem,4
//CHECK:	btc	dword ptr [test_mem], 4
		btc	BYTE PTR test_mem,4
//CHECK:	btc	dword ptr [test_mem], 4
		btc	XMMWORD PTR test_mem,ax
//CHECK:	btc	word ptr [test_mem], ax
		btc	QWORD PTR test_mem,ax
//CHECK:	btc	word ptr [test_mem], ax
		btc	TBYTE PTR test_mem,ax
//CHECK:	btc	word ptr [test_mem], ax
		btc	DWORD PTR test_mem,ax
//CHECK:	btc	word ptr [test_mem], ax
		btc	BYTE PTR test_mem,ax
//CHECK:	btc	word ptr [test_mem], ax
		btc	XMMWORD PTR test_mem,eax
//CHECK:	btc	dword ptr [test_mem], eax
		btc	QWORD PTR test_mem,eax
//CHECK:	btc	dword ptr [test_mem], eax
		btc	TBYTE PTR test_mem,eax
//CHECK:	btc	dword ptr [test_mem], eax
		btc	WORD PTR test_mem,eax
//CHECK:	btc	dword ptr [test_mem], eax
		btc	BYTE PTR test_mem,eax
//CHECK:	btc	dword ptr [test_mem], eax
		btc	XMMWORD PTR test_mem,rax
//CHECK:	btc	qword ptr [test_mem], rax
		btc	TBYTE PTR test_mem,rax
//CHECK:	btc	qword ptr [test_mem], rax
		btc	DWORD PTR test_mem,rax
//CHECK:	btc	qword ptr [test_mem], rax
		btc	WORD PTR test_mem,rax
//CHECK:	btc	qword ptr [test_mem], rax
		btc	BYTE PTR test_mem,rax
//CHECK:	btc	qword ptr [test_mem], rax
