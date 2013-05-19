
#pragma once

#include "mic_device_interface.h"

#ifdef USE_ITT
#include <ocl_itt.h>
#endif

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

extern Intel::OpenCL::MICDevice::mic_exec_env_options gMicExecEnvOptions;

#ifdef USE_ITT
extern  ocl_gpa_data                                  gMicGPAData;
#endif

class DeviceTracer;
extern DeviceTracer gTracer;

}}}

