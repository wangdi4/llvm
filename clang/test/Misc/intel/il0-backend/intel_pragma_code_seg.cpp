// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple=x86_64-apple-darwin -verify -x c++ -o - %s | FileCheck %s
//***INTEL: pragma code_seg test

// CHECK: target
void www();

// CHECK: define void @_Z1av() #0 {
// CHECK: define void @_Z1bv() #0 {
// CHECK: define void @_Z1cv() #0 section "#code#__TEXT, sect1~@~" {
// CHECK: define void @_Z1dv() #0 section "#code#__TEXT, sect2~@~www" {
// CHECK: define void @_Z1ev() #0 section "#code#__TEXT, sect43~@~www" {
// CHECK: define void @_Z1fv() #0 section "#code#__TEXT, sect2~@~www" {
// CHECK: define void @_Z1gv() #0 section "#code#__TEXT, sect2~@~www" {
// CHECK: define void @_Z1hv() #0 section "#code#__TEXT, sect2~@~www" {
// CHECK: define void @_Z5push1v() #0 section "#code#__TEXT, sect4~@~" {
// CHECK: define void @_Z4pop1v() #0 section "#code#__TEXT, sect43~@~www" {
// CHECK: define void @_Z4pop2v() #0 {

#pragma code_seg ; // expected-warning {{extra text after expected end of preprocessing directive}}
void a(){}
#pragma code_seg (
void b(){}
#pragma code_seg ("sect1" 
void c(){}
#pragma code_seg ("sect2", "www"
void d(){}
#pragma code_seg (push, "sect43", "www") dfewer // expected-warning {{extra text after expected end of preprocessing directive}}
void e(){}


#pragma code_seg (push, id1, "sect2", "www")
void f(){}
#pragma code_seg (push, id2, "sect3", "") // expected-warning {{invalid use of null string; pragma ignored}}
void g(){}
#pragma code_seg (pop, "") // expected-warning {{invalid use of null string; pragma ignored}}
void h(){}

#pragma code_seg (push, "sect4")
void push1(){}
#pragma code_seg (pop, id1)
void pop1(){}
#pragma code_seg (pop)
void pop2(){}
// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
  return (0);
}

