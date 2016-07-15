set(LIBRARY_NAME UtyMap)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS
    ${MAIN_SOURCE}/formats/osm/pbf/fileformat.proto
    ${MAIN_SOURCE}/formats/osm/pbf/osmformat.proto)

set(HEADER_FILES
        ${PROTO_HDRS}
        ${LIB_SOURCE}/clipper/clipper.hpp
        ${LIB_SOURCE}/hashing/MurmurHash3.h
        ${LIB_SOURCE}/triangle/triangle.h
        ${LIB_SOURCE}/shapefile/shapefil.h
        BoundingBox.hpp
        Exceptions.hpp
        GeoCoordinate.hpp
        LodRange.hpp
        QuadKey.hpp
        builders/BuilderContext.hpp
        builders/ElementBuilder.hpp
        builders/ExternalBuilder.hpp
        builders/QuadKeyBuilder.hpp
        builders/buildings/BuildingBuilder.hpp
        builders/buildings/facades/CylinderFacadeBuilder.hpp
        builders/buildings/facades/FacadeBuilder.hpp
        builders/buildings/facades/FlatFacadeBuilder.hpp
        builders/buildings/facades/SphereFacadeBuilder.hpp
        builders/buildings/roofs/DomeRoofBuilder.hpp
        builders/buildings/roofs/FlatRoofBuilder.hpp
        builders/buildings/roofs/MansardRoofBuilder.hpp
        builders/buildings/roofs/PyramidalRoofBuilder.hpp
        builders/buildings/roofs/RoofBuilder.hpp
        builders/generators/AbstractGenerator.hpp
        builders/generators/CylinderGenerator.hpp
        builders/generators/IcoSphereGenerator.hpp
        builders/generators/TreeGenerator.hpp
        builders/misc/BarrierBuilder.hpp
        builders/poi/TreeBuilder.hpp
        builders/terrain/LineGridSplitter.hpp
        builders/terrain/TerraBuilder.hpp
        builders/terrain/TerraExtras.hpp
        builders/terrain/TerraGenerator.hpp
        entities/Element.hpp
        entities/ElementVisitor.hpp
        entities/Node.hpp
        entities/Relation.hpp
        entities/Way.hpp
        entities/Area.hpp
        formats/FormatTypes.hpp
        formats/osm/BuildingProcessor.hpp
        formats/osm/MultipolygonProcessor.hpp
        formats/osm/OsmDataContext.hpp
        formats/osm/OsmDataVisitor.hpp
        formats/osm/RelationProcessor.hpp
        formats/osm/pbf/OsmPbfParser.hpp
        formats/osm/xml/OsmXmlParser.hpp
        formats/shape/ShapeParser.hpp
        formats/shape/ShapeDataVisitor.hpp
        heightmap/ElevationProvider.hpp
        heightmap/FlatElevationProvider.hpp
        heightmap/SrtmElevationProvider.hpp
        index/ElementGeometryClipper.hpp
        index/ElementStore.hpp
        index/GeoStore.hpp
        index/InMemoryElementStore.hpp
        index/PersistentElementStore.hpp
        index/StringTable.hpp
        mapcss/Color.hpp
        mapcss/ColorGradient.hpp
        mapcss/MapCssParser.hpp
        mapcss/StyleSheet.hpp
        mapcss/Style.hpp
        mapcss/StyleEvaluator.hpp
        mapcss/StyleDeclaration.hpp
        mapcss/StyleProvider.hpp
        meshing/MeshBuilder.hpp
        meshing/MeshTypes.hpp
        meshing/Polygon.hpp
        utils/CoreUtils.hpp
        utils/ElementUtils.hpp
        utils/GeometryUtils.hpp
        utils/GeoUtils.hpp
        utils/GradientUtils.hpp
        utils/MathUtils.hpp
        utils/NoiseUtils.hpp
        utils/SvgBuilder.hpp
        )

add_library(${LIBRARY_NAME}
        ${HEADER_FILES}
        ${PROTO_SRCS}
        ${LIB_SOURCE}/clipper/clipper.cpp
        ${LIB_SOURCE}/hashing/MurmurHash3.cpp
        ${LIB_SOURCE}/triangle/triangle.c
        ${LIB_SOURCE}/shapefile/dbfopen.c
        ${LIB_SOURCE}/shapefile/safileio.c
        ${LIB_SOURCE}/shapefile/shpopen.c
        builders/misc/BarrierBuilder.cpp
        builders/poi/TreeBuilder.cpp
        builders/terrain/TerraBuilder.cpp
        builders/terrain/TerraExtras.cpp
        builders/terrain/TerraGenerator.cpp
        builders/QuadKeyBuilder.cpp
        builders/buildings/BuildingBuilder.cpp
        formats/osm/MultipolygonProcessor.cpp
        formats/osm/OsmDataVisitor.cpp
        index/ElementGeometryClipper.cpp
        index/ElementStore.cpp
        index/GeoStore.cpp
        index/InMemoryElementStore.cpp
        index/PersistentElementStore.cpp
        index/StringTable.cpp
        mapcss/MapCssParser.cpp
        mapcss/StyleEvaluator.cpp
        mapcss/StyleProvider.cpp
        mapcss/StyleSheet.cpp
        meshing/MeshBuilder.cpp
        utils/GradientUtils.cpp
        utils/NoiseUtils.cpp
        )

set_target_properties(${LIBRARY_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(${LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(${LIBRARY_NAME} ${PROTOBUF_LIBRARY} ${ZLIB_LIBRARY})

include_directories(${MAIN_SOURCE} ${LIB_SOURCE} ${CMAKE_CURRENT_BINARY_DIR})
