#ifndef MESHING_MESHTYPES_HPP_DEFINED
#define MESHING_MESHTYPES_HPP_DEFINED

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace utymap { namespace meshing {

// Represents vector2 which can be used as point or direction in 2d space
struct Vector2
{
    double x;
    double y;
    Vector2() : x(0), y(0)  { }
    Vector2(double x, double y) : x(x), y(y) { }

    bool operator==(const Vector2& rhs) const
    {
        return std::fabs(x - rhs.x) < std::numeric_limits<double>::epsilon() &&
               std::fabs(y - rhs.y) < std::numeric_limits<double>::epsilon();
    }

    bool operator!=(const Vector2& rhs) const
    {
        return !(*this == rhs);
    }

    static double distance(const Vector2& v1, const Vector2& v2)
    {
        double dx = v1.x - v2.x;
        double dy = v1.y - v2.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

// Represents vector3 which can be used as point or direction in 3d space
struct Vector3
{
    double x;
    double y;
    double z;
    Vector3() : x(0), y(0), z(0)  { }
    Vector3(double x, double y, double z) : x(x), y(y), z(z) { }

    inline Vector3 operator*(double mult)
    {
        return Vector3(x * mult, y *mult, z * mult);
    }

    inline Vector3 operator+(const Vector3& rhs)
    {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    inline double magnitude()
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    inline Vector3 normalized()
    {
        double length = magnitude();
        return length > std::numeric_limits<double>::epsilon()
            ? Vector3(x / length, y / length, z / length)
            : Vector3(); // degenerative case
    }
};

/// Geometry line in linear form. General form:
/// Ax + By + C = 0;
/// <see href="http://en.wikipedia.org/wiki/Linear_equation"/>
struct LineLinear
{
    double A;
    double B;
    double C;

    /// Linear line from two points on line.
    LineLinear(const Vector2& p1, const Vector2& p2)
    {
        A = p1.y - p2.y;
        B = p2.x - p1.x;
        C = p1.x * p2.y - p2.x * p1.y;
    }

    // Linear line represented by coefficients.
    LineLinear(double pA, double pB, double pC)
    {
        A = pA;
        B = pB;
        C = pC;
    }

    /// Gets collision point of two lines.
    inline Vector2 collide(const LineLinear& line)
    {
        return collide(*this, line);
    }

    /// <summary> Collision point of two lines. </summary>
    inline static Vector2 collide(const LineLinear& line1, const LineLinear& line2)
    {
        return collide(line1.A, line1.B, line1.C, line2.A, line2.B, line2.C);
    }

    /// <summary> Collision point of two lines. </summary>
    inline static Vector2 collide(double A1, double B1, double C1, double A2, double B2, double C2)
    {
        double WAB = A1*B2 - A2*B1;
        double WBC = B1*C2 - B2*C1;
        double WCA = C1*A2 - C2*A1;

        return WAB == 0 
            ? Vector2() 
            : Vector2(WBC / WAB, WCA / WAB);
    }

    /// <summary> Check whether point belongs to line. </summary>
    inline bool contains(const Vector2& point) const
    {
        return std::abs((point.x * A + point.y * B + C)) < 1E-8;
    }
};

// Represents axis aligned rectangle in 2D space.
struct Rectangle
{
    double xMin, xMax, yMin, yMax;

    LineLinear _left, _right, _bottom, _top;

    Rectangle() 
        : Rectangle(std::numeric_limits<double>::max(), 
                    std::numeric_limits<double>::max(), 
                    std::numeric_limits<double>::min(), 
                    std::numeric_limits<double>::min())
    {
    }

    Rectangle(double xMin, double yMin, double xMax, double yMax) :
        xMin(xMin), yMin(yMin), xMax(xMax), yMax(yMax),
        _left(Vector2(xMin, yMin), Vector2(xMin, yMax)),
        _right(Vector2(xMax, yMin), Vector2(xMax, yMax)),
        _bottom(Vector2(xMin, yMin), Vector2(xMax, yMin)),
        _top(Vector2(xMin, yMax), Vector2(xMax, yMax))
    {
    }

    void expand(const std::vector<Vector2>& contour)
    {
        for (const Vector2& p : contour) {
            expand(p);
        }
    }

    inline void expand(const Vector2& point)
    {
        xMin = std::min(xMin, point.x);
        yMin = std::min(yMin, point.y);
        xMax = std::max(xMax, point.x);
        yMax = std::max(yMax, point.y);
    }

    inline bool contains(const Vector2& pt) const
    {
        return ((pt.x >= xMin) && (pt.x <= xMax) && (pt.y >= yMin) && (pt.y <= yMax));
    }

    // Checks whether point is on border of rectangle.
    inline bool isOnBorder(const Vector2& point) const
    {
        return _left.contains(point) || _right.contains(point) ||
               _bottom.contains(point) || _top.contains(point);
    }
};

// Represents mesh which uses only primitive types to store data due to interoperability.
struct Mesh
{
    std::string name;
    std::vector<double> vertices;
    std::vector<int> triangles;
    std::vector<int> colors;

    Mesh(const std::string& name) : name(name) { }

    // disable copying to prevent accidential copy
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
};

using Contour = std::vector<utymap::meshing::Vector2>;

}}

#endif // MESHING_MESHTYPES_HPP_DEFINED
