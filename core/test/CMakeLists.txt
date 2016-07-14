find_package(Boost COMPONENTS unit_test_framework filesystem REQUIRED)

enable_testing ()

# Define test assets path
file(READ ${TEST_SOURCE}/config.hpp FILE_CONTENT)
string(REPLACE "_TEST_ASSETS_PATH_" "${TEST_SOURCE}/test_assets/"
        MODIFIED_FILE_CONTENT "${FILE_CONTENT}")
file(WRITE ${TEST_SOURCE}/config.hpp "${MODIFIED_FILE_CONTENT}")

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS
    ${MAIN_SOURCE}/formats/osm/pbf/fileformat.proto
    ${MAIN_SOURCE}/formats/osm/pbf/osmformat.proto)

include_directories(${MAIN_SOURCE}
        ${LIB_SOURCE}
        ${SHARED_SOURCE}
        ${TEST_SOURCE}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        )

add_definitions(-DBOOST_TEST_DYN_LINK)

set(HEADER_FILES
        ${PROTO_HDRS}
        config.hpp
        formats/osm/TestOsmDataVisitor.hpp
        formats/shape/TestShapeDataVisitor.hpp
        test_utils/DependencyProvider.hpp
        test_utils/ElementUtils.hpp
        )

set (TEST Test)

add_executable(${TEST}
        ${PROTO_SRCS}
        main.cpp
        BoundingBoxTest.cpp
        ExportLibTest.cpp
        builders/buildings/BuildingBuilderTest.cpp
        builders/buildings/RoofBuildersTest.cpp
        builders/generators/GeneratorTest.cpp
        builders/poi/TreeBuilderTest.cpp
        builders/misc/BarrierBuilderTest.cpp
        builders/terrain/TerraBuilderTest.cpp
        builders/terrain/LineGridSplitterTest.cpp
        entities/ElementTest.cpp
        formats/shape/ShapeParserTest.cpp
        formats/shape/ShapeDataVisitorTest.cpp
        formats/osm/MultipolygonProcessorTest.cpp
        formats/osm/pbf/OsmPbfParserTest.cpp
        formats/osm/xml/OsmXmlParserTest.cpp
        heightmap/SrtmElevationProviderTest.cpp
        index/ElementStoreTest.cpp
        index/InMemoryElementStoreTest.cpp
        index/PersistentElementStoreTest.cpp
        index/StringTableTest.cpp
        mapcss/MapCssParserTest.cpp
        mapcss/StyleDeclarationTest.cpp
        mapcss/StyleProviderTest.cpp
        mapcss/StyleTest.cpp
        meshing/MeshBuilderTest.cpp
        utils/GeometryUtilsTest.cpp
        utils/GeoUtilsTest.cpp
        utils/GradientUtilsTest.cpp
        utils/NoiseUtilsTest.cpp
        ${HEADER_FILES}
        )

target_link_libraries(${TEST} UtyMap 
                              ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                              ${Boost_FILESYSTEM_LIBRARY})

enable_testing ()
add_test (${TEST} ${TEST})
