#ifndef UTILS_ELEMENTUTILS_HPP_DEFINED
#define UTILS_ELEMENTUTILS_HPP_DEFINED

#include "entities/Element.hpp"
#include "formats/FormatTypes.hpp"
#include "formats/osm/OsmDataContext.hpp"
#include "index/StringTable.hpp"

#include <algorithm>
#include <cstdint>

namespace utymap { namespace utils {

// Sets tags to element.
inline void setTags(utymap::index::StringTable& stringTable,
                    utymap::entities::Element& element,
                    const utymap::formats::Tags& tags)
{
    for (const auto& tag : tags) {
        std::uint32_t key = stringTable.getId(tag.key);
        std::uint32_t value = stringTable.getId(tag.value);
        element.tags.push_back(utymap::entities::Tag(key, value));
    }
    stringTable.flush();
    // NOTE: tags should be sorted to speed up mapcss styling
    std::sort(element.tags.begin(), element.tags.end());
}

// Convert format specific tags to entity ones.
inline std::vector<utymap::entities::Tag> convertTags(utymap::index::StringTable& stringTable, 
                                                      const utymap::formats::Tags& tags)
{
    std::vector<utymap::entities::Tag> convertedTags(tags.size());
    std::transform(tags.begin(), tags.end(), convertedTags.begin(), [&](const utymap::formats::Tag& tag) {
        return utymap::entities::Tag(stringTable.getId(tag.key), stringTable.getId(tag.value));
    });

    std::sort(convertedTags.begin(), convertedTags.end());

    return std::move(convertedTags);
}

inline bool hasTag(std::uint32_t key,
                   std::uint32_t value,
                   const std::vector<utymap::entities::Tag>& tags)
{
    auto begin = tags.begin();
    auto end = tags.end();

    while (begin < end) {
        const auto middle = begin + (std::distance(begin, end) / 2);
        if (middle->key == key)
            return middle->value == value;
        else if (middle->key > key)
            end = middle;
        else
            begin = middle + 1;
    }
    return false;
}

inline std::string getTagValue(std::uint32_t key, 
                               const std::vector<utymap::entities::Tag>& tags,
                               utymap::index::StringTable& stringTable)
{
    auto begin = tags.begin();
    auto end = tags.end();
    while (begin < end) {
        const auto middle = begin + (std::distance(begin, end) / 2);
        if (middle->key == key) 
            return stringTable.getString(middle->value);
        else if (middle->key > key)
            end = middle;
        else
            begin = middle + 1;
    }

    throw std::domain_error("Cannot find tag:" + stringTable.getString(key));
}

// Gets mesh name
inline std::string getMeshName(const std::string& prefix, const utymap::entities::Element& element) {
    return prefix + std::to_string(element.id);
}

template <typename T>
static void visitRelationMembers(const utymap::formats::OsmDataContext& context,
                                 const utymap::formats::RelationMembers& members,
                                 T& visitor)
{
    for (const auto& member : members) {
        if (member.type == "n") {
            auto nodePair = context.nodeMap.find(member.refId);
            if (nodePair != context.nodeMap.end())
                visitor.visit(nodePair);
        }
        else if (member.type == "w") {
            auto areaPair = context.areaMap.find(member.refId);
            if (areaPair != context.areaMap.end()) {
                visitor.visit(areaPair);
            }
            else {
                auto wayPair = context.wayMap.find(member.refId);
                if (wayPair != context.wayMap.end())
                    visitor.visit(wayPair);
            }
        }
        else {
            auto relationPair = context.relationMap.find(member.refId);
            if (relationPair != context.relationMap.end())
                visitor.visit(relationPair, member.role);
        }
    }
}

}}

#endif // UTILS_ELEMENTUTILS_HPP_DEFINED
