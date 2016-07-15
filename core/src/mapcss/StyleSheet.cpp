#include "mapcss/StyleSheet.hpp"

#include <numeric>
#include <ostream>

using namespace utymap::mapcss;

std::ostream& utymap::mapcss::operator<<(std::ostream &stream, const Condition &c)
{
    return stream << c.key << c.operation << c.value;
}

std::ostream& utymap::mapcss::operator<<(std::ostream &stream, const Zoom &z)
{
    return stream << "|z" << std::to_string((int)z.start) << "-" << std::to_string((int)z.end);
}

std::ostream& utymap::mapcss::operator<<(std::ostream &stream, const Selector &s)
{
    // don't care about performance here
    stream << std::accumulate(s.names.cbegin(), s.names.cend(), std::string(),
        [](std::string& acc, const std::string &s)
    {
        return acc.empty() ? s : acc + "," + s;
    }) << s.zoom;
        
    for (const Condition& condition : s.conditions) {
        stream << "[" << condition << "]";
    }

    return stream;
}

std::ostream& utymap::mapcss::operator<<(std::ostream &stream, const Declaration &d)
{
    return stream << d.key << ":" << d.value;
}

std::ostream& utymap::mapcss::operator<<(std::ostream &stream, const Rule &r)
{
    for (auto i = 0; i < r.selectors.size(); ++i) {
        stream << r.selectors[i];
        if (i != r.selectors.size() - 1) stream << ",";
    }
    stream << "{";
    for (const Declaration& declaration : r.declarations) {
        stream << declaration << ";";
    }
    return stream << "}";
}

std::ostream& utymap::mapcss::operator<<(std::ostream &stream, const StyleSheet &s)
{
    for (const Rule& rule : s.rules) {
        stream << rule << std::endl;
    }
    return stream;
}
