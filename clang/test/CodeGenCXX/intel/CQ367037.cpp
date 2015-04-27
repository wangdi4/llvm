// RUN: %clang_cc1 -triple x86_64-windows-msvc -fms-compatibility -fintel-ms-compatibility -emit-llvm -o - -std=c++11 -g -o - %s | FileCheck %s
class Foo {
public:
  virtual int getY() { return 0; }
  virtual int getX() { return 1; }
  virtual ~Foo() {}
};

int main() {
  Foo *f = new Foo();
  delete f;
  return 0;
}

// CHECK: !MDSubprogram(name: "getY", linkageName: "{{.+}}getY@Foo{{.+}}", scope: !{{.+}}, file: !{{.+}}, line: [[@LINE-11]], type: !{{.+}}, isLocal: false, isDefinition: false, scopeLine: {{.+}}, containingType: !{{.+}}, virtuality: DW_VIRTUALITY_virtual, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
// CHECK: !MDSubprogram(name: "getX", linkageName: "{{.+}}getX@Foo{{.+}}", scope: !{{.+}}, file: !{{.+}}, line: [[@LINE-11]], type: !{{.+}}, isLocal: false, isDefinition: false, scopeLine: {{.+}}, containingType: !10, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
