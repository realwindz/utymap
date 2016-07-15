#define REAL double
#define ANSI_DECLARATORS

#include "meshing/MeshBuilder.hpp"
#include "triangle/triangle.h"
#include "utils/GradientUtils.hpp"

using namespace utymap::heightmap;
using namespace utymap::meshing;
using namespace utymap::utils;

class MeshBuilder::MeshBuilderImpl
{
public:

    MeshBuilderImpl(const ElevationProvider& eleProvider)
    : eleProvider_(eleProvider) { }
     
    void addPolygon(Mesh& mesh, Polygon& polygon, const MeshBuilder::Options& options) const
    {
        triangulateio in, mid;

        in.numberofpoints = static_cast<int>(polygon.points.size() / 2);
        in.numberofholes = static_cast<int>(polygon.holes.size() / 2);
        in.numberofpointattributes = 0;
        in.numberofregions = 0;
        in.numberofsegments = static_cast<int>(polygon.segments.size() / 2);

        in.pointlist = polygon.points.data();
        in.holelist = polygon.holes.data();
        in.segmentlist = polygon.segments.data();
        in.segmentmarkerlist = nullptr;
        in.pointmarkerlist = nullptr;

        mid.pointlist = nullptr;
        mid.pointmarkerlist = nullptr;
        mid.trianglelist = nullptr;
        mid.segmentlist = nullptr;
        mid.segmentmarkerlist = nullptr;

        ::triangulate(const_cast<char*>("pzBQ"), &in, &mid, nullptr);

        // do not refine mesh if area is not set.
        if (std::abs(options.area) < std::numeric_limits<double>::epsilon()) {
            fillMesh(&mid, mesh, options);
            mid.trianglearealist = nullptr;
        }
        else {

            mid.trianglearealist = (REAL *)malloc(mid.numberoftriangles * sizeof(REAL));
            for (int i = 0; i < mid.numberoftriangles; ++i) {
                mid.trianglearealist[i] = options.area;
            }

            triangulateio out;
            out.pointlist = nullptr;
            out.pointattributelist = nullptr;
            out.trianglelist = nullptr;
            out.triangleattributelist = nullptr;
            out.pointmarkerlist = nullptr;

            std::string triOptions = "prazPQ";
            for (int i = 0; i < options.segmentSplit; i++) {
                triOptions += "Y";
            }
            ::triangulate(const_cast<char*>(triOptions.c_str()), &mid, &out, nullptr);

            fillMesh(&out, mesh, options);

            free(out.pointlist);
            free(out.pointattributelist);
            free(out.trianglelist);
            free(out.triangleattributelist);
            free(out.pointmarkerlist);
        }

        free(in.pointmarkerlist);

        free(mid.pointlist);
        free(mid.pointmarkerlist);
        free(mid.trianglelist);
        free(mid.trianglearealist);
        free(mid.segmentlist);
        free(mid.segmentmarkerlist);
    }


    inline void addPlane(Mesh& mesh, const Vector2& p1, const Vector2& p2, const MeshBuilder::Options& options) const
    {
        double ele1 = eleProvider_.getElevation(p1.y, p1.x);
        double ele2 = eleProvider_.getElevation(p2.y, p2.x);

        ele1 += NoiseUtils::perlin2D(p1.x, p1.y, options.eleNoiseFreq);
        ele2 += NoiseUtils::perlin2D(p2.x, p2.y, options.eleNoiseFreq);

        addPlane(mesh, p1, p2, ele1, ele2, options);
    }

    inline void addPlane(Mesh& mesh, const Vector2& p1, const Vector2& p2, double ele1, double ele2, const MeshBuilder::Options& options) const
    {
        auto color = options.gradient->evaluate((NoiseUtils::perlin2D(p1.x, p1.y, options.colorNoiseFreq) + 1) / 2);
        int index = static_cast<int>(mesh.vertices.size() / 3);

        addVertex(mesh, p1, ele1, color, index);
        addVertex(mesh, p2, ele2, color, index + 2);
        addVertex(mesh, p2, ele2 + options.heightOffset, color, index + 1);
        index += 3;

        addVertex(mesh, p1, ele1 + options.heightOffset, color, index);
        addVertex(mesh, p1, ele1, color, index + 2);
        addVertex(mesh, p2, ele2 + options.heightOffset, color, index + 1);
    }

    inline void addTriangle(Mesh& mesh, const Vector3& v0, const Vector3& v1, const Vector3& v2, const MeshBuilder::Options& options, bool hasBackSide) const
    {
        auto color = options.gradient->evaluate((NoiseUtils::perlin2D(v0.x, v0.z, options.colorNoiseFreq) + 1) / 2);
        int startIndex = static_cast<int>(mesh.vertices.size() / 3);

        addVertex(mesh, v0, color, startIndex);
        addVertex(mesh, v1, color, ++startIndex);
        addVertex(mesh, v2, color, ++startIndex);

        if (hasBackSide) {
            // TODO check indices
            addVertex(mesh, v2, color, startIndex);
            addVertex(mesh, v1, color, ++startIndex);
            addVertex(mesh, v0, color, ++startIndex);
        }
    }

private:

    inline void addVertex(Mesh& mesh, const Vector2& p, double ele, int color, int triIndex) const
    {
        mesh.vertices.push_back(p.x);
        mesh.vertices.push_back(p.y);
        mesh.vertices.push_back(ele);
        mesh.colors.push_back(color);
        mesh.triangles.push_back(triIndex);
    }

    inline void addVertex(Mesh& mesh, const Vector3& vertex, int color, int triIndex) const
    {
        addVertex(mesh, Vector2(vertex.x, vertex.z), vertex.y, color, triIndex);
    }

    void fillMesh(triangulateio* io, Mesh& mesh, const MeshBuilder::Options& options) const
    {
        int triStartIndex = static_cast<int>(mesh.vertices.size() / 3);

        mesh.vertices.reserve(static_cast<std::size_t>(io->numberofpoints * 3 / 2));
        mesh.triangles.reserve(static_cast<std::size_t>(io->numberoftriangles * 3));
        mesh.colors.reserve(static_cast<std::size_t>(io->numberofpoints));

        for (int i = 0; i < io->numberofpoints; i++) {
            double x = io->pointlist[i * 2 + 0];
            double y = io->pointlist[i * 2 + 1];

            double ele = options.heightOffset +
                (options.elevation > std::numeric_limits<double>::lowest()
                ? options.elevation
                : eleProvider_.getElevation(y, x));

            // do no apply noise on boundaries
            if (io->pointmarkerlist != nullptr && io->pointmarkerlist[i] != 1)
                ele += NoiseUtils::perlin2D(x, y, options.eleNoiseFreq);

            mesh.vertices.push_back(x);
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(ele);

            int color = GradientUtils::getColor(*options.gradient, x, y, options.colorNoiseFreq);
            mesh.colors.push_back(color);
        }

        for (std::size_t i = 0; i < io->numberoftriangles; i++) {
            mesh.triangles.push_back(triStartIndex + io->trianglelist[i * io->numberofcorners + 1]);
            mesh.triangles.push_back(triStartIndex + io->trianglelist[i * io->numberofcorners + 0]);
            mesh.triangles.push_back(triStartIndex + io->trianglelist[i * io->numberofcorners + 2]);
          }
    }

    const ElevationProvider& eleProvider_;
};

MeshBuilder::MeshBuilder(const ElevationProvider& eleProvider) :
    pimpl_(new MeshBuilder::MeshBuilderImpl(eleProvider))
{
}

MeshBuilder::~MeshBuilder() { }

void MeshBuilder::addPolygon(Mesh& mesh, Polygon& polygon, const MeshBuilder::Options& options) const
{
    pimpl_->addPolygon(mesh, polygon, options);
}

void MeshBuilder::addPlane(Mesh& mesh, const Vector2& p1, const Vector2& p2, const MeshBuilder::Options& options) const
{
    pimpl_->addPlane(mesh, p1, p2, options);
}

void MeshBuilder::addPlane(Mesh& mesh, const Vector2& p1, const Vector2& p2, double ele1, double ele2, const MeshBuilder::Options& options) const
{
    pimpl_->addPlane(mesh, p1, p2, ele1, ele2, options);
}

void MeshBuilder::addTriangle(Mesh& mesh, const Vector3& v0, const Vector3& v1, const Vector3& v2, const MeshBuilder::Options& options, bool hasBackSide) const
{
    pimpl_->addTriangle(mesh, v0, v1, v2, options, hasBackSide);
}
