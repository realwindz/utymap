#ifndef BUILDERS_TERRAEXTRAS_HPP_DEFINED
#define BUILDERS_TERRAEXTRAS_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>

namespace utymap { namespace builders {

// Provides the way to extend terrain mesh with additional data.
class TerraExtras
{
public:
    // Specifies mesh region which should be used.
    struct MeshContext
    {
        std::size_t startVertex, endVertex;
        std::size_t startTriangle, endTriangle;
        std::size_t startColor, endColor;

        const utymap::mapcss::Style& style;
        const utymap::meshing::MeshBuilder::Options options;

        MeshContext(const utymap::mapcss::Style& style,
                    const utymap::meshing::MeshBuilder::Options& options) :
            startVertex(0), endVertex(0), 
            startTriangle(0), endTriangle(0),
            startColor(0), endColor(0),
            style(style), options(options)
        {
        }
    };

    // Specifies Extras function signature.
    typedef std::function<void(const utymap::builders::BuilderContext&, utymap::meshing::Mesh&, const MeshContext&)> ExtrasFunc;

    // Extends mesh with trees.
    static void addForest(const utymap::builders::BuilderContext& builderContext, 
                          utymap::meshing::Mesh&, 
                          const MeshContext& meshContext);

    // Extends mesh with water surface.
    static void addWater(const utymap::builders::BuilderContext& builderContext,
                         utymap::meshing::Mesh&,
                         const MeshContext& meshContext);
};

}}
#endif // BUILDERS_TERRAEXTRAS_HPP_DEFINED
