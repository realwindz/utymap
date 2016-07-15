#ifndef MAPCSS_STYLE_HPP_INCLUDED
#define MAPCSS_STYLE_HPP_INCLUDED

#include "Exceptions.hpp"
#include "entities/Element.hpp"
#include "mapcss/StyleDeclaration.hpp"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeoUtils.hpp"

#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>

namespace utymap { namespace mapcss {

// Represents style for element.
struct Style
{
    typedef std::uint32_t key_type;
    typedef std::shared_ptr<utymap::mapcss::StyleDeclaration> value_type;

    std::unordered_map<key_type, value_type> declarations;

    Style(const std::vector<utymap::entities::Tag>& tags,
          utymap::index::StringTable& stringTable)
        : tags_(tags), stringTable_(stringTable), declarations()
    {
    }

    inline bool has(key_type key) const
    {
        return declarations.find(key) != declarations.end();
    }

    inline bool has(key_type key, const std::string& value) const
    {
        auto it = declarations.find(key);
        return it != declarations.end() && *it->second->value() == value;
    }

    inline void put(const value_type& declaration)
    {
        declarations[declaration->key()] = declaration;
    }

    inline value_type get(key_type key) const
    {
        auto it = declarations.find(key);
        if (it == declarations.end())
            throw MapCssException(std::string("Cannot find declaration with the key: ") + stringTable_.getString(key));

        return it->second;
    }

    // Gets string by given key. Empty string by default
    inline std::shared_ptr<std::string> getString(const std::string& key) const
    {
        key_type keyId = stringTable_.getId(key);
        return getString(keyId);
    }
    
    // Gets string by given key. Empty string by default
    inline std::shared_ptr<std::string> getString(key_type keyId) const
    {
        if (!has(keyId))
            return std::make_shared<std::string>("");

        auto declaration = get(keyId);

        return declaration->isEval()
               ? std::make_shared<std::string>(declaration->evaluate<std::string>(tags_, stringTable_))
               : declaration->value();
    }

    // Gets double value or zero.
    inline double getValue(const std::string& key,
                           double size = 1,
                           const utymap::GeoCoordinate& coordinate = GeoCoordinate()) const
    {
        key_type keyId = stringTable_.getId(key);
        return getValue(keyId, size, coordinate);
    }

    // Gets double value or zero.
    inline double getValue(key_type keyId,
                           double size = 1,
                           const utymap::GeoCoordinate& coordinate = GeoCoordinate()) const
    {
        if (!has(keyId))
            return 0;

        auto declaration = get(keyId);
        auto rawValue = declaration->value();
        char dimen = (*rawValue)[rawValue->size() - 1];

        if (dimen == 'm') {
            double value = utymap::utils::parseDouble(rawValue->substr(0, rawValue->size() - 1));
            return coordinate.isValid()
                ? utymap::utils::GeoUtils::getOffset(coordinate, value)
                : value;
        }

        // relative to size
        if (dimen == '%') {
            double value = utymap::utils::parseDouble(rawValue->substr(0, rawValue->size() - 1));
            return size * value * 0.01;
        }

        return  declaration->isEval()
                ? declaration->evaluate<double>(tags_, stringTable_)
                : utymap::utils::parseDouble(*rawValue);
    }

private:
    utymap::index::StringTable& stringTable_;
    const std::vector<utymap::entities::Tag> tags_;
};

}}
#endif  // MAPCSS_STYLE_HPP_INCLUDED
