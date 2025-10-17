#pragma once

#include "NsTween.h"

namespace NsTweenCore
{
    using FNsTweenInstance = FNsTween::FBuilder;
    using FNsTweenBuilder = FNsTween::FBuilder;

    template <typename... TArgs>
    auto Play(TArgs&&... Args) -> decltype(FNsTween::Play(Forward<TArgs>(Args)...))
    {
        return FNsTween::Play(Forward<TArgs>(Args)...);
    }
}

