#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "formats/osm/BuildingProcessor.hpp"
#include "formats/osm/MultipolygonProcessor.hpp"
#include "formats/osm/RelationProcessor.hpp"
#include "formats/osm/OsmDataVisitor.hpp"
#include "utils/ElementUtils.hpp"
#include "utils/GeometryUtils.hpp"

#include <algorithm>
#include <vector>

using namespace utymap;
using namespace utymap::formats;
using namespace utymap::entities;
using namespace utymap::index;

namespace {

    std::unordered_set<std::string> AreaKeys
    {
        "building",
        "building:part",
        "landuse",
        "amenity",
        "harbour",
        "historic",
        "leisure",
        "man_made",
        "military",
        "natural",
        "office",
        "place",
        "power",
        "public_transport",
        "shop",
        "sport",
        "tourism",
        "waterway",
        "wetland",
        "water",
        "aeroway",
        "addr:housenumber",
        "addr:housename"
    };

    std::unordered_set<std::string> FalseKeys
    {
        "no", "No", "NO", "false", "False", "FALSE", "0"
    };
}

void OsmDataVisitor::visitBounds(BoundingBox bbox)
{
}

void OsmDataVisitor::visitNode(std::uint64_t id, GeoCoordinate& coordinate, utymap::formats::Tags& tags)
{
    auto node = std::make_shared<Node>();
    node->id = id;
    node->coordinate = coordinate;
    utymap::utils::setTags(stringTable_, *node, tags);
    context_.nodeMap[id] = node;
}

void OsmDataVisitor::visitWay(std::uint64_t id, std::vector<std::uint64_t>& nodeIds, utymap::formats::Tags& tags)
{
    std::vector<GeoCoordinate> coordinates;
    coordinates.reserve(nodeIds.size());
    for (auto nodeId : nodeIds) {
        coordinates.push_back(context_.nodeMap[nodeId]->coordinate);
    }

    if (coordinates.size() > 2 && isArea(tags)) {
        if (coordinates.at(0) == coordinates.at(coordinates.size() - 1)) {
            // NOTE three coordinates are invalid here: skip.
            // TODO should it be considered as way instead?
            if (coordinates.size() == 3)
                return;

            coordinates.pop_back();
        }

        auto area = std::make_shared<Area>();
        area->id = id;
        if (!utymap::utils::isClockwise(coordinates)) {
            std::reverse(coordinates.begin(), coordinates.end());
        }
        area->coordinates = std::move(coordinates);
        utymap::utils::setTags(stringTable_, *area, tags);
        context_.areaMap[id] = area;

    } else {
        auto way = std::make_shared<Way>();
        way->id = id;
        way->coordinates = std::move(coordinates);
        utymap::utils::setTags(stringTable_, *way, tags);
        context_.wayMap[id] = way;
    }
}

void OsmDataVisitor::visitRelation(std::uint64_t id, RelationMembers& members, utymap::formats::Tags& tags)
{
    auto relation = std::make_shared<Relation>();
    relation->id = id;
    relation->tags = utymap::utils::convertTags(stringTable_, tags);

    // NOTE Assume, relation may refer to another relations which are not yet processed.
    // So, store all relation members to resolve them once all relations are visited.
    relationMembers_[id] = members;
    context_.relationMap[id] = relation;
}

bool OsmDataVisitor::isArea(const utymap::formats::Tags& tags) const
{
    for (const auto& tag : tags) {
        if (AreaKeys.find(tag.key) != AreaKeys.end() &&
            FalseKeys.find(tag.value) == FalseKeys.end())
            return true;
    }

    return false;
}

bool OsmDataVisitor::hasTag(const std::string& key, const std::string& value, const std::vector<utymap::entities::Tag>& tags) const
{
    return utymap::utils::hasTag(stringTable_.getId(key), stringTable_.getId(value), tags);
}

void OsmDataVisitor::resolve(Relation& relation)
{
    auto membersPair = relationMembers_.find(relation.id);
    // already resolved
    if (relation.elements.size() == membersPair->second.size())
        return;

    auto resolveFunc = std::bind(&OsmDataVisitor::resolve, this, std::placeholders::_1);

    if (hasTag("type", "multipolygon", relation.tags))
        MultipolygonProcessor(relation, membersPair->second, context_, resolveFunc).process();
    else if (hasTag("type", "building", relation.tags))
        BuildingProcessor(relation, membersPair->second, context_, resolveFunc).process();
    else {
        RelationProcessor(relation, membersPair->second, context_, resolveFunc).process();
    }
}

void OsmDataVisitor::complete()
{
    // All relations are visited can start to resolve them
    for (auto& membersPair : relationMembers_) {
        auto relationPair = context_.relationMap.find(membersPair.first);
        if (relationPair != context_.relationMap.end())
            resolve(*relationPair->second);
    }

    for (const auto& pair : context_.relationMap) {
        add_(*pair.second);
    }

    for (const auto& pair : context_.nodeMap) {
        add_(*pair.second);
    }

    for (const auto& pair : context_.wayMap) {
        add_(*pair.second);
    }

    for (const auto& pair : context_.areaMap) {
        add_(*pair.second);
    }
  
}

OsmDataVisitor::OsmDataVisitor(StringTable& stringTable, std::function<bool(Element&)> add) 
    : stringTable_(stringTable), add_(add), context_()
{
}
