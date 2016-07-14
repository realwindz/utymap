#ifndef INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED
#define INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store elements in persistent store.
class PersistentElementStore : public ElementStore
{
public:
    PersistentElementStore(const std::string& path,
                           utymap::index::StringTable& stringTable);

    ~PersistentElementStore();

    void search(const utymap::QuadKey& quadKey, 
                utymap::entities::ElementVisitor& visitor);

    bool hasData(const utymap::QuadKey& quadKey) const;

    void commit();

protected:
    void storeImpl(const utymap::entities::Element& element, const utymap::QuadKey& quadKey);

private:
    class PersistentElementStoreImpl;
    std::unique_ptr<PersistentElementStoreImpl> pimpl_;
};

}}

#endif // INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED
