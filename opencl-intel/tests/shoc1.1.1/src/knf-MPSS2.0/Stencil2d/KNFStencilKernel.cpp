//===========================================================================
//
// This example from a prerelease of the Scalable HeterOgeneous Computing
// (SHOC) Benchmark Suite Alpha v1.1.1i for Intel MIC architecture
// Contact: Kyle Spafford <kys@ornl.gov>
//         Rezaur Rahman <rezaur.rahman@intel.com>
//
// Copyright (c) 2011, UT-Battelle, LLC
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of Oak Ridge National Laboratory, nor UT-Battelle, LLC, nor
//    the names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ==============================================================================
//
#if defined(__APPLE__)
#if _GLIBCXX_ATOMIC_BUILTINS == 1
#undef _GLIBCXX_ATOMIC_BUILTINS
#endif // _GLIBCXX_ATOMIC_BUILTINS
#endif // __APPLE__

//#pragma offload_attribute(push,target(mic))

#include <stdio.h>
#include <assert.h>
#include "offload.h"
#include "KNFStencil.cpp"
#include <string.h>
#include "omp.h"
#include <stdio.h>
#include "math.h"
#include "Timer.h"

//#pragma offload_attribute(pop)

#define BLOCKX 120
#define BLOCKY 128

template <class T>
void 
prefetcharr(T *arr1,int k, int l, const int ncols)
{
	#ifdef __MIC__
	_mm_vprefetch2(arr1+k*ncols+l,_MM_PFHINT_NONE);
        _mm_vprefetch2(arr1+(k+1)*ncols+l,_MM_PFHINT_NONE);
        _mm_vprefetch2(arr1+(k+2)*ncols+l,_MM_PFHINT_NONE);
	#endif
}

/*
###########################################################################################
Stencil2D Kernel Written by Sumedh Naik <sumedh.naik@intel.com>
###########################################################################################
*/

template <class T>
void
KNFStencil<T>::operator()( Matrix2D<T>& mtx, unsigned int nIters )
{

    // be able to access the matrices as 2D arrays
    typename Matrix2D<T>::DataPtr mtxData = mtx.GetData();

   /* Apply the stencil operator */

    __declspec(target(mic)) static T* rarr1=mtx.GetFlatData();

    __declspec(target(mic)) unsigned int nrows= mtx.GetNumRows();
    __declspec(target(mic)) unsigned int ncols= mtx.GetNumColumns();
    T wcenter=this->wCenter;
    T wdiag=this->wDiagonal;
    T wcardinal=this->wCardinal;

    unsigned int len=nrows*ncols;
    __declspec(target(mic)) unsigned int flip=0;

   static double ktime;

    #pragma offload target(mic)\
    	inout(rarr1:length(len))
    {
     
	//double kstart = curr_second();

        T restrict *arr1;
	T restrict *arr2;
	T *rarr2=(T*)malloc(nrows*ncols*sizeof(T));
	T **ipblockptr,**opblockptr;

	unsigned int numblocks = (unsigned int)ceil((double)nrows/BLOCKX)*ceil((double)ncols/BLOCKY);

	T** arr1blockptr=(T**)malloc(numblocks*sizeof(T*));
        T** arr2blockptr=(T**)malloc(numblocks*sizeof(T*));
        int* xstart=(int*)malloc(numblocks*sizeof(int));
        int* ystart=(int*)malloc(numblocks*sizeof(int));
        int* xend=(int*)malloc(numblocks*sizeof(int));
        int* yend=(int*)malloc(numblocks*sizeof(int));
	

	for(int i=0,x=0;i<nrows;i+=BLOCKX)
	{
                for( int j=0;j<ncols;j+=BLOCKY,x++)
                {
                        arr1blockptr[x]=&rarr1[i*ncols+j];
                        arr2blockptr[x]=&rarr2[i*ncols+j];
                        xstart[x]=(i==0)?1:0;
                        ystart[x]=(j==0)?1:0;
                        xend[x]=(BLOCKX>=(nrows-i))?(nrows-i-1):(BLOCKX);
                        yend[x]=(BLOCKY>=(ncols-j))?(ncols-j-1):(BLOCKY);
                }
	}

   for( unsigned int iter = 0; iter < nIters; iter++ ) 
    {
	arr1=(flip==0)?rarr1:rarr2;
	arr2=(flip==0)?rarr2:rarr1;
	ipblockptr=(flip==0)?arr1blockptr:arr2blockptr;
	opblockptr=(flip==1)?arr1blockptr:arr2blockptr;	
	
	flip=1-flip;
        //DoPreIterationWork( mtx, iter );
	#if 1

        #pragma omp parallel for\
                shared(ipblockptr,opblockptr,xstart,ystart,xend,yend)
        for(int i=0;i<numblocks;i++)
        {

                int istart,jstart,iend,jend;
                T diag, cardinal,center;

                arr1=ipblockptr[i];
                arr2=opblockptr[i];
                istart=xstart[i];
                jstart=ystart[i];
                iend=xend[i];
                jend=yend[i];
		
		#pragma ivdep
		for(int k=istart;k<iend;k++)
                        {
				#pragma ivdep
                                for(int l=jstart;l<jend;l++)
                                {
					//prefetcharr<T>(arr1,k,l,ncols);
                                        int index1,index2,index3,index4;

					/*
                                        cardinal= *(arr1+(k-1)*ncols+l)+\
                                                        *(arr1+(k+1)*ncols+l)+\
                                                        *(arr1+k*ncols+l-1)+\
                                                        *(arr1+k*ncols+l+1);*/
					index1=(k-1)*ncols+l;
					index2=(k+1)*ncols+l;
					index3=k*ncols+l-1;
					index4=k*ncols+l+1;
					cardinal=*(arr1+index1)+*(arr1+index2)+*(arr1+index3)+*(arr1+index4);

					/*
                                        diag=     arr1[(k-1)*ncols+(l-1)]+\
                                                        arr1[(k+1)*ncols+(l+1)]+\
                                                        arr1[(k-1)*ncols+(l+1)]+\
                                                        arr1[(k+1)*ncols+(l-1)];*/

					index1=(k-1)*ncols+(l-1);
					index2=(k-1)*ncols+l+1;
					index3=(k+1)*ncols+(l-1);
					index4=(k+1)*ncols+l+1;
					diag=*(arr1+index1)+*(arr1+index2)+*(arr1+index3)+*(arr1+index4);

                                        center=arr1[k*ncols+l];
                                        arr2[k*ncols+l]=wcenter*center+wcardinal*cardinal+wdiag*diag;
					
                                }
                        }
        }
	#endif
    } 
	if(flip==1)
	{ 
		T *tarrptr;
		tarrptr=rarr1;
		rarr1=rarr2;
		rarr2=tarrptr;
	}

	free(ipblockptr);
        free(opblockptr);
        free(xstart);
        free(ystart);
        free(xend);
        free(yend);
	free(rarr2);
	//ktime=curr_second()-kstart;
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

