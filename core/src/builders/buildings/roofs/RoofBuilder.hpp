#ifndef BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"
#include "mapcss/ColorGradient.hpp"

#include <vector>

namespace utymap { namespace builders {

// Specifies base roof builder functionality.
class RoofBuilder
{
public:
    RoofBuilder(const utymap::builders::BuilderContext& builderContext,
                utymap::builders::MeshContext& meshContext):
        builderContext_(builderContext), meshContext_(meshContext),
        minHeight_(0), height_(0), colorNoiseFreq_(0)
    {
    }

    // Sets height.
    inline RoofBuilder& setHeight(double height) { height_ = height; return *this; }

    // Sets height above ground level.
    inline RoofBuilder& setMinHeight(double minHeight) { minHeight_ = minHeight; return *this; }

    // Sets color.
    inline RoofBuilder& setColor(std::shared_ptr<const utymap::mapcss::ColorGradient>& gradient, double noiseFreq)
    {
        gradient_ = gradient;
        colorNoiseFreq_ = noiseFreq;
        return *this;
    }

    // Builds roof from polygon.
    virtual void build(utymap::meshing::Polygon& polygon) = 0;

protected:
    const utymap::builders::BuilderContext& builderContext_;
    utymap::builders::MeshContext& meshContext_;
    std::shared_ptr<const utymap::mapcss::ColorGradient> gradient_;
    double height_, minHeight_, colorNoiseFreq_;
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
