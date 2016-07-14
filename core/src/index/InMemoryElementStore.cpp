#include "BoundingBox.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/InMemoryElementStore.hpp"

#include <map>

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;
using namespace utymap::mapcss;

namespace {
    struct QuadKeyComparator
    {
        bool operator() (const QuadKey& lhs, const QuadKey& rhs) const
        {
            if (lhs.levelOfDetail == rhs.levelOfDetail) {
                if (lhs.tileX == rhs.tileX) {
                    return lhs.tileY < rhs.tileY;
                }
                return lhs.tileX < rhs.tileX;
            }
            return lhs.levelOfDetail < rhs.levelOfDetail;
        }
    };

    typedef std::vector<std::shared_ptr<Element>> Elements;
    typedef std::map<QuadKey, Elements, QuadKeyComparator> ElementMap;

    class ElementMapVisitor : public ElementVisitor
    {
    public:
        ElementMapVisitor(const QuadKey& quadKey, ElementMap& elementsMap) :
            quadKey_(quadKey), elementsMap_(elementsMap)
        {
        }

        void visitNode(const utymap::entities::Node& node)
        {
            elementsMap_[quadKey_].push_back(std::make_shared<Node>(node));
        }

        void visitWay(const utymap::entities::Way& way)
        {
            elementsMap_[quadKey_].push_back(std::make_shared<Way>(way));
        }

        void visitArea(const utymap::entities::Area& area)
        {
            elementsMap_[quadKey_].push_back(std::make_shared<Area>(area));
        }

        void visitRelation(const utymap::entities::Relation& relation)
        {
            elementsMap_[quadKey_].push_back(std::make_shared<Relation>(relation));
        }
    private:
        const QuadKey& quadKey_;
        ElementMap& elementsMap_;
    };
}

class InMemoryElementStore::InMemoryElementStoreImpl
{
 public:
    ElementMap elementsMap;

    inline ElementMap::const_iterator begin(const utymap::QuadKey& quadKey) const
    {
        return elementsMap.find(quadKey);
    }

    inline ElementMap::const_iterator end() const
    {
        return elementsMap.cend();
    }

    inline bool hasData(const utymap::QuadKey& quadKey) const
    {
        return elementsMap.find(quadKey) != elementsMap.end();
    }  
};

InMemoryElementStore::InMemoryElementStore(StringTable& stringTable) :
    ElementStore(stringTable), pimpl_(new InMemoryElementStore::InMemoryElementStoreImpl())
{
}

InMemoryElementStore::~InMemoryElementStore()
{
}

void InMemoryElementStore::storeImpl(const utymap::entities::Element& element, const QuadKey& quadKey)
{
    ElementMapVisitor visitor(quadKey, pimpl_->elementsMap);
    element.accept(visitor);
}

bool InMemoryElementStore::hasData(const utymap::QuadKey& quadKey) const
{
    return pimpl_->hasData(quadKey);
}

void InMemoryElementStore::search(const utymap::QuadKey& quadKey, utymap::entities::ElementVisitor& visitor)
{
    auto it = pimpl_->begin(quadKey);

    // No elements for this quadkey
    if (it == pimpl_->end())
        return;

    for (const auto& element : it->second) {
        element->accept(visitor);
    }
}

void InMemoryElementStore::commit()
{

}
