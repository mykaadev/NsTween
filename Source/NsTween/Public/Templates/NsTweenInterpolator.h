// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/** Provides interpolation helpers for NsTween value types. */
template <typename TValue>
struct FNsTweenInterpolator
{
    /** Linearly interpolates between two values of the same type. */
    static TValue Lerp(const TValue& A, const TValue& B, float Alpha)
    {
        return FMath::Lerp(A, B, Alpha);
    }
};

/** Interpolator specialization for FVector2D values. */
template <>
struct FNsTweenInterpolator<FVector2D>
{
    /** Interpolates between two 2D vectors using component-wise lerp. */
    static FVector2D Lerp(const FVector2D& A, const FVector2D& B, float Alpha)
    {
        return A + (B - A) * Alpha;
    }
};

/** Interpolator specialization for FRotator values. */
template <>
struct FNsTweenInterpolator<FRotator>
{
    /** Interpolates between two rotations via spherical linear interpolation. */
    static FRotator Lerp(const FRotator& A, const FRotator& B, float Alpha)
    {
        const float T = FMath::Clamp(Alpha, 0.f, 1.f);
        const FQuat QA = A.Quaternion();
        const FQuat QB = B.Quaternion();
        const FQuat Q = FQuat::Slerp(QA, QB, T).GetNormalized();
        return Q.Rotator();
    }
};

/** Interpolator specialization for FTransform values. */
template <>
struct FNsTweenInterpolator<FTransform>
{
    /** Interpolates between two transforms with blend semantics. */
    static FTransform Lerp(const FTransform& A, const FTransform& B, float Alpha)
    {
        FTransform Result;
        Result.Blend(A, B, Alpha);
        return Result;
    }
};

/** Interpolator specialization for FLinearColor values. */
template <>
struct FNsTweenInterpolator<FLinearColor>
{
    /** Interpolates between two linear colors using HSV blending. */
    static FLinearColor Lerp(const FLinearColor& A, const FLinearColor& B, float Alpha)
    {
        return FLinearColor::LerpUsingHSV(A, B, Alpha);
    }
};

/** Interpolator specialization for FQuat values. */
template <>
struct FNsTweenInterpolator<FQuat>
{
    /** Interpolates between two quaternions via spherical linear interpolation. */
    static FQuat Lerp(const FQuat& A, const FQuat& B, float Alpha)
    {
        const float T = FMath::Clamp(Alpha, 0.f, 1.f);
        return FQuat::Slerp(A, B, T).GetNormalized();
    }
};

