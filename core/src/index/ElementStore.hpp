#ifndef INDEX_ELEMENTSTORE_HPP_DEFINED
#define INDEX_ELEMENTSTORE_HPP_DEFINED

#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/ElementVisitor.hpp"
#include "LodRange.hpp"
#include "formats/FormatTypes.hpp"
#include "mapcss/StyleProvider.hpp"

#include <cstdint>
#include <memory>

namespace utymap { namespace index {

// Defines API to store elements.
class ElementStore
{
public:
    ElementStore(utymap::index::StringTable& stringTable);

    virtual ~ElementStore();

    // Searches for elements for given quadKey
    virtual void search(const utymap::QuadKey& quadKey,
                        utymap::entities::ElementVisitor& visitor) = 0;

    // Checks whether there is data for given quadkey.
    virtual bool hasData(const utymap::QuadKey& quadKey) const = 0;

    // Stores element in storage in all affected tiles at given level of details range.
    bool store(const utymap::entities::Element& element, 
               const utymap::LodRange& range,
               const utymap::mapcss::StyleProvider& styleProvider);

    // Stores element in storage only in given quadkey.
    bool store(const utymap::entities::Element& element,
               const utymap::QuadKey& quadKey,
               const utymap::mapcss::StyleProvider& styleProvider);

    // Stores element in storage only in given bounding box.
    bool store(const utymap::entities::Element& element,
               const utymap::BoundingBox& bbox,
               const utymap::LodRange& range,
               const utymap::mapcss::StyleProvider& styleProvider);

    // Commits changes done in element store.
    virtual void commit() = 0;

protected:
    // Stores element in given quadkey.
    virtual void storeImpl(const utymap::entities::Element& element, const utymap::QuadKey& quadKey) = 0;

private:
    template <typename Visitor>
    bool store(const utymap::entities::Element& element,
               const utymap::LodRange& range,
               const utymap::mapcss::StyleProvider& styleProvider,
               const Visitor& visitor);

    bool checkSize(const utymap::BoundingBox& quadKeyBBox,
                   const utymap::BoundingBox& elementBbox,
                   double minSize) const;

    std::uint32_t clipKeyId_, skipKeyId_, sizeKeyId_;
};

}}

#endif // INDEX_ELEMENTSTORE_HPP_DEFINED
