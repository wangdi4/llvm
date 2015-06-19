// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma nounroll_and_jam test

// CHECK: define i32 @main{{.*}}
int main(int argc, char **argv)
{
  int i, lll;
  int j, k, aaa;
  i = 1;
// CHECK: store i32 1, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store i32 0, i32* %i{{.*}}
  #pragma nounroll_and_jam 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

nounroll_and_jam_label1:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: load i32, i32* %j{{.*}}
  #pragma nounroll_and_jam
  i+=j;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nounroll_and_jam
  do
  {
    ++i;
  }
  while (i > argc);
  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nounroll_and_jam
  while(i > argc)
  ;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: load i32, i32* %i{{.*}}
// CHECK-NEXT: store i32 %{{.*}}, i32* %l{{.*}}
  #pragma nounroll_and_jam
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
// CHECK: store i32 {{.*}}, i32* %aaa{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store i32 0, i32* %k{{.*}}
  #pragma nounroll_and_jam
    for(k = 0; k < argc; k++)
    ;
  }
  ;
nounroll_and_jam_label2:  
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
  #pragma nounroll_and_jam
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: br label {{.*}}
      #pragma nounroll_and_jam
      #pragma nounroll_and_jam 
      ;
      break;
    default:
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: store{{.*}}
// CHECK-NEXT: br label{{.*}}
      #pragma nounroll_and_jam
      return (1);
  }
  ++i;
// CHECK: store i32 %{{.*}}, i32* %i{{.*}}
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
  #pragma nounroll_and_jam 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma nounroll_and_jam;        // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma nounroll_and_jam
  
  return (i);
}

// CHECK: define void {{.*}}www{{.*}}(
void www()
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"UJAM_COUNT", metadata !"AMOUNT", metadata !"CONSTANT", metadata i32 0)
// CHECK-NEXT: ret {{.*}}
  #pragma nounroll_and_jam

  return;
}

