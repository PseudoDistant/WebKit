/*
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
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

#include "config.h"
#include "SourceAlpha.h"

#include "Color.h"
#include "Filter.h"
#include "GraphicsContext.h"
#include <wtf/text/TextStream.h>

namespace WebCore {

Ref<SourceAlpha> SourceAlpha::create(FilterEffect& sourceEffect)
{
    return adoptRef(*new SourceAlpha(sourceEffect));
}

SourceAlpha::SourceAlpha(FilterEffect& sourceEffect)
    : FilterEffect(FilterEffect::Type::SourceAlpha)
{
    setOperatingColorSpace(sourceEffect.operatingColorSpace());
    inputEffects().append(&sourceEffect);
}

void SourceAlpha::determineAbsolutePaintRect(const Filter& filter)
{
    inputEffect(0)->determineAbsolutePaintRect(filter);
    setAbsolutePaintRect(inputEffect(0)->absolutePaintRect());
}

bool SourceAlpha::platformApplySoftware(const Filter&)
{
    ImageBuffer* resultImage = createImageBufferResult();
    if (!resultImage)
        return false;
    
    ImageBuffer* imageBuffer = inputEffect(0)->imageBufferResult();
    if (!imageBuffer)
        return false;

    FloatRect imageRect(FloatPoint(), absolutePaintRect().size());
    GraphicsContext& filterContext = resultImage->context();

    filterContext.fillRect(imageRect, Color::black);
    filterContext.drawImageBuffer(*imageBuffer, IntPoint(), CompositeOperator::DestinationIn);

    return true;
}

TextStream& SourceAlpha::externalRepresentation(TextStream& ts, RepresentationType) const
{
    ts << indent << "[SourceAlpha]\n";
    return ts;
}

} // namespace WebCore
