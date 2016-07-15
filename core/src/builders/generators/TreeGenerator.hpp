#ifndef BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"
#include "meshing/MeshTypes.hpp"

#include <cmath>

namespace utymap { namespace builders {

// Generates tree.
class TreeGenerator : public AbstractGenerator
{
public:
    TreeGenerator(const utymap::builders::BuilderContext& builderContext,
                  utymap::builders::MeshContext& meshContext) :
            AbstractGenerator(builderContext, meshContext), // ignored
            trunkGenerator(builderContext, meshContext),
            foliageGenerator(builderContext, meshContext),
            position_(),
            trunkHeight_(0),
            trunkRadius_(0),
            foliageRadius_(0),
            foliageHeight_(0)
    {
    }

    // Sets position of tree
    TreeGenerator& setPosition(const utymap::meshing::Vector3& position)
    {
        position_ = position;
        return *this;
    }

    // Sets heigh of trunk
    TreeGenerator& setTrunkHeight(double height)
    {
        trunkHeight_ = height;
        return *this;
    }

    // Sets radius of trunk
    TreeGenerator& setTrunkRadius(double radius)
    {
        trunkRadius_ = radius;
        return *this;
    }

    // Sets radius of foliage
    TreeGenerator& setFoliageRadius(double radius, double height = 1)
    {
        foliageRadius_ = radius;
        foliageHeight_ = height;
        return *this;
    }

    // Sets trunk color.
    TreeGenerator& setTrunkColor(std::shared_ptr<const utymap::mapcss::ColorGradient>& gradient, double noiseFreq)
    {
        trunkGenerator.setColor(gradient, noiseFreq);
        return *this;
    }

    // Sets foliage color.
    TreeGenerator& setFoliageColor(std::shared_ptr<const utymap::mapcss::ColorGradient>& gradient, double noiseFreq)
    {
        foliageGenerator.setColor(gradient, noiseFreq);
        return *this;
    }

    void generate()
    {
        // generate trunk
        trunkGenerator
            .setCenter(position_)
            .setHeight(trunkHeight_)
            .setRadius(trunkRadius_)
            .setMaxSegmentHeight(5)
            .setRadialSegments(7)
            .setVertexNoiseFreq(0.1f)
            .generate();

        // generate foliage
        foliageGenerator
            .setCenter(utymap::meshing::Vector3(
                position_.x,
                position_.y + trunkHeight_ + foliageRadius_,
                position_.z))
            .setRadius(foliageRadius_, foliageHeight_)
            .setRecursionLevel(1)
            .setVertexNoiseFreq(0.1f)
            .generate();
    }

private:
    CylinderGenerator trunkGenerator;
    IcoSphereGenerator foliageGenerator;
    meshing::Vector3 position_;
    double trunkHeight_, trunkRadius_, foliageRadius_, foliageHeight_;
};
}}

#endif // BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
