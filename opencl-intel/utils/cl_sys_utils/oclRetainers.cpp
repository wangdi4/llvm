#include "oclRetainers.h"

OclRetainer* getRetainer(cl_mem memobj)
{
    return new OclMemObjRetainer(memobj);
}

OclRetainer* getRetainer(cl_kernel kernel)
{
    return new OclKernelRetainer(kernel);
}