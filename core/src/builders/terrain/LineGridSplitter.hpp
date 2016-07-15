#ifndef BUILDERS_TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
#define BUILDERS_TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "meshing/MeshTypes.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace utymap { namespace builders {

// Splits line to segments on axis aligned grid intersections.
class LineGridSplitter
{
    typedef utymap::meshing::Vector2 Point;
    typedef std::vector<Point> Points;

    struct sort_x
    {
        inline bool operator() (const Point& a, const Point& b) { return a.x < b.x; }
    };

    struct sort_reverse_x
    {
        inline bool operator() (const Point& a, const Point& b) { return a.x > b.x; }
    };

    struct sort_y
    {
        inline bool operator() (const Point& a, const Point& b) { return a.y < b.y; }
    };

    struct sort_reverse_y
    {
        inline bool operator() (const Point& a, const Point& b) { return a.y > b.y; }
    };

public:
    LineGridSplitter() : step_(1), scale_(1)
    {
    }

    void setParams(double scale, double step)
    {
        scale_ = scale;
        step_ = step;
    }

    // Splits line to segments.
    void split(const ClipperLib::IntPoint& start, const ClipperLib::IntPoint& end, Points& result) const
    {
        Point s(start.X / scale_, start.Y / scale_);
        Point e(end.X / scale_, end.Y / scale_);

        Points points;
        points.reserve(2);
        points.push_back(s);

        double slope = (e.y - s.y) / (e.x - s.x);
        if (std::isinf(slope) || std::abs(slope) < std::numeric_limits<double>::epsilon())
            zeroSlope(s, e, points);
        else
            normalCase(s, e, slope, points);

        points.push_back(e);

        mergeResults(points, result);
    }

private:

    inline double ceil(double value) const
    {
        return std::ceil(value / step_) * step_;
    }

    void zeroSlope(Point start, Point end, Points& points) const
    {
        if ((start.x - end.x) == 0)
        {
            bool isBottomTop = start.y < end.y;
            if (!isBottomTop)
            {
                Point tmp = start;
                start = end;
                end = tmp;
            }

            double yStart = ceil(start.y);
            double yEnd = end.y;
            for (double y = yStart; y < yEnd; y += step_)
                points.push_back(Point(start.x, y));

            if (isBottomTop)
                std::sort(points.begin(), points.end(), sort_y());
            else
                std::sort(points.begin(), points.end(), sort_reverse_y());
        }
        else
        {
            bool isLeftRight = start.x < end.x;
            if (!isLeftRight)
            {
                Point tmp = start;
                start = end;
                end = tmp;
            }

            double xStart = ceil(start.x);
            double xEnd = end.x;
            for (double x = xStart; x < xEnd; x += step_)
                points.push_back(Point(x, start.y));

            if (isLeftRight)
                std::sort(points.begin(), points.end(), sort_x());
            else
                std::sort(points.begin(), points.end(), sort_reverse_x());
        }
    }

    void normalCase(Point start, Point end, double slope, Points& points) const
    {
        double inverseSlope = 1 / slope;
        double b = start.y - slope * start.x;

        bool isLeftRight = start.x < end.x;
        if (!isLeftRight)
        {
            Point tmp = start;
            start = end;
            end = tmp;
        }

        double xStart = ceil(start.x);
        double xEnd = end.x;
        for (double x = xStart; x < xEnd; x += step_)
            points.push_back(Point(x, slope * x + b));

        bool isBottomTop = start.y < end.y;
        if (!isBottomTop)
        {
            Point tmp = start;
            start = end;
            end = tmp;
        }

        double yStart = ceil(start.y);
        double yEnd = end.y;
        for (double y = yStart; y < yEnd; y += step_)
            points.push_back(Point((y - b) * inverseSlope, y));

        if (isLeftRight)
            std::sort(points.begin(), points.end(), sort_x());
        else
            std::sort(points.begin(), points.end(), sort_reverse_x());
    }

    void mergeResults(const Points& points, Points& result) const
    {
        for (std::size_t i = 0; i < points.size(); ++i) {
            Point candidate = points[i];
            if (result.size() > 0)  {
                Point last = result[result.size() - 1];
                if (std::abs(last.x - candidate.x) < std::numeric_limits<double>::epsilon() &&
                    std::abs(last.y - candidate.y) < std::numeric_limits<double>::epsilon())
                    continue;
            }

            result.push_back(candidate);
        }
    }

    double scale_;
    double step_;
};

}
}

#endif // BUILDERS_TERRAIN_LINEGRIDSPLITTER_HPP_DEFINED
