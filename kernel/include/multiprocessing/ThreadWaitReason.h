#pragma once

namespace Kernel::Multiprocessing
{
    enum class WaitReasonType
    {

    };

    struct ThreadWaitReason
    {
        const WaitReasonType type;

        ThreadWaitReason(WaitReasonType type) : type(type)
        {}
    };
}
