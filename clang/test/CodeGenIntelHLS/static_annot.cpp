//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -o %t %s

//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:foo:depth}
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{merge:bar:width}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:4,5}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numbanks:4}{bank_bits:5,4}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{numreadports:2}{numwriteports:3}
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{bankwidth:4}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:1}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4}{pump:2}
//CHECK: [[ANN11:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{sizeinfo:4,5}{merge:bar:width}

//CHECK: @llvm.global.annotations

__attribute__((ihc_component))
void foo_two() {
  // 1.
//CHECK-SAME: var_one{{.*}}[[ANN1]]{{.*}}i32 22
  static int __attribute__((merge("foo","depth"))) var_one;
//CHECK-SAME: var_two{{.*}}[[ANN2]]{{.*}}i32 24
  static int __attribute__((merge("bar","width"))) var_two;

  // 2. 3.
//CHECK-SAME: var_three{{.*}}[[ANN3]]{{.*}}i32 28
  static int __attribute__((bank_bits(4,5))) var_three;
//CHECK-SAME: var_four{{.*}}[[ANN3]]{{.*}}i32 30
  static int __attribute__((numbanks(4),bank_bits(4,5))) var_four;
//CHECK-SAME: var_five{{.*}}[[ANN3]]{{.*}}i32 32
  static int __attribute__((bank_bits(4,5),numbanks(4))) var_five;
//CHECK-SAME: var_six{{.*}}[[ANN4]]{{.*}}i32 34
  static int __attribute__((bank_bits(5,4),numbanks(4))) var_six;

  // 4. 5. 6.
//CHECK-SAME: var_seven{{.*}}[[ANN5]]{{.*}}i32 38
  static int __attribute__((numports_readonly_writeonly(2,3))) var_seven;
//CHECK-SAME: var_eight{{.*}}[[ANN5]]{{.*}}i32 40
  static int __attribute__((numreadports(2),numwriteports(3))) var_eight;

  // 7.
//CHECK-SAME: var_nine{{.*}}[[ANN6]]{{.*}}i32 44
  static int __attribute__((register)) var_nine;

  // 8.
//CHECK-SAME: var_ten{{.*}}[[ANN7]]{{.*}}i32 48
  static int __attribute__((__memory__)) var_ten;

  // 9.
//CHECK-SAME: var_eleven{{.*}}[[ANN8]]{{.*}}i32 52
  static int __attribute__((__bankwidth__(4))) var_eleven;

  // 10.
//CHECK-SAME: var_twelve{{.*}}[[ANN9]]{{.*}}i32 56
  static int __attribute__((singlepump)) var_twelve;

  // 11.
//CHECK-SAME: var_thirteen{{.*}}[[ANN10]]{{.*}}i32 60
  static int __attribute__((doublepump)) var_thirteen;
}

//CHECK-SAME: cfo{{.*}}[[ANN11]]{{.*}}i32 117

// 1.
//CHECK-SAME: evar_one{{.*}}[[ANN1]]{{.*}}i32 67
static int __attribute__((merge("foo","depth"))) evar_one;
//CHECK-SAME: evar_two{{.*}}[[ANN2]]{{.*}}i32 69
static int __attribute__((merge("bar","width"))) evar_two;

// 2. 3.
//CHECK-SAME: evar_three{{.*}}[[ANN3]]{{.*}}i32 73
static int __attribute__((bank_bits(4,5))) evar_three;
//CHECK-SAME: evar_four{{.*}}[[ANN3]]{{.*}}i32 75
static int __attribute__((numbanks(4),bank_bits(4,5))) evar_four;
//CHECK-SAME: evar_five{{.*}}[[ANN3]]{{.*}}i32 77
static int __attribute__((bank_bits(4,5),numbanks(4))) evar_five;
//CHECK-SAME: evar_six{{.*}}[[ANN4]]{{.*}}i32 79
static int __attribute__((bank_bits(5,4),numbanks(4))) evar_six;

// 4. 5. 6.
//CHECK-SAME: evar_seven{{.*}}[[ANN5]]{{.*}}i32 83
static int __attribute__((numports_readonly_writeonly(2,3))) evar_seven;
//CHECK-SAME: evar_eight{{.*}}[[ANN5]]{{.*}}i32 85
static int __attribute__((numreadports(2),numwriteports(3))) evar_eight;

// 7.
//CHECK-SAME: evar_nine{{.*}}[[ANN6]]{{.*}}i32 89
static int __attribute__((register)) evar_nine;

// 8.
//CHECK-SAME: evar_ten{{.*}}[[ANN7]]{{.*}}i32 93
static int __attribute__((__memory__)) evar_ten;

// 9.
//CHECK-SAME: evar_eleven{{.*}}[[ANN8]]{{.*}}i32 97
static int __attribute__((__bankwidth__(4))) evar_eleven;

// 10.
//CHECK-SAME: evar_twelve{{.*}}[[ANN9]]{{.*}}i32 101
static int __attribute__((singlepump)) evar_twelve;

// 11.
//CHECK-SAME: evar_thirteen{{.*}}[[ANN10]]{{.*}}i32 105
static int __attribute__((doublepump)) evar_thirteen;

__attribute__((ihc_component))
int foo_one() {
  return evar_one+evar_two+evar_three+evar_four+evar_five+evar_six+evar_seven+
         evar_eight+evar_nine+evar_ten+evar_eleven+evar_twelve+evar_thirteen;
}

//CHECK-SAME: afoo{{.*}}[[ANN11]]{{.*}}i32 114
static int __attribute__((merge("bar", "width"))) afoo[5];
__attribute__((ihc_component))
void width_manual(int *rdata) {
  static int __attribute__((merge("bar", "width"))) cfoo[5];
  *rdata = 0;
  *rdata += afoo[2];
  *rdata += cfoo[2];
}
