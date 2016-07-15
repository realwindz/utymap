#ifndef BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "mapcss/ColorGradient.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/NoiseUtils.hpp"

#include <algorithm>

namespace utymap { namespace builders {

// Specifies basic behaviour of mesh generators.
class AbstractGenerator
{
public:

    AbstractGenerator(const utymap::builders::BuilderContext& builderContext,
                      utymap::builders::MeshContext& meshContext):
        builderContext_(builderContext),
        meshContext_(meshContext),
        options_(0, 0,  0, 0, nullptr),
        vertNoiseFreq_(0),
        colorNoiseFreq(0)
    {
    }
    // Generates mesh data and updates given mesh.
    virtual void generate() = 0;

    // Sets vertex noise frequency
    AbstractGenerator& setVertexNoiseFreq(double vertNoiseFreq)
    {
        vertNoiseFreq_ = vertNoiseFreq;
        return *this;
    }

    // Sets color
    AbstractGenerator& setColor(std::shared_ptr<const utymap::mapcss::ColorGradient>& gradient,
                                double noiseFreq)
    {
        options_.gradient = gradient;
        options_.colorNoiseFreq = colorNoiseFreq;
        return *this;
    }

protected:

    // Adds triangle to mesh.
    void addTriangle(const utymap::meshing::Vector3& v0,
                     const utymap::meshing::Vector3& v1,
                     const utymap::meshing::Vector3& v2,
                     bool hasBackSide = false) const
    {
        double noise = std::abs(vertNoiseFreq_) < 1E-5
                       ? utymap::utils::NoiseUtils::perlin2D(v0.x, v0.z, vertNoiseFreq_)
                       : 0;

        builderContext_.meshBuilder.addTriangle(meshContext_.mesh,
                                 utymap::meshing::Vector3(v0.x + noise, v0.y + noise, v0.z + noise),
                                 utymap::meshing::Vector3(v1.x + noise, v1.y + noise, v1.z + noise),
                                 utymap::meshing::Vector3(v2.x + noise, v2.y + noise, v2.z + noise),
                                 options_,
                                 hasBackSide);
    }

private:
    const utymap::builders::BuilderContext& builderContext_;
    utymap::builders::MeshContext& meshContext_;
    utymap::meshing::MeshBuilder::Options options_;
    double vertNoiseFreq_, colorNoiseFreq;
};

}}

#endif // BUILDERS_GENERATORS_ABSTRACTGENERATOR_HPP_DEFINED
