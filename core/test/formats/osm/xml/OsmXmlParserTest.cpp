#include "BoundingBox.hpp"
#include "formats/osm/xml/OsmXmlParser.hpp"
#include "formats/osm/TestOsmDataVisitor.hpp"
#include "config.hpp"

#include <boost/test/unit_test.hpp>

using namespace utymap::formats;

namespace {
    struct Formats_Osm_Xml_OsmXmlParserFixture
    {
        Formats_Osm_Xml_OsmXmlParserFixture() :
            istream(TEST_XML_FILE, std::ios::in)
        {
        }

        OsmXmlParser<TestOsmDataVisitor> parser;
        TestOsmDataVisitor visitor;
        std::ifstream istream;
    };
}

BOOST_FIXTURE_TEST_SUITE(Formats_Osm_Xml_Parser, Formats_Osm_Xml_OsmXmlParserFixture)

BOOST_AUTO_TEST_CASE(GivenDefaultXml_WhenParserParse_ThenHasExpectedElementCount)
{
    parser.parse(istream, visitor);

    BOOST_CHECK_EQUAL(1, visitor.bounds);
    BOOST_CHECK_EQUAL(7653, visitor.nodes);
    BOOST_CHECK_EQUAL(1116, visitor.ways);
    BOOST_CHECK_EQUAL(92, visitor.relations);
}

BOOST_AUTO_TEST_SUITE_END()
