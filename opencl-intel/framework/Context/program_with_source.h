#pragma once
#include "program.h"
#include "observer.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithSource : public Program
	{

        PREPARE_SHARED_PTR(ProgramWithSource);

	public:		

        static SharedPtr<ProgramWithSource> Allocate(SharedPtr<Context> pContext, cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths, cl_int* piRet)
        {
            return SharedPtr<ProgramWithSource>(new ProgramWithSource(pContext, uiNumStrings, pSources, pszLengths, piRet));
        }		

		cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

        // Returns a read only pointer to internal source, used for build stages by program service
        virtual const char*     GetSourceInternal() { return m_szSourceString; };

	protected:

        ProgramWithSource(SharedPtr<Context> pContext, cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths, cl_int* piRet);

		virtual ~ProgramWithSource();

		bool CopySourceStrings(cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths);

		char*  m_szSourceString;
	
	private:
		ProgramWithSource(const ProgramWithSource&);
		ProgramWithSource& operator=(const ProgramWithSource&);
	};
}}}
