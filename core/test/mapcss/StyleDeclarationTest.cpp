#include "entities/Node.hpp"
#include "mapcss/StyleDeclaration.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdio>

#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

namespace {
    struct MapCss_StyleDeclarationFixture
    {
        DependencyProvider dependencyProvider;
    };
}

BOOST_FIXTURE_TEST_SUITE(MapCss_StyleDeclaration, MapCss_StyleDeclarationFixture)

BOOST_AUTO_TEST_CASE(GivenOnlySingleTag_WhenDoubleEvaluate_ThenReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('height')\")");

    double result = styleDeclaration.evaluate<double>(
        ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 0, { { "height", "2.5" } }).tags,
        *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 2.5);
}

BOOST_AUTO_TEST_CASE(GiveTwoTags_WhenDoubleEvaluate_ThenReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('building:height') - tag('roof:height')\")");

    double result = styleDeclaration.evaluate<double>(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
        0, { { "building:height", "10" }, { "roof:height", "2.5" } }).tags,
         *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 7.5);
}

BOOST_AUTO_TEST_CASE(GiveOneTagOneNumber_WhenDoubleEvaluate_ThenReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('building:levels') * 3\")");

    double result = styleDeclaration.evaluate<double>(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(), 
        0, { { "building:levels", "5" } }).tags,
    *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, 15);
}

BOOST_AUTO_TEST_CASE(GiveRawValue_WhenDoubleEvaluate_ThenThrowsException)
{
    StyleDeclaration styleDeclaration(0, "13");

    BOOST_CHECK_THROW(styleDeclaration.evaluate<double>(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
        0, { { "building:levels", "5" } }).tags, *dependencyProvider.getStringTable()),
    utymap::MapCssException);
}

BOOST_AUTO_TEST_CASE(GiveOneTagOneNumber_WhenStringEvaluate_ThenReturnValue)
{
    StyleDeclaration styleDeclaration(0, "eval(\"tag('color')\")");

    std::string result = styleDeclaration.evaluate<std::string>(ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
        0, { { "color", "red" } }).tags,
    *dependencyProvider.getStringTable());

    BOOST_CHECK_EQUAL(result, "red");
}

BOOST_AUTO_TEST_SUITE_END()
