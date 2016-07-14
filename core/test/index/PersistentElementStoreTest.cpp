#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/PersistentElementStore.hpp"

#include <boost/test/unit_test.hpp>
#include "test_utils/DependencyProvider.hpp"
#include "test_utils/ElementUtils.hpp"

#include <boost/filesystem/operations.hpp>
#include <cstdio>

using namespace utymap;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

namespace {
    const std::string TestZoomDirectory = "1";

    const std::string stylesheet = "area|z1[any],way|z1[any],node|z1[any] { clip: true; }";

    struct Index_PersistentElementStoreFixture 
    {
        Index_PersistentElementStoreFixture() :
            dependencyProvider(),
            elementStore("", *dependencyProvider.getStringTable())
        {
            boost::filesystem::create_directory(TestZoomDirectory);
        }

        ~Index_PersistentElementStoreFixture()
        {
            elementStore.commit();

            boost::filesystem::path dir(TestZoomDirectory);
            for (boost::filesystem::directory_iterator dirEnd, it(dir); it != dirEnd; ++it) {
                boost::filesystem::remove_all(it->path());
            }
            boost::filesystem::remove(TestZoomDirectory);
        }

        DependencyProvider dependencyProvider;
        PersistentElementStore elementStore;
    };

    struct ElementCounter : public ElementVisitor
    {
        int times = 0;
        std::shared_ptr<Element> element;

        void visitNode(const Node& node) 
        { 
            ++times; 
            element = std::make_shared<Node>(node);
        }

        void visitWay(const Way& way)
        {
            ++times;
            element = std::make_shared<Way>(way);
        }
        void visitArea(const Area& area)
        {
            ++times;
            element = std::make_shared<Area>(area);
        }
        void visitRelation(const Relation& relation)
        {
            ++times;
            element = std::make_shared<Relation>(relation);
        }
    };
}

BOOST_FIXTURE_TEST_SUITE(Index_PersistentElementStore, Index_PersistentElementStoreFixture)

BOOST_AUTO_TEST_CASE(GivenNode_WhenStoreAndSearch_ThenItIsStoredAndReadBack)
{
    LodRange range(1, 2);
    QuadKey quadKey(1, 0, 0);
    auto styleProvider = dependencyProvider.getStyleProvider(stylesheet);
    Node node = ElementUtils::createElement<Node>(*dependencyProvider.getStringTable(),
    { { "any", "true" } });
    node.id = 7;
    node.coordinate = { 5, -5 };
    ElementCounter counter;

    elementStore.store(node, range, *styleProvider);
    elementStore.commit();
    elementStore.search(quadKey, counter);

    BOOST_CHECK_EQUAL(counter.times, 1);
    BOOST_CHECK_EQUAL(node.id, counter.element->id);
    BOOST_CHECK_EQUAL(node.tags.size(), counter.element->tags.size());

    auto resultNode = std::dynamic_pointer_cast<Node>(counter.element);
    BOOST_CHECK_EQUAL(node.coordinate.latitude, resultNode->coordinate.latitude);
    BOOST_CHECK_EQUAL(node.coordinate.longitude, resultNode->coordinate.longitude);
}

BOOST_AUTO_TEST_SUITE_END()