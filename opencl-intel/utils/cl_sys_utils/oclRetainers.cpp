#include "oclRetainers.h"

OclKernelRetainer::OclKernelRetainer(cl_kernel kernel):
    m_kernel(kernel)
{
    retain();
}

OclKernelRetainer::~OclKernelRetainer()
{
    release();
}

void OclKernelRetainer::retain()
{
    cl_int err = _clRetainKernelINTERNAL(m_kernel, true);
    m_retained = (CL_SUCCESS == err) ? true : false;
}

void OclKernelRetainer::release()
{
    if (m_retained)
    {
        _clReleaseKernelINTERNAL(m_kernel, true);
        m_retained = false;
    }
}

OclMemObjRetainer::OclMemObjRetainer(cl_mem memobj):
    m_memobj(memobj)
{
    retain();
}

OclMemObjRetainer::~OclMemObjRetainer()
{
    release();
}


void OclMemObjRetainer::retain()
{
    cl_int err = _clRetainMemObjectINTERNAL(m_memobj, true);
    m_retained = (CL_SUCCESS == err) ? true : false;
}

void OclMemObjRetainer::release()
{
    if (m_retained)
    {
        _clReleaseMemObjectINTERNAL(m_memobj, true);
        m_retained = false;
    }
}


OclRetainer* getRetainer(cl_mem memobj)
{
    return new OclMemObjRetainer(memobj);
}

OclRetainer* getRetainer(cl_kernel kernel)
{
    return new OclKernelRetainer(kernel);
}