//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s

//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{numbanks:4}{bank_bits:4,5}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{numreadports:2}{numwriteports:3}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{register:0}
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{bankwidth:4}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{pump:1}
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{pump:2}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{merge:foo:depth}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{merge:bar:width}
//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{register:0}{pump:1}{bankwidth:4}{numbanks:8}{numreadports:2}{numwriteports:3}{bank_bits:2,3,4}{merge:merge_foo_one:depth}

__attribute__((ihc_component))
void foo_two() {
  //CHECK: llvm.var.annotation{{.*}}var_two{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: llvm.var.annotation{{.*}}var_three{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: llvm.var.annotation{{.*}}var_four{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: llvm.var.annotation{{.*}}var_five{{.*}}[[ANN3]]
  int __attribute__((numports_readonly_writeonly(2,3))) var_five;
  //CHECK: llvm.var.annotation{{.*}}var_six{{.*}}[[ANN3]]
  int __attribute__((numreadports(2),numwriteports(3))) var_six;
  //CHECK: llvm.var.annotation{{.*}}var_seven{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: llvm.var.annotation{{.*}}var_eight{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: llvm.var.annotation{{.*}}var_nine{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: llvm.var.annotation{{.*}}var_ten{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: llvm.var.annotation{{.*}}var_eleven{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: llvm.var.annotation{{.*}}var_twelve{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: llvm.var.annotation{{.*}}var_thirteen{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
}

template <int bankwidth, int numbanks, int readports, int writeports,
          int bit1, int bit2, int bit3>
__attribute__((ihc_component))
void foo_one()
{
  __attribute__((bankwidth(bankwidth),numbanks(numbanks),
                 numports_readonly_writeonly(readports,writeports),
                 merge("merge_foo_one","depth"), memory, singlepump,
                 __bank_bits__(bit1,bit2,bit3)))
  int var_one;
}

//CHECK: define{{.*}}foo_one{{.*}}!ihc_component
//CHECK: llvm.var.annotation{{.*}}var_one{{.*}}[[ANN1]]

void call()
{
  foo_one<4,8,2,3,2,3,4>();
}
