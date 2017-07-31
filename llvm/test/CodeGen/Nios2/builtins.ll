; RUN: llc -march=nios2 -O0 < %s | FileCheck %s

declare i32 @llvm.nios2.ldbio(i8*) #1
declare i32 @llvm.nios2.ldbuio(i8*) #1
declare i32 @llvm.nios2.ldhio(i8*) #1
declare i32 @llvm.nios2.ldhuio(i8*) #1
declare i32 @llvm.nios2.ldwio(i8*) #1
declare void @llvm.nios2.stbio(i8*, i32) #2
declare void @llvm.nios2.sthio(i8*, i32) #2
declare void @llvm.nios2.stwio(i8*, i32) #2
declare void @llvm.nios2.sync() #3
declare i32 @llvm.nios2.rdctl(i32) #3
declare i32 @llvm.nios2.rdprs(i32, i32) #3
declare void @llvm.nios2.wrctl(i32, i32) #3
declare void @llvm.nios2.flushd(i8*) #3
declare void @llvm.nios2.flushda(i8*) #3

define i32 @builtins() {
entry:
; CHECK: builtins:
; CHECK:   ldbuio {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   ldhio  {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   ldhuio {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   ldwio  {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   stbio  {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   sthio  {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   stwio  {{r[0-9]+}}, {{[0-9]+}}({{r[0-9]+}})
; CHECK:   sync
; CHECK:   rdctl  {{r[0-9]+}}, {{r[0-9]+}}
; CHECK:   rdprs  {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
; CHECK:   wrctl  {{r[0-9]+}}, {{r[0-9]+}}
; CHECK:   flushd  {{[0-9]+}}({{r[0-9]+}})
; CHECK:   flushda {{[0-9]+}}({{r[0-9]+}})
  %p = alloca i8*, align 4
  %k = alloca i32, align 4
  store i8* null, i8** %p, align 4
  store volatile i32 0, i32* %k, align 4
  %0 = load i8*, i8** %p, align 4
  %1 = call i32 @llvm.nios2.ldbio(i8* %0)
  store volatile i32 %1, i32* %k, align 4
  %2 = load i8*, i8** %p, align 4
  %3 = call i32 @llvm.nios2.ldbuio(i8* %2)
  store volatile i32 %3, i32* %k, align 4
  %4 = load i8*, i8** %p, align 4
  %5 = call i32 @llvm.nios2.ldhio(i8* %4)
  store volatile i32 %5, i32* %k, align 4
  %6 = load i8*, i8** %p, align 4
  %7 = call i32 @llvm.nios2.ldhuio(i8* %6)
  store volatile i32 %7, i32* %k, align 4
  %8 = load i8*, i8** %p, align 4
  %9 = call i32 @llvm.nios2.ldwio(i8* %8)
  store volatile i32 %9, i32* %k, align 4
  %10 = load i8*, i8** %p, align 4
  call void @llvm.nios2.stbio(i8* %10, i32 0)
  %11 = load i8*, i8** %p, align 4
  call void @llvm.nios2.sthio(i8* %11, i32 0)
  %12 = load i8*, i8** %p, align 4
  call void @llvm.nios2.stwio(i8* %12, i32 0)
  call void @llvm.nios2.sync()
  %13 = call i32 @llvm.nios2.rdctl(i32 0)
  store volatile i32 %13, i32* %k, align 4
  %14 = call i32 @llvm.nios2.rdprs(i32 0, i32 0)
  store volatile i32 %14, i32* %k, align 4
  call void @llvm.nios2.wrctl(i32 0, i32 0)
  %15 = load i8*, i8** %p, align 4
  call void @llvm.nios2.flushd(i8* %15)
  %16 = load i8*, i8** %p, align 4
  call void @llvm.nios2.flushda(i8* %16)
  %17 = load volatile i32, i32* %k, align 4
  ret i32 %17
}
