#ifndef TESTS_FORMATS_OSMDATAVISITOR_HPP_DEFINED
#define TESTS_FORMATS_OSMDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"

#include <cstdint>

using namespace utymap::formats;

struct TestOsmDataVisitor
{
    int bounds;
    int nodes;
    int ways;
    int relations;

    TestOsmDataVisitor() : bounds(0), nodes(0), ways(0), relations(0) {}

    void visitBounds(utymap::BoundingBox bbox)
    {
        bounds++;
    }

    void visitNode(uint64_t id, utymap::GeoCoordinate& coordinate, Tags& tags)
    {
        nodes++;
    }

    void visitWay(uint64_t id, std::vector<uint64_t>& nodeIds, Tags& tags)
    {
        ways++;
    }

    void visitRelation(uint64_t id, RelationMembers& members, Tags& tags)
    {
        relations++;
    }
};

#endif // TESTS_FORMATS_OSMDATAVISITOR_HPP_DEFINED
