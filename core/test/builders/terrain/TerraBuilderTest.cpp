#include "QuadKey.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "heightmap/FlatElevationProvider.hpp"

#include <boost/test/unit_test.hpp>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::meshing;

namespace {
    const std::string stylesheet =
        "canvas|z1 { grid-cell-size: 1%; layer-priority: water; ele-noise-freq: 0.05; color-noise-freq: 0.1; color:gradient(red); max-area: 5%;"
        "water-ele-noise-freq: 0.05; water-color-noise-freq: 0.1; water-color:gradient(red);  water-max-area: 5%;}"
        "area|z1[natural=water] { builders:terrain; terrain-layer:water; }";

    struct Builders_Terrain_TerraBuilderFixture
    {
        DependencyProvider dependencyProvider;
        std::shared_ptr<TerraBuilder> terraBuilder;
    };
}

BOOST_FIXTURE_TEST_SUITE(Builders_Terrain_TerraBuilder, Builders_Terrain_TerraBuilderFixture)

BOOST_AUTO_TEST_CASE(GivenLargeWater_WhenComplete_ThenMeshIsNotEmpty)
{
    bool isCalled = false;
    QuadKey quadKey(1, 0, 0);
    BuilderContext context(quadKey,
        *dependencyProvider.getStyleProvider(stylesheet),
        *dependencyProvider.getStringTable(),
        *dependencyProvider.getElevationProvider(),
        [&](const Mesh& mesh) {
            isCalled = true;
            BOOST_CHECK_GT(mesh.vertices.size(), 0);
            BOOST_CHECK_GT(mesh.triangles.size(), 0);
        }, nullptr);
    TerraBuilder terraBuilder(context);
    ElementUtils::createElement<Area>(*dependencyProvider.getStringTable(),
    { { "natural", "water" } },
    { { 0, 0 }, { 20, 0 }, { 20, 20 }, { 0, 20 } }).accept(terraBuilder);

    terraBuilder.complete();

    BOOST_CHECK(isCalled);
}

BOOST_AUTO_TEST_SUITE_END()
