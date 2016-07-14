#ifndef QUADKEY_HPP_DEFINED
#define QUADKEY_HPP_DEFINED

namespace utymap {

// Represents quadkey: a node of quadtree
struct QuadKey
{
    // Level of details (zoom).
    int levelOfDetail;
    // Tile x at given LOD
    int tileX;
    // Tile y at given LOD
    int tileY;

    QuadKey() : QuadKey(0, 0, 0)
    {
    }

    QuadKey(int levelOfDetail, int tileX, int tileY) :
        levelOfDetail(levelOfDetail), tileX(tileX), tileY(tileY)
    {
    }

    bool operator==(const QuadKey& other) const
    {
        return levelOfDetail == other.levelOfDetail &&
               tileX == other.tileX && tileY == other.tileY;
    }
};

}
#endif // QUADKEY_HPP_DEFINED
