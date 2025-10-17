#pragma once

#include "NsTween.h"

struct NsTweenCore
{
    using FNsTweenInstance = FNsTweenBuilder;
    using FNsTweenBuilder = FNsTweenBuilder;

    template <typename... TArgs>
    static auto Play(TArgs&&... Args) -> decltype(FNsTween::Play(Forward<TArgs>(Args)...))
    {
        return FNsTween::Play(Forward<TArgs>(Args)...);
    }
};
