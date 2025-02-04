/*
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Filter.h"
#include "FilterEffectVector.h"
#include "FloatRect.h"
#include <wtf/Ref.h>
#include <wtf/TypeCasts.h>

namespace WebCore {

class SVGFilterBuilder;
class SVGFilterElement;

class SVGFilter final : public Filter {
public:
    static RefPtr<SVGFilter> create(SVGFilterElement&, SVGFilterBuilder&, const FloatSize& filterScale, const FloatRect& sourceImageRect, const FloatRect& filterRegion, FilterEffect& previousEffect);
    static RefPtr<SVGFilter> create(SVGFilterElement&, SVGFilterBuilder&, const FloatSize& filterScale, const FloatRect& sourceImageRect, const FloatRect& filterRegion, const FloatRect& targetBoundingBox);
    static RefPtr<SVGFilter> create(SVGFilterElement&, SVGFilterBuilder&, const FloatSize& filterScale, const FloatRect& sourceImageRect, const FloatRect& filterRegion, const FloatRect& targetBoundingBox, FilterEffect* previousEffect);

    FloatSize scaledByFilterScale(FloatSize) const final;

    FloatRect targetBoundingBox() const { return m_targetBoundingBox; }
    bool apply() override;

    void setExpression(FilterEffectVector&& expression) { m_expression = WTFMove(expression); }
    RefPtr<FilterEffect> lastEffect() const { return !m_expression.isEmpty() ? m_expression.last() : nullptr; }

private:
    SVGFilter(const FloatSize& filterScale, const FloatRect& sourceImageRect, const FloatRect& targetBoundingBox, const FloatRect& filterRegion, bool effectBBoxMode);

    bool apply(const Filter&) override;
    IntOutsets outsets() const override;
    void clearResult() override;

    FloatRect m_targetBoundingBox;
    bool m_effectBBoxMode;

    FilterEffectVector m_expression;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::SVGFilter)
    static bool isType(const WebCore::Filter& filter) { return filter.isSVGFilter(); }
    static bool isType(const WebCore::FilterFunction& function) { return function.isSVGFilter(); }
SPECIALIZE_TYPE_TRAITS_END()
