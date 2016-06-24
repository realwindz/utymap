#include "heightmap/FlatElevationProvider.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "mapcss/ColorGradient.hpp"
#include "meshing/Polygon.hpp"
#include "meshing/MeshBuilder.hpp"

#include <boost/test/unit_test.hpp>

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;

namespace {
    typedef Vector2 DPoint;
    auto colorGradient = std::make_shared<const ColorGradient>(ColorGradient::GradientData{ { 0, Color(0, 0, 0, 0) } });

    struct Meshing_MeshingFixture
    {
        Meshing_MeshingFixture() : eleProvider(), builder(eleProvider)
        {
            BOOST_TEST_MESSAGE("setup fixture");
        }
        ~Meshing_MeshingFixture()       { BOOST_TEST_MESSAGE("teardown fixture"); }

        FlatElevationProvider eleProvider;
        MeshBuilder builder;
    };
}

BOOST_FIXTURE_TEST_SUITE(Meshing_MeshBuilder, Meshing_MeshingFixture)

BOOST_AUTO_TEST_CASE(GivenPolygon_WhenAddPolygon_ThenRefinesCorrectly)
{
    Mesh mesh("");
    Polygon polygon(4, 0);
    polygon.addContour(std::vector<DPoint>
    {
        DPoint(0, 0),
        DPoint(10, 0),
        DPoint(10, 10),
        DPoint(0, 10)
    });

    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    (
        /* area=*/ 5,
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    ));

    BOOST_CHECK_EQUAL(mesh.vertices.size() / 3, 23);
    BOOST_CHECK_EQUAL(mesh.triangles.size() / 3, 34);
}

BOOST_AUTO_TEST_CASE(GivenPolygonWithHole_WhenAddPolygon_ThenRefinesCorrectly)
{
    Mesh mesh("");
    Polygon polygon(8, 1);
    polygon.addContour(std::vector<DPoint>
    {
        DPoint(0, 0),
        DPoint(10, 0),
        DPoint(10, 10),
        DPoint(0, 10)
    });
    polygon.addHole(std::vector<DPoint>
    {
        DPoint(3, 3),
        DPoint(6, 3),
        DPoint(6, 6),
        DPoint(3, 6)
    });

    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    (
        /* area=*/ 1, 
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    ));

    BOOST_CHECK(mesh.vertices.size() > 0);
    BOOST_CHECK(mesh.triangles.size() > 0);
}

BOOST_AUTO_TEST_CASE(GivenPolygonProcessedByGridSplitter_WhenAddPolygon_ThenRefinesCorrectly)
{
    std::vector<Vector2> contour;
    LineGridSplitter splitter;
    int scale = 10;
    splitter.setParams(scale, 1);
    std::vector<IntPoint> inputPoints
    {
        IntPoint(0 * scale, 0 * scale),
        IntPoint(10 * scale, 0 * scale),
        IntPoint(10 * scale, 10 * scale),
        IntPoint(0 * scale, 10 * scale)
    };
    int lastItemIndex = inputPoints.size() - 1;
    for (int i = 0; i <= lastItemIndex; i++) {
        IntPoint start = inputPoints[i];
        IntPoint end = inputPoints[i == lastItemIndex ? 0 : i + 1];
        splitter.split(start, end, contour);
    }

    Polygon polygon(contour.size(), 0);
    polygon.addContour(contour);

    Mesh mesh("");
    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    (
        /* area=*/ 1. / scale,
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    ));

    BOOST_CHECK(mesh.vertices.size() > 0);
    BOOST_CHECK(mesh.triangles.size() > 0);
}

BOOST_AUTO_TEST_CASE(GivenPolygonWithSharePoint_WhenAddPolygon_ThenRefinesCorrectly)
{
    Polygon polygon(8, 0);
    polygon.addContour({ { 0, 0 }, { 10, 0 }, { 10, 10 }, { 0, 10 } });
    polygon.addContour({ { 10, 10 }, { 15, 10 }, { 15, 15 }, { 10, 15 } });
    Mesh mesh("");

    builder.addPolygon(mesh, polygon, MeshBuilder::Options
    (
        /* area=*/ 1. / 1.,
        /* elevation noise frequency*/ 0,
        /* color noise frequency */ 0,
        /* height offset */ 0,
        /* color gradient */ colorGradient
    ));

    BOOST_CHECK(mesh.vertices.size() > 0);
}

BOOST_AUTO_TEST_SUITE_END()
