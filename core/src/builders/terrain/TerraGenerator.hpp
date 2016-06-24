#ifndef BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "builders/terrain/TerraExtras.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace utymap { namespace builders {

// Provides the way to generate terrain mesh.
class TerraGenerator
{
public:

    // Region context encapsulates information about given region.
    struct RegionContext
    {
        const utymap::mapcss::Style& style;
        const std::string prefix;  // Prefix in mapcss.
        const utymap::meshing::MeshBuilder::Options options;

        RegionContext(const utymap::mapcss::Style& style,
                      const std::string& prefix,
                      const utymap::meshing::MeshBuilder::Options& options) :
            style(style), prefix(prefix), options(options)
        {
        }
    };

    // Represents terrain region.
    struct Region
    {
        bool isLayer;
        std::shared_ptr<RegionContext> context; // optional: might be empty if polygon is layer
        ClipperLib::Paths points;
    };

    TerraGenerator(const BuilderContext& context,
                   const utymap::mapcss::Style& style,
                   ClipperLib::ClipperEx& foregroundClipper_);

    // Adds region
    void addRegion(const std::string& type, const Region& region);

    // Generates mesh and calls callback from context.
    void generate(ClipperLib::Path& tileRect);

    // Creates region  context.
    RegionContext createRegionContext(const utymap::mapcss::Style& style,
                                      const std::string& prefix);

private:
    typedef std::vector<utymap::meshing::Vector2> Points;
    typedef std::vector<Region> Regions;
    typedef std::unordered_map<std::string, Regions> Layers;

    // Builds all objects for quadkey organized by layers
    void buildLayers();

    // Builds background as clip area of layers
    void buildBackground(ClipperLib::Path& tileRect);

    void buildFromRegions(const Regions& regions, const RegionContext& regionContext);

    void buildFromPaths(ClipperLib::Paths& paths, const RegionContext& regionContext);

    void populateMesh(ClipperLib::Paths& paths, const RegionContext& regionContext);

    Points restorePoints(const ClipperLib::Path& path);

    void fillMesh(utymap::meshing::Polygon& polygon, const RegionContext& regionContext);

    // Adds extras to mesh, e.g. trees, water surface if meshExtras are specified in options.
    void addExtrasIfNecessary(utymap::meshing::Mesh& mesh,
                              TerraExtras::MeshContext& context,
                              const RegionContext& regionContext);

    void processHeightOffset(const Points& points, const RegionContext& regionContext);

    const BuilderContext& context_;
    const utymap::mapcss::Style& style_;
    ClipperLib::ClipperEx& foregroundClipper_;
    ClipperLib::ClipperEx backGroundClipper_;
    LineGridSplitter splitter_;
    utymap::meshing::Mesh mesh_;
    Layers layers_;
    utymap::meshing::Rectangle rect_;
};

}}

#endif // BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
