#include "entities/ElementVisitor.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/GradientUtils.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

namespace {

// Contains operation types supported by mapcss parser.
enum OpType { Exists, Equals, NotEquals };

struct ConditionType
{
    uint32_t key;
    uint32_t value;
    OpType type;
};

struct Filter
{
    std::vector<ConditionType> conditions;
    std::unordered_map<uint32_t, Style::value_type> declarations;
};

// key: level of detais, value: filters for specific element type.
typedef std::unordered_map<int, std::vector<Filter>> FilterMap;

struct FilterCollection
{
    FilterMap nodes;
    FilterMap ways;
    FilterMap areas;
    FilterMap relations;
    FilterMap canvases;
};

class StyleBuilder : public ElementVisitor
{
    typedef std::vector<Tag>::const_iterator TagIterator;
public:

    StyleBuilder(std::vector<Tag> tags, StringTable& stringTable,
                 const FilterCollection& filters, int levelOfDetails, bool onlyCheck = false) :
            filters_(filters),
            levelOfDetails_(levelOfDetails),
            onlyCheck_(onlyCheck),
            canBuild_(false),
            style_(tags, stringTable)
    {
    }

    void visitNode(const Node& node) { checkOrBuild(node.tags, filters_.nodes); }

    void visitWay(const Way& way) { checkOrBuild(way.tags, filters_.ways); }

    void visitArea(const Area& area) { checkOrBuild(area.tags, filters_.areas); }

    void visitRelation(const Relation& relation) { checkOrBuild(relation.tags, filters_.relations); }

    inline bool canBuild() { return canBuild_; }

    inline Style build()
    {
        if (onlyCheck_)
            throw std::domain_error("Cannot build feature for check mode.");

        return style_;
    }

private:

    inline void checkOrBuild(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        if (onlyCheck_)
            check(tags, filters);
        else
            build(tags, filters);
    }

    // checks tag's value assuming that the key is already checked.
    inline bool match_tag(const Tag& tag, const ConditionType& condition)
    {
        switch (condition.type) {
            case OpType::Exists:
                return true;
            case OpType::Equals:
                return tag.value == condition.value;
            case OpType::NotEquals:
                return tag.value != condition.value;
        }
        return false;
    }

    // tries to find tag which satisfy condition using binary search.
    bool match_tags(TagIterator begin, TagIterator end, const ConditionType& condition)
    {
        while (begin < end) {
            const TagIterator middle = begin + (std::distance(begin, end) / 2);
            if (middle->key == condition.key)
                return match_tag(*middle, condition);
            else if (middle->key > condition.key)
                end = middle;
            else
                begin = middle + 1;
        }
        return false;
    }

    // Builds style object. More expensive to call than check.
    void build(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                bool isMatched = true;
                for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                    isMatched &= match_tags(tags.cbegin(), tags.cend(), *it);
                }
                // merge declarations to style
                if (isMatched) {
                    canBuild_ = true;
                    for (const auto& d : filter.declarations) {
                        style_.put(d.second);
                    }
                }
            }
        }
    }

    // Just checks whether style can be created without constructing actual style.
    void check(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                bool isMatched = true;
                for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                    isMatched &= match_tags(tags.cbegin(), tags.cend(), *it);
                }
                if (isMatched) {
                    canBuild_ = true;
                    return;
                }
            }
        }
    }

    const FilterCollection &filters_;
    int levelOfDetails_;
    bool onlyCheck_;
    bool canBuild_;
    Style style_;
};

}

// Converts mapcss stylesheet to index optimized representation to speed search query up.
class StyleProvider::StyleProviderImpl
{
public:

    FilterCollection filters;
    StringTable& stringTable;
    std::unordered_map<std::string, std::shared_ptr<const ColorGradient>> gradients;

    StyleProviderImpl(const StyleSheet& stylesheet, StringTable& stringTable) :
        stringTable(stringTable),
        filters(),
        gradients()
    {
        filters.nodes.reserve(24);
        filters.ways.reserve(24);
        filters.areas.reserve(24);
        filters.relations.reserve(24);
        filters.canvases.reserve(24);

        for (const Rule& rule : stylesheet.rules) {
            for (const Selector& selector : rule.selectors) {
                for (const std::string& name : selector.names) {
                    FilterMap* filtersPtr = nullptr;
                    if (name == "node") filtersPtr = &filters.nodes;
                    else if (name == "way") filtersPtr = &filters.ways;
                    else if (name == "area") filtersPtr = &filters.areas;
                    else if (name == "relation") filtersPtr = &filters.relations;
                    else if (name == "canvas") filtersPtr = &filters.canvases;
                    else
                        throw std::domain_error("Unexpected selector name:" + name);

                    Filter filter = Filter();
                    filter.conditions.reserve(selector.conditions.size());
                    for (const Condition& condition : selector.conditions) {
                        ConditionType c;
                        if (condition.operation == "") c.type = OpType::Exists;
                        else if (condition.operation == "=") c.type = OpType::Equals;
                        else if (condition.operation == "!=") c.type = OpType::NotEquals;
                        else
                            throw std::domain_error("Unexpected condition operation:" + condition.operation);

                        c.key = stringTable.getId(condition.key);
                        c.value = stringTable.getId(condition.value);
                        filter.conditions.push_back(c);
                    }

                    filter.declarations.reserve(rule.declarations.size());
                    for (auto i = 0; i < rule.declarations.size(); ++i) {
                        Declaration declaration = rule.declarations[i];
                        uint32_t key = stringTable.getId(declaration.key);

                        if (utymap::utils::GradientUtils::isGradient(declaration.value))
                            addGradient(declaration.value);

                        filter.declarations[key] = Style::value_type(new StyleDeclaration(key, declaration.value));
                    }

                    std::sort(filter.conditions.begin(), filter.conditions.end(),
                        [](const ConditionType& c1, const ConditionType& c2) { return c1.key > c2.key; });
                    for (int i = selector.zoom.start; i <= selector.zoom.end; ++i) {
                        (*filtersPtr)[i].push_back(filter);
                    }
                }
            }
        }
    }

    inline void addGradient(const std::string& key)
    {
        if (gradients.find(key) == gradients.end()) {
            auto gradient = utymap::utils::GradientUtils::parseGradient(key);
            if (!gradient->empty())
                gradients[key] = gradient;
        }
    }

    inline std::shared_ptr<const ColorGradient> getGradient(const std::string& key)
    {
        auto gradientPair = gradients.find(key);
        if (gradientPair == gradients.end()) {
            auto gradient = utymap::utils::GradientUtils::parseGradient(key);
            if (gradient->empty())
                throw MapCssException("Invalid gradient: " + key);
            // TODO store it in map in thread safe way
            return gradient;
        }
        return gradientPair->second;
    }
};

StyleProvider::StyleProvider(const StyleSheet& stylesheet, StringTable& stringTable) :
    pimpl_(new StyleProvider::StyleProviderImpl(stylesheet, stringTable))
{
}

StyleProvider::~StyleProvider()
{
}

bool StyleProvider::hasStyle(const utymap::entities::Element& element, int levelOfDetails) const
{
    StyleBuilder builder(element.tags, pimpl_->stringTable, pimpl_->filters, levelOfDetails, true);
    element.accept(builder);
    return builder.canBuild();
}

Style StyleProvider::forElement(const Element& element, int levelOfDetails) const
{
    StyleBuilder builder(element.tags, pimpl_->stringTable, pimpl_->filters, levelOfDetails);
    element.accept(builder);
    return std::move(builder.build());
}

Style StyleProvider::forCanvas(int levelOfDetails) const
{
    Style style({}, pimpl_->stringTable);
    for (const auto &filter : pimpl_->filters.canvases[levelOfDetails]) {
        for (const auto &declaration : filter.declarations) {
            style.put(declaration.second);
        }
    }
    return std::move(style);
}

std::shared_ptr<const ColorGradient> StyleProvider::getGradient(const std::string& key) const
{
    return pimpl_->getGradient(key);
}
