#pragma once
#include "program.h"
#include "observer.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithSource : public Program
	{
	public:
		ProgramWithSource(Context* pContext, cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths, cl_int* piRet, ocl_entry_points * pOclEntryPoints);

		cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

	protected:
		virtual ~ProgramWithSource();

		bool CopySourceStrings(cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths);

		cl_uint m_uiNumStrings;
		size_t* m_pszStringLengths;
		char**  m_pSourceStrings;



	};
}}}
