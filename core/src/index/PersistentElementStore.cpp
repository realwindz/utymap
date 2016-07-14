#include "BoundingBox.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/PersistentElementStore.hpp"

#include <fstream>
#include <sstream>

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::utils;

namespace {
    //                                      Index file format
    //   DESCRIPTION    |                       DETAILS                                                     |
    //------------------------------------------------------------------------------------------------------|
    //     Element      |  List of entries, each is represented by element id (8b) and file offset (4b)     |
    //------------------------------------------------------------------------------------------------------|
    const std::string IndexFileExtension = ".idf";

    //                                      Data file format
    //------------------------------------------------------------------------------------------------------|
    //   DESCRIPTION    |                       DETAILS                                                     |
    //------------------------------------------------------------------------------------------------------|
    //  (1b) Flags      |  00 00 00 AA, where:                                                              |
    //                  |    AA - Element type (00 - Node, 01 - Way, 10 - Area, 11 - Relation)              |
    //------------------------------------------------------------------------------------------------------|
    //  (2b) Tags Size  |  Size of tag list where each tag represented by key-value pair (4b + 4b)          |
    //------------------------------------------------------------------------------------------------------|
    //      Tags        |               Tags data with size (2b)                                            |
    //------------------------------------------------------------------------------------------------------|
    //    Geometry      |             Geometry for Node, Way or Area                                        |
    //       or         |                                                                                   |
    //    Element List  |             Element list in the same format + id                                  |
    //                  |                                                                                   |
    //------------------------------------------------------------------------------------------------------|
    const std::string DataFileExtension = ".dat";

    // Writes element to file stream.
    class ElementWriter : public ElementVisitor
    {
    public:
        ElementWriter(std::fstream& dataFile) : dataFile_(dataFile)
        {
        }

        void visitNode(const Node& node)
        {
            writeFlags(0);
            writeTags(node.tags);
            writeCoordinate(node.coordinate);
        }

        void visitWay(const Way& way)
        {
            writeFlags(1);
            writeTags(way.tags);
            std::uint16_t size = static_cast<std::uint16_t>(way.coordinates.size());
            for (const auto& coord : way.coordinates) {
                writeCoordinate(coord);
            }
        }

        void visitArea(const Area& area)
        {
            writeFlags(2);
            writeTags(area.tags);
            // NOTE do not write the last one
            auto coordSize = area.coordinates.size() - 1;
            std::uint16_t size = static_cast<std::uint16_t>(coordSize);
            for (std::size_t i = 0; i < coordSize; ++i) {
                writeCoordinate(area.coordinates[i]);
            }
        }

        void visitRelation(const Relation& relation)
        {
            writeFlags(3);
            writeTags(relation.tags);
            for (const auto& element : relation.elements) {
                dataFile_.write(reinterpret_cast<const char*>(&element->id), sizeof(element->id));
                element->accept(*this);
            }
        }

    private:

        void writeFlags(const std::uint8_t flags)
        {
            dataFile_.write(reinterpret_cast<const char*>(&flags), sizeof(flags));
        }

        void writeTags(const std::vector<Tag>& tags)
        {
            std::uint16_t size = static_cast<std::uint16_t>(tags.size());
            dataFile_.write(reinterpret_cast<const char*>(&size), sizeof(size));
            for (const auto& tag : tags) {
                dataFile_.write(reinterpret_cast<const char*>(&tag.key), sizeof(tag.key));
                dataFile_.write(reinterpret_cast<const char*>(&tag.value), sizeof(tag.value));
            }
        }

        void writeCoordinate(const GeoCoordinate& coord)
        {
            dataFile_.write(reinterpret_cast<const char*>(&coord.latitude), sizeof(coord.latitude));
            dataFile_.write(reinterpret_cast<const char*>(&coord.longitude), sizeof(coord.longitude));
        }

        std::fstream& dataFile_;
    };

    // Reads element from file stream.
    class ElementReader
    {
    public:
        ElementReader(std::fstream& dataFile) : dataFile_(dataFile)
        {
        }

        std::shared_ptr<Element> readElement(std::uint64_t id, std::uint32_t offset)
        {
            dataFile_.seekg(offset, std::ios::beg);
            auto element = readElement();
            element->id = id;
            return element;
        }

    private:

        std::shared_ptr<Element> readElement()
        {
            std::uint8_t flags;
            dataFile_.read(reinterpret_cast<char*>(&flags), sizeof(flags));
            std::uint8_t elementType = flags & 0x3;

            switch (elementType) {
            case 0:
                return readNode();
            case 1:
                return readWay();
            case 2:
                return readArea();
            default:
                return readRelation();
            }
        }

        std::shared_ptr<Node> readNode()
        {
            auto node = std::make_shared<Node>();
            node->tags = readTags();
            node->coordinate = readCoordinate();
            return node;
        }

        std::shared_ptr<Way> readWay()
        {
            auto way = std::make_shared<Way>();
            way->tags = readTags();
            way->coordinates = readCoordinates(false);
            return way;
        }

        std::shared_ptr<Area> readArea()
        {
            auto area = std::make_shared<Area>();
            area->tags = readTags();
            area->coordinates = readCoordinates(true);
            return area;
        }

        std::shared_ptr<Relation> readRelation()
        {
            auto relation = std::make_shared<Relation>();
            relation->tags = readTags();
            std::uint16_t elementSize;
            dataFile_.read(reinterpret_cast<char*>(&elementSize), sizeof(elementSize));

            for (std::uint16_t i = 0; i < elementSize; ++i) {
                std::uint8_t elementType;
                relation->elements.push_back(readElement());
            }
            return relation;
        }

        inline GeoCoordinate readCoordinate()
        {
            GeoCoordinate coord;
            dataFile_.read(reinterpret_cast<char*>(&coord.latitude), sizeof(coord.latitude));
            dataFile_.read(reinterpret_cast<char*>(&coord.longitude), sizeof(coord.longitude));
            return coord;
        }

        inline std::vector<GeoCoordinate> readCoordinates(bool hasExtraElement)
        {
            std::uint16_t coordSize;
            dataFile_.read(reinterpret_cast<char*>(&coordSize), sizeof(coordSize));
            if (hasExtraElement) 
                ++coordSize;

            std::vector<GeoCoordinate> coordinates;
            coordinates.reserve(coordSize);
            for (std::size_t i = 0; i < coordSize; ++i) {
                coordinates.push_back(readCoordinate());
            }

            return std::move(coordinates);
        }

        inline std::vector<Tag> readTags()
        {
            std::uint16_t tagSize;
            dataFile_.read(reinterpret_cast<char*>(&tagSize), sizeof(tagSize));

            std::vector<Tag> tags;
            for (std::size_t i = 0; i < tagSize; ++i) {
                Tag tag;
                dataFile_.read(reinterpret_cast<char*>(&tag.key), sizeof(tag.key));
                dataFile_.read(reinterpret_cast<char*>(&tag.value), sizeof(tag.value));
                tags.push_back(tag);
            }

            return std::move(tags);
        }

        std::fstream& dataFile_;
    };
}

class PersistentElementStore::PersistentElementStoreImpl
{
public:
    PersistentElementStoreImpl(const std::string& dataPath)
            : dataPath_(dataPath)
    {
    }

    void store(const Element& element, const QuadKey& quadKey)
    {
        ensureFiles(quadKey);

        // write element data
        std::uint32_t offset = static_cast<std::uint32_t>(dataFile_.tellg());

        ElementWriter visitor(dataFile_);
        element.accept(visitor);

        // write element index
        indexFile_.seekg(0, std::ios::end);
        indexFile_.write(reinterpret_cast<const char*>(&element.id), sizeof(element.id));
        indexFile_.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
    }

    void search(const QuadKey& quadKey, ElementVisitor& visitor)
    {
        ensureFiles(quadKey);

        std::uint32_t count = static_cast<std::uint32_t>(indexFile_.tellg() /
                (sizeof(std::uint64_t) + sizeof(std::uint32_t)));

        ElementReader reader(dataFile_);

        indexFile_.seekg(0, std::ios::beg);
        for (std::uint32_t i = 0; i < count; ++i) {
            std::uint64_t id;
            std::uint32_t offset;
            indexFile_.read(reinterpret_cast<char*>(&id), sizeof(id));
            indexFile_.read(reinterpret_cast<char*>(&offset), sizeof(offset));

            reader.readElement(id, offset)->accept(visitor);
        }
    }

    bool hasData(const QuadKey& quadKey) const
    {
        std::ifstream file(getFilePath(quadKey, DataFileExtension));
        return file.good();
    }

    void commit()
    {
        closeFiles();
        currentQuadKey_ = QuadKey();
    }

private:
    // gets full file path for given quadkey
    inline std::string getFilePath(const QuadKey& quadKey, const std::string& extension) const
    {
        std::stringstream ss;
        ss << dataPath_ << quadKey.levelOfDetail << "/" << GeoUtils::quadKeyToString(quadKey) << extension;
        return ss.str();
    }

    // Ensures that open/close function is called on files.
    inline void ensureFiles(const QuadKey& quadKey)
    {
        if (quadKey == currentQuadKey_)
            return;

        closeFiles();
        
        using std::ios;
        dataFile_.open(getFilePath(quadKey, DataFileExtension), ios::in | ios::out | ios::binary | ios::app | ios::ate);
        indexFile_.open(getFilePath(quadKey, IndexFileExtension),ios::in | ios::out | ios::binary | ios::app | ios::ate);

        currentQuadKey_ = quadKey;
    }

    inline void closeFiles()
    {
        if (dataFile_.good()) dataFile_.close();
        if (indexFile_.good()) indexFile_.close();
    }

    const std::string dataPath_;
    
    QuadKey currentQuadKey_;

    std::fstream indexFile_;
    std::fstream dataFile_;
};

PersistentElementStore::PersistentElementStore(const std::string& dataPath, StringTable& stringTable) :
        ElementStore(stringTable), pimpl_(new PersistentElementStore::PersistentElementStoreImpl(dataPath))
{
}

PersistentElementStore::~PersistentElementStore()
{
}

void PersistentElementStore::storeImpl(const Element& element, const QuadKey& quadKey)
{
    pimpl_->store(element, quadKey);
}

void PersistentElementStore::search(const QuadKey& quadKey, ElementVisitor& visitor)
{
    pimpl_->search(quadKey, visitor);
}

bool PersistentElementStore::hasData(const QuadKey& quadKey) const
{
    return pimpl_->hasData(quadKey);
}

void PersistentElementStore::commit()
{
    pimpl_->commit();
}
