#pragma once

#include "CL/cl.h"
#include "oclInternalFunctions.h"

// Provides smart wrapper for retaining and releasing OCL objects.
// Retains the given object on construction, and 
// releases it on destruction.
class OclRetainer
{
public:
    virtual void retain() = 0;
    virtual void release() = 0;
    virtual bool hasRetained() const
    {
        return m_retained;
    };

protected:
    bool m_retained;
};

class OclKernelRetainer : public OclRetainer
{
public:
    OclKernelRetainer(cl_kernel kernel):
        m_kernel(kernel)
    {
        retain();
    }
    ~OclKernelRetainer()
    {
        release();
    }
    virtual void retain()
    {
        cl_int err = _clRetainKernelINTERNAL(m_kernel, true);
        m_retained = (CL_SUCCESS == err) ? true : false;

    };
    virtual void release()
    {
        if (m_retained)
        {
            _clReleaseKernelINTERNAL(m_kernel, true);
            m_retained = false;
        }
    };

private:
    const cl_kernel m_kernel;
};

class OclMemObjRetainer : public OclRetainer
{
public:
    OclMemObjRetainer(cl_mem memobj):
        m_memobj(memobj)
    {
        retain();
    }
    ~OclMemObjRetainer()
    {
        release();
    }

    virtual void retain()
    {
        cl_int err = _clRetainMemObjectINTERNAL(m_memobj, true);
        m_retained = (CL_SUCCESS == err) ? true : false;

    };
    virtual void release()
    {
        if (m_retained)
        {
            _clReleaseMemObjectINTERNAL(m_memobj, true);
            m_retained = false;
        }
    };

private:
    const cl_mem m_memobj;
};

OclRetainer* getRetainer(cl_mem memobj);
OclRetainer* getRetainer(cl_kernel kernel);