#ifndef TESTS_FORMATS_SHAPEDATAVISITOR_HPP_DEFINED
#define TESTS_FORMATS_SHAPEDATAVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "formats/FormatTypes.hpp"

#include <cstdint>

using namespace utymap::formats;

struct TestShapeDataVisitor
{
    int bounds;
    int nodes;
    int ways;
    int relations;

    utymap::GeoCoordinate lastCoordinate;
    Coordinates lastCoordinates;
    PolygonMembers lastMembers;
    Tags lastTags;
    bool isRing;

    TestShapeDataVisitor() : bounds(0), nodes(0), ways(0), relations(0),
        lastCoordinate(), lastCoordinates(), lastTags(), isRing() {}

    void visitBounds(utymap::BoundingBox bbox)
    {
        bounds++;
    }

    void visitNode(utymap::GeoCoordinate& coordinate, Tags& tags)
    {
        lastCoordinate = coordinate;
        lastTags = tags;
        nodes++;
    }

    void visitWay(Coordinates& coordinates, Tags& tags, bool isRing)
    {
        this->isRing = isRing;
        lastCoordinates = coordinates;
        lastTags = tags;
        ways++;
    }

    void visitRelation(PolygonMembers& members, Tags& tags)
    {
        lastMembers = members;
        lastTags = tags;
        relations++;
    }
};

#endif // TESTS_FORMATS_SHAPEDATAVISITOR_HPP_DEFINED
