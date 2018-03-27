//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s

//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{merge:foo:depth}
//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{merge:bar:width}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{numbanks:4}{bank_bits:4,5}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{numbanks:4}{bank_bits:5,4}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{numreadports:2}{numwriteports:3}
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{register:0}
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{bankwidth:4}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{pump:1}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{pump:2}


//CHECK: @llvm.global.annotations

__attribute__((ihc_component))
void foo_two() {
  // 1.
//CHECK-SAME: var_one{{.*}}[[ANN1]]{{.*}}i32 21
  static int __attribute__((merge("foo","depth"))) var_one;
//CHECK-SAME: var_two{{.*}}[[ANN2]]{{.*}}i32 23
  static int __attribute__((merge("bar","width"))) var_two;

  // 2. 3.
//CHECK-SAME: var_three{{.*}}[[ANN3]]{{.*}}i32 27
  static int __attribute__((bank_bits(4,5))) var_three;
//CHECK-SAME: var_four{{.*}}[[ANN3]]{{.*}}i32 29
  static int __attribute__((numbanks(4),bank_bits(4,5))) var_four;
//CHECK-SAME: var_five{{.*}}[[ANN3]]{{.*}}i32 31
  static int __attribute__((bank_bits(4,5),numbanks(4))) var_five;
//CHECK-SAME: var_six{{.*}}[[ANN4]]{{.*}}i32 33
  static int __attribute__((bank_bits(5,4),numbanks(4))) var_six;

  // 4. 5. 6.
//CHECK-SAME: var_seven{{.*}}[[ANN5]]{{.*}}i32 37
  static int __attribute__((numports_readonly_writeonly(2,3))) var_seven;
//CHECK-SAME: var_eight{{.*}}[[ANN5]]{{.*}}i32 39
  static int __attribute__((numreadports(2),numwriteports(3))) var_eight;

  // 7.
//CHECK-SAME: var_nine{{.*}}[[ANN6]]{{.*}}i32 43
  static int __attribute__((register)) var_nine;

  // 8.
//CHECK-SAME: var_ten{{.*}}[[ANN7]]{{.*}}i32 47
  static int __attribute__((__memory__)) var_ten;

  // 9.
//CHECK-SAME: var_eleven{{.*}}[[ANN8]]{{.*}}i32 51
  static int __attribute__((__bankwidth__(4))) var_eleven;

  // 10.
//CHECK-SAME: var_twelve{{.*}}[[ANN9]]{{.*}}i32 55
  static int __attribute__((singlepump)) var_twelve;

  // 11.
//CHECK-SAME: var_thirteen{{.*}}[[ANN10]]{{.*}}i32 59
  static int __attribute__((doublepump)) var_thirteen;
}

//CHECK-SAME: cfo{{.*}}[[ANN2]]{{.*}}i32 116

// 1.
//CHECK-SAME: evar_one{{.*}}[[ANN1]]{{.*}}i32 66
static int __attribute__((merge("foo","depth"))) evar_one;
//CHECK-SAME: evar_two{{.*}}[[ANN2]]{{.*}}i32 68
static int __attribute__((merge("bar","width"))) evar_two;

// 2. 3.
//CHECK-SAME: evar_three{{.*}}[[ANN3]]{{.*}}i32 72
static int __attribute__((bank_bits(4,5))) evar_three;
//CHECK-SAME: evar_four{{.*}}[[ANN3]]{{.*}}i32 74
static int __attribute__((numbanks(4),bank_bits(4,5))) evar_four;
//CHECK-SAME: evar_five{{.*}}[[ANN3]]{{.*}}i32 76
static int __attribute__((bank_bits(4,5),numbanks(4))) evar_five;
//CHECK-SAME: evar_six{{.*}}[[ANN4]]{{.*}}i32 78
static int __attribute__((bank_bits(5,4),numbanks(4))) evar_six;

// 4. 5. 6.
//CHECK-SAME: evar_seven{{.*}}[[ANN5]]{{.*}}i32 82
static int __attribute__((numports_readonly_writeonly(2,3))) evar_seven;
//CHECK-SAME: evar_eight{{.*}}[[ANN5]]{{.*}}i32 84
static int __attribute__((numreadports(2),numwriteports(3))) evar_eight;

// 7.
//CHECK-SAME: evar_nine{{.*}}[[ANN6]]{{.*}}i32 88
static int __attribute__((register)) evar_nine;

// 8.
//CHECK-SAME: evar_ten{{.*}}[[ANN7]]{{.*}}i32 92
static int __attribute__((__memory__)) evar_ten;

// 9.
//CHECK-SAME: evar_eleven{{.*}}[[ANN8]]{{.*}}i32 96
static int __attribute__((__bankwidth__(4))) evar_eleven;

// 10.
//CHECK-SAME: evar_twelve{{.*}}[[ANN9]]{{.*}}i32 100
static int __attribute__((singlepump)) evar_twelve;

// 11.
//CHECK-SAME: evar_thirteen{{.*}}[[ANN10]]{{.*}}i32 104
static int __attribute__((doublepump)) evar_thirteen;

__attribute__((ihc_component))
int foo_one() {
  return evar_one+evar_two+evar_three+evar_four+evar_five+evar_six+evar_seven+
         evar_eight+evar_nine+evar_ten+evar_eleven+evar_twelve+evar_thirteen;
}

//CHECK-SAME: afoo{{.*}}[[ANN2]]{{.*}}i32 113
static int __attribute__((merge("bar", "width"))) afoo[5];
__attribute__((ihc_component))
void width_manual(int *rdata) {
  static int __attribute__((merge("bar", "width"))) cfoo[5];
  *rdata = 0;
  *rdata += afoo[2];
  *rdata += cfoo[2];
}
