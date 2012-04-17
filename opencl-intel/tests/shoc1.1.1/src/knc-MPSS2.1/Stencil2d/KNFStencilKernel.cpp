#if defined(__APPLE__)
#if _GLIBCXX_ATOMIC_BUILTINS == 1
#undef _GLIBCXX_ATOMIC_BUILTINS
#endif // _GLIBCXX_ATOMIC_BUILTINS
#endif // __APPLE__

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "omp.h"
#include "math.h"
#include "offload.h"
#include "Timer.h"
#include "KNFStencil.cpp"

#define MAX_OMP_THREADS	256

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel Author:	Valentin Andrei	- valentin.andrei@intel.com	(SSG/SSD/PTAC/PAC Characterization & Measurement)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
void
KNFStencil<T>::operator()( Matrix2D<T>& mtx, unsigned int nIters )
{
   	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned int uOMP_Threads = 236;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    __declspec(target(mic)) static T* 	rarr1 = mtx.GetFlatData();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    __declspec(target(mic)) unsigned int 	nrows			= mtx.GetNumRows();
    __declspec(target(mic)) unsigned int 	ncols			= mtx.GetNumColumns();
    __declspec(target(mic))	unsigned int	nextralines		= (nrows - 2) % uOMP_Threads;
	__declspec(target(mic)) unsigned int	uLinesPerThread = (unsigned int)floor((double)(nrows - 2) / uOMP_Threads);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    __declspec(target(mic))	T wcenter	= this->wCenter;
    __declspec(target(mic)) T wdiag		= this->wDiagonal;
    __declspec(target(mic))	T wcardinal	= this->wCardinal;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned int len = nrows * ncols;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	#pragma offload target(mic) inout(rarr1:length(len)) in(uOMP_Threads) in(uLinesPerThread) in(nrows) in(ncols) in(nextralines) in(wcenter) in(wdiag) in(wcardinal)
	{
		T*	pTmp	= rarr1;
		T*	pCrnt	= (T*)malloc(len * sizeof(T));
		T*	pAux	= NULL;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		for (unsigned int cntIterations = 0; cntIterations < nIters; cntIterations ++)
		{
			#pragma omp parallel for firstprivate(pTmp, pCrnt, uLinesPerThread, wdiag, wcardinal, wcenter, ncols)
			for (unsigned int uThreadId = 0; uThreadId < uOMP_Threads; uThreadId++)
			{
				unsigned int uStartLine = 0;
				unsigned int uEndLine	= 0;

				if (uThreadId < nextralines)
				{
					uStartLine	= uThreadId 	* (uLinesPerThread + 1) + 1;
					uEndLine	= uStartLine 	+ (uLinesPerThread + 1);
				}
				else
				{
					uStartLine	= nextralines 	+ uThreadId	* uLinesPerThread + 1;
					uEndLine	= uStartLine 	+ uLinesPerThread;
				}

				T	cardinal0	= 0.0;
				T	diagonal0	= 0.0;
				T	center0		= 0.0;

				for (unsigned int cntLine = uStartLine; cntLine < uEndLine; cntLine ++)
				{
					#pragma ivdep
					for (unsigned int cntColumn = 1; cntColumn < (ncols - 1); cntColumn ++)
					{
						cardinal0 	= 	pTmp[(cntLine - 1) * ncols + cntColumn] +
										pTmp[(cntLine + 1) * ncols + cntColumn] +
										pTmp[ cntLine * ncols + cntColumn - 1] +
										pTmp[ cntLine * ncols + cntColumn + 1];

						diagonal0 	= 	pTmp[(cntLine - 1) * ncols + cntColumn - 1] +
										pTmp[(cntLine - 1) * ncols + cntColumn + 1] +
										pTmp[(cntLine + 1) * ncols + cntColumn - 1] +
										pTmp[(cntLine + 1) * ncols + cntColumn + 1];

						center0		= 	pTmp[cntLine * ncols + cntColumn];


						pCrnt[cntLine * ncols + cntColumn]	= wcenter * center0 + wdiag * diagonal0 + wcardinal * cardinal0;
					}
				}
			}

			// Switch pointers
			pAux	= pTmp;
			pTmp 	= pCrnt;
			pCrnt	= pAux;
		}
	}
}

void
EnsureStencilInstantiation( void )
{
    KNFStencil<float> csf( 0, 0, 0, 0 );
    Matrix2D<float> mf( 2, 2 );
    csf( mf, 0 );

    KNFStencil<double> csd( 0, 0, 0, 0 );
    Matrix2D<double> md( 2, 2 );
    csd( md, 0 );
}

