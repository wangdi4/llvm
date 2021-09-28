// INTEL_COPYRIGHT_BEGIN
// Copyright (c) 2020, Intel Corporation. All rightrs reserved.
// INTEL_COPYRIGHT_END

#include "kernel.hpp"
#include "ze_buffer.hpp"
#include "ze_utils.hpp"
#include <cassert>
#include <cstdio>

namespace cpulevel0 {

Kernel::Kernel(ZeModule *module) : module_(module) {}

Kernel::~Kernel()
{
}

ze_result_t Kernel::initialize(const ze_kernel_desc_t *desc)
{
    assert(desc != nullptr);

    descriptor_ = *desc;

    return ZE_RESULT_SUCCESS;
}


ze_result_t Kernel::setArgumentValue(uint32_t argIndex,
                                       size_t argSize,
                                       const void *pArgValue)
{
    auto status = ZE_RESULT_SUCCESS;

    if (argIndex >= param_count_)
    {
        ZESIMERR << "Incorrect arg index, expecting argIndex= " << argIndex
                 << " < " << param_count_;
        return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX;
    }
    
    return ZE_RESULT_SUCCESS;
}

ze_result_t Kernel::suggestGroupSize(uint32_t globalSizeX,
                                       uint32_t globalSizeY,
                                       uint32_t globalSizeZ,
                                       uint32_t *groupSizeX,
                                       uint32_t *groupSizeY,
                                       uint32_t *groupSizeZ)
{
    (void)globalSizeX;
    (void)globalSizeY;
    (void)globalSizeZ;
    *groupSizeX = 4;
    *groupSizeY = 1;
    *groupSizeZ = 1;
    return ZE_RESULT_SUCCESS;
}

ze_result_t Kernel::setGroupSize(uint32_t groupSizeX,
                                   uint32_t groupSizeY,
                                   uint32_t groupSizeZ)
{
    if ((0 == groupSizeX) || (0 == groupSizeY) || (0 == groupSizeZ))
    {
        ZESIMERR << "Cannot have groupSize zero, got [" << groupSizeX << ","
                 << groupSizeY << "," << groupSizeZ << "]";
        return ZE_RESULT_ERROR_INVALID_ARGUMENT;
    }

    this->groupSize[0] = groupSizeX;
    this->groupSize[1] = groupSizeY;
    this->groupSize[2] = groupSizeZ;
    auto status = genISAi_kernel_set_3d_group_size(genISAi_kernel_,
                                                   this->groupSize.data());
    if (GENISAI_STATUS_SUCCESS != status)
    {
        ZESIMERR << "genISAi: failed to set 3d thread space, error=" << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }
    return ZE_RESULT_SUCCESS;
}

ze_result_t Kernel::launch(ze_group_count_t config)
{
    return module()->device()->launch(this, config);
}

ze_result_t Kernel::setSurfaceState(uint32_t argIndex,
                                      const void *pArgValue,
                                      unsigned int *bti)
{
    auto ret =
        genISAi_kernel_get_binding_table_index(genISAi_kernel_, argIndex, bti);
    if (GENISAI_STATUS_SUCCESS != ret || *bti > max_bti)
    {
        ZESIMERR << "genISAi: failed to get BTI, error=" << ret;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    // get ZeBuffer
    auto buffer = module_->device()->driver()->getZeBuffer(
        *reinterpret_cast<void *const *>(pArgValue));
    if (!buffer)
    {
        ZESIMERR << "Invalid pointer " << pArgValue
                 << ", unable to map it to device allocation";
        return ZE_RESULT_ERROR_INVALID_ARGUMENT;
    }

    // set surface state
    genISAi_surface_state_t surface_state;
    surface_state.width =
        (unsigned int)(buffer->ptr_end() -
                       *reinterpret_cast<uint8_t *const *>(pArgValue));
    surface_state.height = 1;
    surface_state.depth = 1;
    surface_state.pitch = 0;
    surface_state.base_address = *reinterpret_cast<void *const *>(pArgValue);

    // register surface state in binding table of genISAi. if the BTI is
    // occupied, unregister it first, then register with new surface state
    auto status = genISAi_kernel_register_surface_with_descriptor(
        genISAi_kernel_, *bti, &surface_state);
    if (GENISAI_STATUS_ERROR_OCCUPIED_BTI == status)
    {
        ZESIMOUT << "genISAi: BTI " << *bti << " is occupied";
        genISAi_kernel_unregister_surface_with_BTI(genISAi_kernel_, *bti);
        status = genISAi_kernel_register_surface_with_descriptor(
            genISAi_kernel_, *bti, &surface_state);
    }
    if (GENISAI_STATUS_SUCCESS != status)
    {
        ZESIMERR << "genISAi: failed to register BTI " << *bti
                 << ", error= " << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    return ZE_RESULT_SUCCESS;
}

ze_result_t
Kernel::getArgAttributes(uint32_t argIndex,
                           unsigned int &size,
                           genISAi_kernel_param_type_t &arg_type,
                           genISAi_kernel_param_address_mode_t &addr_mode,
                           genISAi_kernel_param_address_space_t &addr_space,
                           genISAi_kernel_param_access_type_t &access_type)
{
    auto status = genISAi_kernel_get_kernel_param_type(
        genISAi_kernel_, argIndex, &arg_type);
    if (GENISAI_STATUS_SUCCESS != status)
    {
        fprintf(stderr, "failed to get the argument type!\n");
        ZESIMERR << "genISAi: failed to get kernel param type, error="
                 << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    status =
        genISAi_kernel_get_kernel_param_size(genISAi_kernel_, argIndex, &size);
    if (GENISAI_STATUS_SUCCESS != status)
    {
        ZESIMERR << "genISAi: failed to get kernel param size, error="
                 << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    status = genISAi_kernel_get_kernel_param_address_mode(
        genISAi_kernel_, argIndex, &addr_mode);
    if (GENISAI_STATUS_SUCCESS != status)
    {
        ZESIMERR << "genISAi: failed to get kernel param address mode, error="
                 << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    status = genISAi_kernel_get_kernel_param_address_space(
        genISAi_kernel_, argIndex, &addr_space);
    if (GENISAI_STATUS_SUCCESS != status)
    {
        ZESIMERR << "genISAi: failed to get kernel param address space, error="
                 << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    status = genISAi_kernel_get_kernel_param_access_type(
        genISAi_kernel_, argIndex, &access_type);
    if (GENISAI_STATUS_SUCCESS != status)
    {
        ZESIMERR << "genISAi: failed to get kernel param access type, error="
                 << status;
        return ZE_RESULT_ERROR_UNKNOWN;
    }

    return ZE_RESULT_SUCCESS;
}

unsigned int Kernel::getStaticSLMSize()
{
    unsigned int static_slm_size = 0;
    genISAi_kernel_get_group_static_SLM_size(genISAi_kernel_, &static_slm_size);
    return static_slm_size;
}

ze_result_t Kernel::setGroupCount(uint32_t groupCountX,
                                    uint32_t groupCountY,
                                    uint32_t groupCountZ)
{
    if ((0 == groupCountX) || (0 == groupCountY) || (0 == groupCountZ))
    {
        ZESIMERR << "Cannot have groupSize zero, got [" << groupCountX << ","
                 << groupCountY << "," << groupCountZ << "]";
        return ZE_RESULT_ERROR_INVALID_ARGUMENT;
    }

    this->groupCount[0] = groupCountX;
    this->groupCount[1] = groupCountY;
    this->groupCount[2] = groupCountZ;

    return ZE_RESULT_SUCCESS;
}
}    // namespace __zert__
