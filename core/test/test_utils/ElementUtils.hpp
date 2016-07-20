#ifndef TEST_ELEMENTUTILS_HPP_DEFINED
#define TEST_ELEMENTUTILS_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleProvider.hpp"

#include <cstdint>
#include <string>
#include <initializer_list>

class ElementUtils
{
public:

    template <typename T>
    static T createElement(utymap::index::StringTable& stringTable,
                           std::initializer_list<std::pair<const char*, const char*>> tags)
    {
        T t;
        for (const auto& pair : tags) {
            t.tags.push_back(createTag(stringTable, pair.first, pair.second));
        }
        return std::move(t);
    }

    template <typename T>
    static T createElement(utymap::index::StringTable& stringTable,
                           std::initializer_list<std::pair<const char*, const char*>> tags,
                           std::initializer_list<std::pair<double, double>> geometry)
    {
        T t = createElement<T>(stringTable, tags);
        t.coordinates.reserve(geometry.size());;
        for (const auto& pair : geometry) {
            t.coordinates.push_back(utymap::GeoCoordinate(pair.first, pair.second));
        }
        return std::move(t);
    }

    static utymap::entities::Tag createTag(utymap::index::StringTable& stringTable, 
                                           const std::string& key, 
                                           const std::string& value)
    {
        uint32_t k = stringTable.getId(key);
        uint32_t v = stringTable.getId(value);
        return utymap::entities::Tag(k, v);
    }
};


#endif // TEST_ELEMENTUTILS_HPP_DEFINED
