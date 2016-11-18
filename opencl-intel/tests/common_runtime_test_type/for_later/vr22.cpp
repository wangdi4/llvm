// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "common_runtime_tests.h"

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
//#include <CL\cl_d3d9.h>

//|	TEST: CommonRuntime.D3D9Share
//|
//|	Purpose
//|	-------
//|	
//|	verify that the extension cl_intel_d3d9_sharing 
//| is supported by both CPU and GPU devices TC-27
//|
//|	Method
//|	------
//|
//|	Querry for OpenCL devices corresponding to Direct3D device
//| using clGetDeviceIDsFromD3D9Intel
//|	
//|	Pass criteria
//|	-------------
//|
//|	The API should return 2 Devices (1 for the GPU and 1 for the CPU)
//|

TEST_F(CommonRuntime, D3D9Share)
{
	// Create an invisible window for d3d9	
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, 		
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,		
		"WIN CLASS", NULL };	
	RegisterClassEx(&wc);	
	HINSTANCE mWindowClass = wc.hInstance;	
	HWND mWindow = CreateWindow(		
		"WIN CLASS", "Invisible", WS_OVERLAPPEDWINDOW,		
		0, 0, 1, 1,		
		GetDesktopWindow(), 0,		
		mWindowClass, NULL);
	

	IDirect3D9 *pD3D = NULL; //global variable that will be used by a couple of our function  
	IDirect3DDevice9 *pDevice = NULL; //a device that will be used for most of our function created inside *pD3D 

	D3DPRESENT_PARAMETERS d3dpp; //the presentation parameters that will be used when we will create the device  
	ZeroMemory(&d3dpp,sizeof(d3dpp)); //to be sure d3dpp is empty  
	d3dpp.Windowed = true; //use our global windowed variable to tell if the program is windowed or not  
	d3dpp.hDeviceWindow = mWindow; //give the window handle of the window we created above  
	d3dpp.BackBufferCount = 1; //set it to only use 1 backbuffer  
	d3dpp.BackBufferWidth = 1; //set the buffer to our window width  
	d3dpp.BackBufferHeight = 1; //set the buffer to out window height  
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8; //the backbuffer format  
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //SwapEffect  
  
	pD3D = Direct3DCreate9(D3D_SDK_VERSION); //Create the presentation parameters  
  
	ASSERT_EQ(D3D_OK,(pD3D->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,mWindow,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&pDevice)))
		<<"Could not create D3D9 Device\n";

	clGetDeviceIDsFromD3D9Intel_fn clGetDeviceIDsFromD3D9Intel;
	clGetDeviceIDsFromD3D9Intel = (clGetDeviceIDsFromD3D9Intel_fn)clGetExtensionFunctionAddress("clGetDeviceIDsFromD3D9Intel");
	ASSERT_NE((void*)NULL,(void*)clGetDeviceIDsFromD3D9Intel)<<"Extension clGetDeviceIDsFromD3D9Intel address not found";

	//Attempt to create OpenCL context
	//Get Platforms
	cl_uint num_entries = 2;
	cl_uint num_platforms = 0;
	cl_platform_id platforms[] = {0,0,0};
	ASSERT_NO_FATAL_FAILURE(getPlatformIDs(num_entries, platforms, &num_platforms));
	
	cl_context_properties ctxProps[] = 
	{
		CL_CONTEXT_PLATFORM, 
		(cl_context_properties)platforms[0], //assumes intel is first platform
		0
	};
	cl_context sharedcontext = 0;
	cl_int err;
	sharedcontext = clCreateContextFromType( ctxProps, CL_DEVICE_TYPE_ALL, NULL, NULL, &err );
	//check errors
	EXPECT_NE(NULL,(int)sharedcontext) << "Returned Context is NULL";
	
	const int maxdevices = 4; //assume no more than 4 devices in platform
	cl_uint numdevices;
	//get gpu only devices
	cl_device_id devices[maxdevices];
	err = clGetDeviceIDsFromD3D9Intel(platforms[0],CL_D3D9_DEVICE_INTEL,pDevice,CL_ALL_DEVICES_FOR_D3D9_INTEL,maxdevices,devices,&numdevices);
	EXPECT_EQ(CL_SUCCESS,err)<<"Failed to get device ids from D3D9 device";
	EXPECT_EQ(numdevices,2)<<"Failed to get both CPU and GPU device";

	clReleaseContext(sharedcontext);
	//clean up
	if(pD3D) 
			pD3D->Release(); 
	if(pDevice) 
		pDevice->Release(); 

}

#endif