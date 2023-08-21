// RUN: %clang_cc1 -regcall4 -emit-llvm %s -o - -ffreestanding -triple=i386-pc-win32 -fintel-compatibility       | FileCheck %s --check-prefixes=Win32
// RUN: %clang_cc1 -regcall4 -emit-llvm %s -o - -ffreestanding -triple=x86_64-pc-win32  -fintel-compatibility   | FileCheck %s --check-prefixes=Win64
// RUN: %clang_cc1 -regcall4 -emit-llvm %s -o - -ffreestanding -triple=i386-pc-linux-gnu -fintel-compatibility  | FileCheck %s --check-prefixes=Lin32
// RUN: %clang_cc1 -regcall4 -emit-llvm %s -o - -ffreestanding -triple=x86_64-pc-linux-gnu -fintel-compatibility | FileCheck %s --check-prefixes=Lin64

#include <xmmintrin.h>

void __regcall v1(int a, int b) {}
// Lin32: define dso_local x86_regcallcc void @__regcall4__v1(i32 inreg noundef %a, i32 inreg noundef %b)
// Win32: define dso_local x86_regcallcc void @"\01__regcall4__v1"(i32 inreg noundef %a, i32 inreg noundef %b)
// Lin64: define dso_local x86_regcallcc void @__regcall4__v1(i32 noundef %a, i32 noundef %b)
// Win64: define dso_local x86_regcallcc void @"\01__regcall4__v1"(i32 noundef %a, i32 noundef %b)

void __attribute__((regcall)) v1b(int a, int b) {}
// Lin32: define dso_local x86_regcallcc void @__regcall4__v1b(i32 inreg noundef %a, i32 inreg noundef %b)
// Win32: define dso_local x86_regcallcc void @"\01__regcall4__v1b"(i32 inreg noundef %a, i32 inreg noundef %b)
// Lin64: define dso_local x86_regcallcc void @__regcall4__v1b(i32 noundef %a, i32 noundef %b)
// Win64: define dso_local x86_regcallcc void @"\01__regcall4__v1b"(i32 noundef %a, i32 noundef %b)

void __regcall v2(char a, char b) {}
// Lin32: define dso_local x86_regcallcc void @__regcall4__v2(i8 inreg noundef signext %a, i8 inreg noundef signext %b)
// Win32: define dso_local x86_regcallcc void @"\01__regcall4__v2"(i8 inreg noundef signext %a, i8 inreg noundef signext %b)
// Lin64: define dso_local x86_regcallcc void @__regcall4__v2(i8 noundef signext %a, i8 noundef signext %b)
// Win64: define dso_local x86_regcallcc void @"\01__regcall4__v2"(i8 noundef %a, i8 noundef %b)

void __stdcall v3(char a, char b) {}
// Lin32: define dso_local x86_stdcallcc void @v3(i8 noundef signext %a, i8 noundef signext %b)
// Win32: define dso_local x86_stdcallcc void @"\01_v3@8"(i8 noundef signext %a, i8 noundef signext %b)
// Lin64: define dso_local void @v3(i8 noundef signext %a, i8 noundef signext %b)
// Win64: define dso_local void @v3(i8 noundef %a, i8 noundef %b)
void __fastcall v4(char a, char b) {}
// Lin32: define dso_local x86_fastcallcc void @v4(i8 inreg noundef signext %a, i8 inreg noundef signext %b)
// Win32: define dso_local x86_fastcallcc void @"\01@v4@8"(i8 inreg noundef signext %a, i8 inreg noundef signext %b)
// Lin64: define dso_local void @v4(i8 noundef signext %a, i8 noundef signext %b)
// Win64: define dso_local void @v4(i8 noundef %a, i8 noundef %b)
