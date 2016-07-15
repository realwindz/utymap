#ifndef EXPORTELEMENTVISITOR_HPP_DEFINED
#define EXPORTELEMENTVISITOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "Callbacks.hpp"
#include "GeoCoordinate.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Area.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleProvider.hpp"

#include <string>
#include <vector>

// Exports elements to external code using element callback.
struct ExportElementVisitor : public utymap::entities::ElementVisitor
{
    using Tags = std::vector<utymap::formats::Tag>;
    using Coordinates = std::vector<utymap::GeoCoordinate>;

    ExportElementVisitor(utymap::index::StringTable& stringTable,
                        utymap::mapcss::StyleProvider& styleProvider,
                        int levelOfDetail,
                        OnElementLoaded* elementCallback) :
    stringTable_(stringTable), styleProvider_(styleProvider), levelOfDetail_(levelOfDetail), elementCallback_(elementCallback)
    {
    }

    void visitNode(const utymap::entities::Node& node)
    {
        visitElement(node, Coordinates{ node.coordinate });
    }

    void visitWay(const utymap::entities::Way& way)
    {
        visitElement(way, way.coordinates);
    }

    void visitArea(const utymap::entities::Area& area)
    {
        visitElement(area, area.coordinates);
    }

    void visitRelation(const utymap::entities::Relation& relation)
    {
        // TODO not supported yet.
    }
private:

    void visitElement(const utymap::entities::Element& element, const Coordinates& coordinates)
    {
        // convert tags
        std::vector<const char*> ctags;
        tagStrings_.reserve(element.tags.size() * 2);
        ctags.reserve(element.tags.size() * 2);
        for (std::size_t i = 0; i < element.tags.size(); ++i) {
            const utymap::entities::Tag& tag = element.tags[i];
            tagStrings_.push_back(stringTable_.getString(tag.key));
            tagStrings_.push_back(stringTable_.getString(tag.value));
            ctags.push_back(tagStrings_[tagStrings_.size() - 2].c_str());
            ctags.push_back(tagStrings_[tagStrings_.size() - 1].c_str());
        }
        // convert geometry
        std::vector<double> coords;
        coords.reserve(coordinates.size() * 2);
        for (std::size_t i = 0; i < coordinates.size(); ++i) {
            const utymap::GeoCoordinate coordinate = coordinates[i];
            coords.push_back(coordinate.longitude);
            coords.push_back(coordinate.latitude);
        }
        // convert style
        utymap::mapcss::Style style = styleProvider_.forElement(element, levelOfDetail_);
        std::vector<const char*> cstyles;
        styleStrings_.reserve(style.declarations.size() * 2);
        cstyles.reserve(style.declarations.size());
        for (const auto pair : style.declarations) {
            styleStrings_.push_back(stringTable_.getString(pair.first));
            styleStrings_.push_back(*pair.second->value());
            cstyles.push_back(styleStrings_[styleStrings_.size() - 2].c_str());
            cstyles.push_back(styleStrings_[styleStrings_.size() - 1].c_str());
        }

        elementCallback_(element.id,
            ctags.data(), static_cast<int>(ctags.size()),
            coords.data(), static_cast<int>(coords.size()),
            cstyles.data(), static_cast<int>(cstyles.size()));

        // NOTE clear vectors after raw array data is consumed by external code
        tagStrings_.clear();
        styleStrings_.clear();
    }
    utymap::index::StringTable& stringTable_;
    utymap::mapcss::StyleProvider& styleProvider_;
    int levelOfDetail_;
    OnElementLoaded* elementCallback_;
    std::vector<std::string> tagStrings_;   // holds temporary tag strings
    std::vector<std::string> styleStrings_; // holds temporary style strings
};

#endif // EXPORTELEMENTVISITOR_HPP_DEFINED
