// RUN:  ocl-tblgen -inject city.Name=Haifa %s | FileCheck %s
// CHECK: string Name = "Haifa";

class Injectable{
  string InjectValue;
}

class City : Injectable{
  string Name = InjectValue;
}

def city : City;
