#include "GeoCoordinate.hpp"
#include "builders/MeshContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "builders/buildings/BuildingBuilder.hpp"
#include "builders/buildings/facades/CylinderFacadeBuilder.hpp"
#include "builders/buildings/facades/FlatFacadeBuilder.hpp"
#include "builders/buildings/facades/SphereFacadeBuilder.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/buildings/roofs/PyramidalRoofBuilder.hpp"
#include "builders/buildings/roofs/MansardRoofBuilder.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/ElementUtils.hpp"
#include "utils/GradientUtils.hpp"

#include <exception>
#include <unordered_map>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::index;
using namespace utymap::utils;

namespace {

    const std::string RoofTypeKey = "roof-type";
    const std::string RoofHeightKey = "roof-height";
    const std::string RoofColorKey = "roof-color";

    const std::string FacadeTypeKey = "facade-type";
    const std::string FacadeColorKey = "facade-color";

    const std::string HeightKey = "height";
    const std::string MinHeightKey = "min-height";

    const std::string MeshNamePrefix = "building:";

    // Defines roof builder which does nothing.
    class EmptyRoofBuilder : public RoofBuilder {
    public:
        EmptyRoofBuilder(const BuilderContext& bc, MeshContext& mc)
            : RoofBuilder(bc, mc) { }
        void build(utymap::meshing::Polygon&) {}
    };

    typedef std::function<std::shared_ptr<RoofBuilder>(const BuilderContext&, MeshContext&)> RoofBuilderFactory;
    std::unordered_map<std::string, RoofBuilderFactory> RoofBuilderFactoryMap =
    {
        {
            "none",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<EmptyRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "flat",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<FlatRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "dome",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<DomeRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "pyramidal",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<PyramidalRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "mansard",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<MansardRoofBuilder>(builderContext, meshContext);
            }
        }
    };

    typedef std::function<std::shared_ptr<FacadeBuilder>(const BuilderContext&, MeshContext&)> FacadeBuilderFactory;
    std::unordered_map<std::string, FacadeBuilderFactory> FacadeBuilderFactoryMap =
    {
        {
            "flat",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<FlatFacadeBuilder>(builderContext, meshContext);
            }
        },
        {
            "cylinder",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<CylinderFacadeBuilder>(builderContext, meshContext);
            }
        },
        {
            "sphere",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<SphereFacadeBuilder>(builderContext, meshContext);
            }
        }
    };

    // Creates points for polygon
    std::vector<Vector2> toPoints(const std::vector<GeoCoordinate>& coordinates)
    {
        std::vector<Vector2> points;
        points.reserve(coordinates.size());
        for (const auto& coordinate : coordinates) {
            points.push_back(Vector2(coordinate.longitude, coordinate.latitude));
        }

        return std::move(points);
    }

    // Responsible for processing multipolygon relation.
    class MultiPolygonVisitor : public ElementVisitor
    {
    public:

        MultiPolygonVisitor(std::shared_ptr<Polygon> polygon)
            : polygon_(polygon) {}

        void visitNode(const utymap::entities::Node& node) { fail(node); }

        void visitWay(const utymap::entities::Way& way) { fail(way); }

        void visitRelation(const utymap::entities::Relation& relation) { fail(relation); }

        void visitArea(const utymap::entities::Area& area)
        {
            if (utymap::utils::isClockwise(area.coordinates))
                polygon_->addContour(toPoints(area.coordinates));
            else
                polygon_->addHole(toPoints(area.coordinates));
        }

    private:
        inline void fail(const utymap::entities::Element& element)
        {
            throw std::domain_error("Unexpected element in multipolygon: " + utymap::utils::toString(element.id));
        }
        std::shared_ptr<Polygon> polygon_;
    };
}

namespace utymap { namespace builders {

class BuildingBuilder::BuildingBuilderImpl : public ElementBuilder
{
public:
    BuildingBuilderImpl(const utymap::builders::BuilderContext& context) :
        ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node&) { }

    void visitWay(const utymap::entities::Way&) { }

    void visitArea(const utymap::entities::Area& area)
    {
        Style style = context_.styleProvider.forElement(area, context_.quadKey.levelOfDetail);

        // NOTE this might happen if relation contains not a building
        if (!isBuilding(style))
            return;

        bool justCreated = ensureContext(area);
        polygon_->addContour(toPoints(area.coordinates));
        build(area, style);

        completeIfNecessary(justCreated);
    }

    void visitRelation(const utymap::entities::Relation& relation)
    {
        if (relation.elements.empty())
            return;

        bool justCreated = ensureContext(relation);

        Style style = context_.styleProvider.forElement(relation, context_.quadKey.levelOfDetail);

        if (isMultipolygon(style) && isBuilding(style)) {
            MultiPolygonVisitor visitor(polygon_);

            for (const auto& element : relation.elements)
                element->accept(visitor);

            build(relation, style);
        }
        else {
            for (const auto& element : relation.elements)
                element->accept(*this);
        }

        completeIfNecessary(justCreated);
    }

    void complete()
    {
    }

private:

    inline bool ensureContext(const Element& element)
    {
        if (polygon_ == nullptr)
            polygon_ = std::make_shared<Polygon>(1, 0);

        if (mesh_ == nullptr) {
            mesh_ = std::make_shared<Mesh>(utymap::utils::getMeshName(MeshNamePrefix, element));
            return true;
        }

        return false;
    }

    inline void completeIfNecessary(bool justCreated)
    {
        if (justCreated) {
            context_.meshCallback(*mesh_);
            mesh_.reset();
        }
    }

    inline bool isBuilding(const Style& style) const
    {
        return *style.getString("building") == "true";
    }

    inline bool isMultipolygon(const Style& style) const
    {
        return *style.getString("multipolygon") == "true";
    }

    void build(const Element& element, const Style& style)
    {
        MeshContext meshContext(*mesh_, style);

        auto geoCoordinate = GeoCoordinate(polygon_->points[1], polygon_->points[0]);

        double height = style.getValue(HeightKey);
        // NOTE do not allow height to be zero. This might happen due to the issues in input osm data.
        if (height == 0)
            height = 10;

        double minHeight = style.getValue(MinHeightKey);

        double elevation = context_.eleProvider.getElevation(geoCoordinate) + minHeight;

        height -= minHeight;

        // roof
        auto roofType = style.getString(RoofTypeKey);
        double roofHeight = style.getValue(RoofHeightKey);
        auto roofGradient = GradientUtils::evaluateGradient(context_.styleProvider, meshContext.style, element.tags, RoofColorKey);
        auto roofBuilder = RoofBuilderFactoryMap.find(*roofType)->second(context_, meshContext);
        roofBuilder->setHeight(roofHeight);
        roofBuilder->setMinHeight(elevation + height);
        roofBuilder->setColor(roofGradient, 0);
        roofBuilder->build(*polygon_);

        // facade
        auto facadeType = style.getString(FacadeTypeKey);
        auto facadeBuilder = FacadeBuilderFactoryMap.find(*facadeType)->second(context_, meshContext);
        auto facadeGradient = GradientUtils::evaluateGradient(context_.styleProvider, meshContext.style, element.tags, FacadeColorKey);
        facadeBuilder->setHeight(height);
        facadeBuilder->setMinHeight(elevation);
        facadeBuilder->setColor(facadeGradient, 0);
        facadeBuilder->build(*polygon_);

        polygon_.reset();
    }

    std::shared_ptr<Polygon> polygon_;
    std::shared_ptr<Mesh> mesh_;
};

BuildingBuilder::BuildingBuilder(const BuilderContext& context)
    : ElementBuilder(context), pimpl_(new BuildingBuilder::BuildingBuilderImpl(context))
{
}

BuildingBuilder::~BuildingBuilder() { }

void BuildingBuilder::visitNode(const Node&) { }

void BuildingBuilder::visitWay(const Way&) { }

void BuildingBuilder::visitArea(const Area& area)
{
    area.accept(*pimpl_);
}

void BuildingBuilder::complete()
{
    pimpl_->complete();
}

void BuildingBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    relation.accept(*pimpl_);
}

}}
