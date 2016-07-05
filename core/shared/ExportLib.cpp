#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#elif _GCC
#define EXPORT_API __attribute__((visibility("default")))
#else
#define EXPORT_API
#endif

#include "Application.hpp"

static Application* applicationPtr = nullptr;

extern "C"
{
    // Composes object graph.
    void EXPORT_API configure(const char* stringPath,  // path to string table directory
                              const char* elePath,     // path to elevation directory
                              OnError* errorCallback)  // completion callback.
    {
        applicationPtr = new Application(stringPath, elePath, errorCallback);
    }

    // Performs cleanup.
    void EXPORT_API cleanup()
    {
        delete applicationPtr;
    }

    // Register stylesheet.
    void EXPORT_API registerStylesheet(const char* path)
    {
        applicationPtr->registerStylesheet(path);
    }

    // Preloads elevation data.
    void EXPORT_API preloadElevation(int tileX,        // tile x
                                     int tileY,        // tile y
                                     int levelOfDetail) // level of detail
    {
        applicationPtr->preloadElevation(utymap::QuadKey(levelOfDetail, tileX, tileY));
    }

    // Registers new in-memory store.
    void EXPORT_API registerInMemoryStore(const char* key)
    {
        applicationPtr->registerInMemoryStore(key);
    }

    // Registers new persistent store.
    void EXPORT_API registerPersistentStore(const char* key, const char* dataPath)
    {
        applicationPtr->registerPersistentStore(key, dataPath);
    }

    // Adds data to store to specific level of details range.
    void EXPORT_API addToStoreInRange(const char* key,           // store key
                                      const char* styleFile,     // style file
                                      const char* path,          // path to data
                                      int startLod,              // start zoom level
                                      int endLod,                // end zoom level
                                      OnError* errorCallback)    // completion callback
   {
        applicationPtr->addToStore(key, styleFile, path, utymap::LodRange(startLod, endLod), errorCallback);
   }

    // Adds data to store to specific level of details range.
    void EXPORT_API addToStoreInBoundingBox(const char* key,           // store key
                                            const char* styleFile,     // style file
                                            const char* path,          // path to data
                                            double minLat,             // minimal latitude
                                            double minLon,             // minimal longitude
                                            double maxLat,             // maximal latitude
                                            double maxLon,             // maximal longitude
                                            int startLod,              // start zoom level
                                            int endLod,                // end zoom level
                                            OnError* errorCallback)    // completion callback
    {
        utymap::BoundingBox bbox(utymap::GeoCoordinate(minLat, minLon), utymap::GeoCoordinate(maxLat,maxLon));
        utymap::LodRange lod(startLod, endLod);
        applicationPtr->addToStore(key, styleFile, path, bbox, lod, errorCallback);
    }

    // Adds data to store to specific quadkey only.
    void EXPORT_API addToStoreInQuadKey(const char* key,           // store key
                                        const char* styleFile,     // style file
                                        const char* path,          // path to data
                                        int tileX,                 // tile x
                                        int tileY,                 // tile y
                                        int levelOfDetail,         // level of detail
                                        OnError* errorCallback)    // completion callback
    {
        applicationPtr->addToStore(key, styleFile, path, utymap::QuadKey(levelOfDetail, tileX, tileY), errorCallback);
    }

    // Adds element to store. NOTE: relation is not yet supported.
    void EXPORT_API addToStoreElement(const char* key,           // store key
                                      const char* styleFile,     // style file
                                      std::uint64_t id,          // element id
                                      const double* vertices,    // vertex array
                                      int vertexLength,          // vertex array length,
                                      const char** tags,          // tag array
                                      int tagLength,             // tag array length
                                      int startLod,              // start zoom level
                                      int endLod,                // end zoom level
                                      OnError* errorCallback)    // completion callback
    {
        utymap::LodRange lod(startLod, endLod);
        std::vector<utymap::entities::Tag> elementTags;
        elementTags.reserve(tagLength / 2);
        for (int i = 0; i < tagLength; i+=2) {
            auto keyId = applicationPtr->getStringId(tags[i]);
            auto valueId = applicationPtr->getStringId(tags[i + 1]);
            elementTags.push_back(utymap::entities::Tag(keyId, valueId));
        }

        // Node
        if (vertexLength / 2 == 1) {
            utymap::entities::Node node;
            node.id = id;
            node.tags = elementTags;
            node.coordinate = utymap::GeoCoordinate(vertices[0], vertices[1]);
            applicationPtr->addToStore(key, styleFile, node, lod, errorCallback);
            return;
        }

        std::vector<utymap::GeoCoordinate> coordinates;
        coordinates.reserve(vertexLength / 2);
        for (int i = 0; i < vertexLength; i+=2) {
            coordinates.push_back(utymap::GeoCoordinate(vertices[i], vertices[i + 1]));
        }

        // Way or Area
        if (coordinates[0] == coordinates[coordinates.size() - 1]) {
            utymap::entities::Area area;
            area.id = id;
            area.coordinates = coordinates;
            area.tags = elementTags;
            applicationPtr->addToStore(key, styleFile, area, lod, errorCallback);
        } else {
            utymap::entities::Way way;
            way.id = id;
            way.coordinates = coordinates;
            way.tags = elementTags;
            applicationPtr->addToStore(key, styleFile, way, lod, errorCallback);
        }
    }

    // Loads quadkey.
    void EXPORT_API loadQuadKey(const char* styleFile,                   // style file
                                int tileX, int tileY, int levelOfDetail, // quadkey info
                                OnMeshBuilt* meshCallback,               // mesh callback
                                OnElementLoaded* elementCallback,        // element callback
                                OnError* errorCallback)                  // completion callback
    {
        utymap::QuadKey quadKey(levelOfDetail, tileX, tileY);
        applicationPtr->loadQuadKey(styleFile, quadKey, meshCallback, elementCallback, errorCallback);
    }

    // Checks whether there is data for given quadkey
    bool EXPORT_API hasData(int tileX, int tileY, int levelOfDetail) // quadkey info
    {
        return applicationPtr->hasData(utymap::QuadKey(levelOfDetail, tileX, tileY));
    }
}
