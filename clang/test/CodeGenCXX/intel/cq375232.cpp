// RUN: %clang_cc1 -triple x86_64-windows-msvc -fintel-compatibility -emit-llvm -o - %s | FileCheck %s

class CVssWriter
{
public:
  __stdcall CVssWriter() {}
  virtual __stdcall ~CVssWriter() {}
};

class CVssWriterEx : public CVssWriter
{};

int main() {
  CVssWriterEx *T = new CVssWriterEx();
  delete T;
// CHECK: %this1 = load %class.CVssWriterEx*, %class.CVssWriterEx** %this.addr
// CHECK: %0 = bitcast %class.CVssWriterEx* %this1 to %class.CVssWriter*
// CHECK: call void @"\01??1CVssWriter@@UEAA@XZ"(%class.CVssWriter* %0)
  return 0;
}

