#include "entities/Element.hpp"
#include "LodRange.hpp"
#include "formats/shape/ShapeDataVisitor.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/osm/xml/OsmXmlParser.hpp"
#include "formats/osm/pbf/OsmPbfParser.hpp"
#include "formats/osm/OsmDataVisitor.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "utils/CoreUtils.hpp"

#include <set>
#include <map>
#include <memory>

using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;
using namespace utymap::mapcss;

class GeoStore::GeoStoreImpl
{
    // Prevents to visit element twice if it exists in multiply stores.
    class FilterElementVisitor : public ElementVisitor
    {
    public:
        FilterElementVisitor(const QuadKey& quadKey, const StyleProvider& styleProvider, ElementVisitor& visitor)
                : visitor_(visitor), quadKey_(quadKey), styleProvider_(styleProvider), ids_()
        {
        }

        void visitNode(const Node& node) { visitIfNecessary(node); }

        void visitWay(const Way& way)  { visitIfNecessary(way); }

        void visitArea(const Area& area)  { visitIfNecessary(area); }

        void visitRelation(const Relation& relation)  { visitIfNecessary(relation); }

    private:

        inline void visitIfNecessary(const Element& element)
        {
            if (element.id == 0 || ids_.find(element.id) == ids_.end() ||
                    styleProvider_.hasStyle(element, quadKey_.levelOfDetail)) {
                element.accept(visitor_);
                ids_.insert(element.id);
            }
        }

        const QuadKey& quadKey_;
        const StyleProvider& styleProvider_;
        ElementVisitor& visitor_;

        std::set<std::uint64_t> ids_;
    };

public:

    GeoStoreImpl(StringTable& stringTable) :
        stringTable_(stringTable)
    {
    }

    void registerStore(const std::string& storeKey, const std::shared_ptr<ElementStore>& store)
    {
        storeMap_[storeKey] = store;
    }

    void add(const std::string& storeKey, const Element& element, const LodRange& range, const StyleProvider& styleProvider)
    {
        auto elementStore = storeMap_[storeKey];
        elementStore->store(element, range, styleProvider);
        elementStore->commit();
    }

    void add(const std::string& storeKey, const std::string& path, const QuadKey& quadKey, const StyleProvider& styleProvider)
    {
        auto elementStore = storeMap_[storeKey];
        add(path, styleProvider, [&](Element& element) {
            return elementStore->store(element, quadKey, styleProvider);
        });
        elementStore->commit();
    }

    void add(const std::string& storeKey, const std::string& path, const LodRange& range, const StyleProvider& styleProvider)
    {
        auto elementStore = storeMap_[storeKey];
        add(path, styleProvider, [&](Element& element) {
            return elementStore->store(element, range, styleProvider);
        });
        elementStore->commit();
    }

    void add(const std::string& storeKey, const std::string& path, const BoundingBox& bbox, const LodRange& range, const StyleProvider& styleProvider)
    {
        auto elementStore = storeMap_[storeKey];
        add(path, styleProvider, [&](Element& element) {
            return elementStore->store(element, bbox, range, styleProvider);
        });
        elementStore->commit();
    }

    void add(const std::string& path, const StyleProvider& styleProvider, const std::function<bool(Element&)>& functor)
    {
        switch (getFormatTypeFromPath(path)) {
            case FormatType::Shape: {
                ShapeParser<ShapeDataVisitor> parser;
                ShapeDataVisitor visitor(stringTable_, functor);
                parser.parse(path, visitor);
                visitor.complete();
                break;
            }
            case FormatType::Xml: {
                OsmXmlParser<OsmDataVisitor> parser;
                std::ifstream xmlFile(path);
                OsmDataVisitor visitor(stringTable_, functor);
                parser.parse(xmlFile, visitor);
                visitor.complete();
                break;
            }
            case FormatType::Pbf: {
                OsmPbfParser<OsmDataVisitor> parser;
                std::ifstream pbfFile(path, std::ios::in | std::ios::binary);
                OsmDataVisitor visitor(stringTable_, functor);
                parser.parse(pbfFile, visitor);
                visitor.complete();
                break;
            }
            default:
                throw std::domain_error("Not implemented.");
        }
    }

    void search(const QuadKey& quadKey, const utymap::mapcss::StyleProvider& styleProvider, ElementVisitor& visitor)
    {
        FilterElementVisitor filter(quadKey, styleProvider, visitor);
        for (const auto& pair : storeMap_) {
            pair.second->search(quadKey, filter);
        }
    }

    void search(const GeoCoordinate& coordinate, double radius, const StyleProvider& styleProvider, ElementVisitor& visitor)
    {
        throw std::domain_error("Not implemented");
    }

    bool hasData(const QuadKey& quadKey)
    {
        for (const auto& pair : storeMap_) {
            if (pair.second->hasData(quadKey))
                return true;
        }
        return false;
    }

private:
    StringTable& stringTable_;
    std::map<std::string, std::shared_ptr<ElementStore>> storeMap_;

    FormatType getFormatTypeFromPath(const std::string& path)
    {
        if (utymap::utils::endsWith(path, "pbf"))
            return FormatType::Pbf;
        if (utymap::utils::endsWith(path, "xml"))
            return FormatType::Xml;

        return FormatType::Shape;
    }
};

GeoStore::GeoStore(StringTable& stringTable) : pimpl_(new GeoStore::GeoStoreImpl(stringTable))
{
}

GeoStore::~GeoStore()
{
    // according to docs, should be called only once on app end.
    google::protobuf::ShutdownProtobufLibrary();
}

void utymap::index::GeoStore::registerStore(const std::string& storeKey, const std::shared_ptr<ElementStore>& store)
{
    pimpl_->registerStore(storeKey, store);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const Element& element, const LodRange& range, const StyleProvider& styleProvider)
{
    pimpl_->add(storeKey, element, range, styleProvider);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const std::string& path, const LodRange& range, const StyleProvider& styleProvider)
{
    pimpl_->add(storeKey, path, range, styleProvider);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const std::string& path, const QuadKey& quadKey, const StyleProvider& styleProvider)
{
    pimpl_->add(storeKey, path, quadKey, styleProvider);
}

void utymap::index::GeoStore::add(const std::string &storeKey, const std::string &path, const utymap::BoundingBox& bbox, const LodRange& range, const utymap::mapcss::StyleProvider& styleProvider)
{
    pimpl_->add(storeKey, path, bbox, range, styleProvider);
}

void utymap::index::GeoStore::search(const QuadKey& quadKey, const utymap::mapcss::StyleProvider& styleProvider, ElementVisitor& visitor)
{
    pimpl_->search(quadKey, styleProvider, visitor);
}

void utymap::index::GeoStore::search(const GeoCoordinate& coordinate, double radius, const StyleProvider& styleProvider, ElementVisitor& visitor)
{
    pimpl_->search(coordinate, radius, styleProvider, visitor);
}

bool utymap::index::GeoStore::hasData(const QuadKey& quadKey)
{
    return pimpl_->hasData(quadKey);
}
