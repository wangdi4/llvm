// RUN: llvm-mc -x86-asm-syntax=intel -output-asm-variant=1 < %s | FileCheck %s

		bt	ax,4
//CHECK:	bt	ax, 4
		bt	ax,dx
//CHECK:	bt	ax, dx
		bt	eax,4
//CHECK:	bt	eax, 4
		bt	eax,ecx
//CHECK:	bt	eax, ecx
		bt	rax,4
//CHECK:	bt	rax, 4
		bt	rax,rcx
//CHECK:	bt	rax, rcx
		bt	QWORD PTR test_mem,4
//CHECK:	bt	qword ptr [test_mem], 4
		bt	DWORD PTR test_mem,4
//CHECK:	bt	dword ptr [test_mem], 4
		bt	WORD PTR test_mem,4
//CHECK:	bt	word ptr [test_mem], 4
		bt	WORD PTR test_mem,ax
//CHECK:	bt	word ptr [test_mem], ax
		bt	DWORD PTR test_mem,eax
//CHECK:	bt	dword ptr [test_mem], eax
		bt	QWORD PTR test_mem,rax
//CHECK:	bt	qword ptr [test_mem], rax
		bt	XMMWORD PTR test_mem,4
//CHECK:	bt	dword ptr [test_mem], 4
		bt	TBYTE PTR test_mem,4
//CHECK:	bt	dword ptr [test_mem], 4
		bt	BYTE PTR test_mem,4
//CHECK:	bt	dword ptr [test_mem], 4
		bt	XMMWORD PTR test_mem,ax
//CHECK:	bt	word ptr [test_mem], ax
		bt	QWORD PTR test_mem,ax
//CHECK:	bt	word ptr [test_mem], ax
		bt	TBYTE PTR test_mem,ax
//CHECK:	bt	word ptr [test_mem], ax
		bt	DWORD PTR test_mem,ax
//CHECK:	bt	word ptr [test_mem], ax
		bt	BYTE PTR test_mem,ax
//CHECK:	bt	word ptr [test_mem], ax
		bt	XMMWORD PTR test_mem,eax
//CHECK:	bt	dword ptr [test_mem], eax
		bt	QWORD PTR test_mem,eax
//CHECK:	bt	dword ptr [test_mem], eax
		bt	TBYTE PTR test_mem,eax
//CHECK:	bt	dword ptr [test_mem], eax
		bt	WORD PTR test_mem,eax
//CHECK:	bt	dword ptr [test_mem], eax
		bt	BYTE PTR test_mem,eax
//CHECK:	bt	dword ptr [test_mem], eax
		bt	XMMWORD PTR test_mem,rax
//CHECK:	bt	qword ptr [test_mem], rax
		bt	TBYTE PTR test_mem,rax
//CHECK:	bt	qword ptr [test_mem], rax
		bt	DWORD PTR test_mem,rax
//CHECK:	bt	qword ptr [test_mem], rax
		bt	WORD PTR test_mem,rax
//CHECK:	bt	qword ptr [test_mem], rax
		bt	BYTE PTR test_mem,rax
//CHECK:	bt	qword ptr [test_mem], rax
