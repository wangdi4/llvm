//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -o %t %s

//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:foo:depth}
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:bar:width}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:4,5}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:5,4}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:1}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:2}
//CHECK: [[ANN11:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,5}{merge:bar:width}

//CHECK: @llvm.global.annotations

__attribute__((ihc_component))
void foo_two() {
  // 1.
  static int __attribute__((merge("foo","depth"))) var_one;
  static int __attribute__((merge("bar","width"))) var_two;

  // 2. 3.
  static int __attribute__((bank_bits(4,5))) var_three;
  static int __attribute__((numbanks(4),bank_bits(4,5))) var_four;
  static int __attribute__((bank_bits(4,5),numbanks(4))) var_five;
  static int __attribute__((bank_bits(5,4),numbanks(4))) var_six;

  // 7.
  static int __attribute__((register)) var_nine;

  // 8.
  static int __attribute__((__memory__)) var_ten;

  // 9.
  static int __attribute__((__bankwidth__(4))) var_eleven;

  // 10.
  static int __attribute__((singlepump)) var_twelve;

  // 11.
  static int __attribute__((doublepump)) var_thirteen;
}

// 1.
static int __attribute__((merge("foo","depth"))) evar_one;
static int __attribute__((merge("bar","width"))) evar_two;

// 2. 3.
static int __attribute__((bank_bits(4,5))) evar_three;
static int __attribute__((numbanks(4),bank_bits(4,5))) evar_four;
static int __attribute__((bank_bits(4,5),numbanks(4))) evar_five;
static int __attribute__((bank_bits(5,4),numbanks(4))) evar_six;

// 4. 5. 6.

// 7.
static int __attribute__((register)) evar_nine;

// 8.
static int __attribute__((__memory__)) evar_ten;

// 9.
static int __attribute__((__bankwidth__(4))) evar_eleven;

// 10.
static int __attribute__((singlepump)) evar_twelve;

// 11.
static int __attribute__((doublepump)) evar_thirteen;

__attribute__((ihc_component))
int foo_one() {
  return evar_one+evar_two+evar_three+evar_four+evar_five+
         evar_six+evar_nine+evar_ten+evar_eleven+evar_twelve+
         evar_thirteen;
}

static int __attribute__((merge("bar", "width"))) afoo[5];
__attribute__((ihc_component))
void width_manual(int *rdata) {
  static int __attribute__((merge("bar", "width"))) cfoo[5];
  *rdata = 0;
  *rdata += afoo[2];
  *rdata += cfoo[2];
}
