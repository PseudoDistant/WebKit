/*
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Color.h"
#include "FilterEffect.h"

namespace WebCore {
    
class FEDropShadow : public FilterEffect {
public:
    static Ref<FEDropShadow> create(float stdX, float stdY, float dx, float dy, const Color& shadowColor, float shadowOpacity);

    float stdDeviationX() const { return m_stdX; }
    void setStdDeviationX(float stdX) { m_stdX = stdX; }

    float stdDeviationY() const { return m_stdY; }
    void setStdDeviationY(float stdY) { m_stdY = stdY; }

    float dx() const { return m_dx; }
    void setDx(float dx) { m_dx = dx; }

    float dy() const { return m_dy; }
    void setDy(float dy) { m_dy = dy; }

    const Color& shadowColor() const { return m_shadowColor; } 
    void setShadowColor(const Color& shadowColor) { m_shadowColor = shadowColor; }

    float shadowOpacity() const { return m_shadowOpacity; }
    void setShadowOpacity(float shadowOpacity) { m_shadowOpacity = shadowOpacity; }

private:
    FEDropShadow(float stdX, float stdY, float dx, float dy, const Color& shadowColor, float shadowOpacity);

    void determineAbsolutePaintRect(const Filter&) override;

    IntOutsets outsets() const override;

    bool platformApplySoftware(const Filter&) override;

    WTF::TextStream& externalRepresentation(WTF::TextStream&, RepresentationType) const override;

    float m_stdX;
    float m_stdY;
    float m_dx;
    float m_dy;
    Color m_shadowColor;
    float m_shadowOpacity;
};
    
} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_FILTER_EFFECT(FEDropShadow)
