//RUN: %clang_cc1 -fhls -O0 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-pch -o %t.pch %s
//RUN: %clang_cc1 -fhls -O0 -triple x86_64-unknown-linux-gnu \
//RUN:  -fdelayed-template-parsing -emit-pch -o %t.del.pch %s

//RUN: %clang_cc1 -DMAIN_FILE -fhls -O0 -triple x86_64-unknown-linux-gnu \
//RUN:  -include-pch %t.pch -emit-llvm -o - %s  | FileCheck %s

//RUN: %clang_cc1 -DMAIN_FILE -fhls -O0 -triple x86_64-unknown-linux-gnu \
//RUN:  -fdelayed-template-parsing -include-pch %t.del.pch -emit-llvm -o - %s  | FileCheck %s

#ifndef MAIN_FILE
struct SIVDep {
  int A[32];
}SV;

struct SIVDep2 {
  SIVDep X[8][16];
}SV2, *Sv2p = &SV2;

void bar(int i);
int ibar(int i);

template <typename T>
void a(T select) {

  int myArray[32];
  #pragma ivdep array(myArray)
  for (int i=0;i<32;++i) { myArray[i] = ibar(i); }

  #pragma ivdep array(Sv2p->X[2][3].A)
  for (int i=0;i<32;++i) { Sv2p->X[2][3].A[i] = ibar(i); }

}
#else
  //CHECK: !llvm.index.group [[IVDEP3:![0-9]+]]
  //CHECK: br{{.*}}!llvm.loop [[IVDEP4:![0-9]+]]

  //CHECK: !llvm.index.group [[IVDEP13:![0-9]+]]
  //CHECK: br{{.*}}!llvm.loop [[IVDEP14:![0-9]+]]
void foo()
{
  a(1);
}
#endif
