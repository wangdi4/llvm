//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility \
//RUN:  -O2 %s -emit-llvm -o - | FileCheck %s

void foobar();

typedef int Key;
typedef bool Val;

struct pair { Key first; Val second; } p, *pp = &p;

struct map_base {
  map_base() {}
  pair*& insert_node() { foobar(); return pp;}
  Val& operator[](const Key& __k) { return insert_node()->second; }
};

//Ensure that fakeload processing doesn't emit return value more than once.

//CHECK: call {{.*}}foobar
//CHECK-NOT: call {{.*}}foobar

int main() {
  map_base mb;
  mb[0] = true;
  return 0;
}
