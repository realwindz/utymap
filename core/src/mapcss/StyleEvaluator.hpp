#ifndef MAPCSS_STYLEEVALUATOR_HPP_INCLUDED
#define MAPCSS_STYLEEVALUATOR_HPP_INCLUDED

#include "entities/Element.hpp"
#include "index/StringTable.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/ElementUtils.hpp"

#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>

#include <cstdint>
#include <string>
#include <list>
#include <memory>
#include <type_traits>
#include <vector>

namespace utymap { namespace mapcss {

// Represents style declaration which support evaluation.
struct StyleEvaluator
{
    // NOTE has to put these declarations here due to evaluate function implementation
    struct Nil {};
    struct Signed;
    struct Tree;
    typedef boost::variant<Nil, double, std::string, boost::recursive_wrapper<Signed>, boost::recursive_wrapper<Tree>> Operand;

    struct Signed
    {
        char sign;
        Operand operand;
    };

    struct Operation
    {
        char operator_;
        Operand operand;
    };

    struct Tree
    {
        Operand first;
        std::list<Operation> rest;
    };

    // Parses expression into AST.
    static std::shared_ptr<Tree> parse(const std::string& expression);

    // Evaluates expression using tags.
    template <typename T>
    static T evaluate(const Tree& tree,
                      const std::vector<utymap::entities::Tag>& tags,
                      utymap::index::StringTable& stringTable)
    {
        typedef typename std::conditional<std::is_same<T, std::string>::value, StringEvaluator, DoubleEvaluator>::type EvaluatorType;
        return EvaluatorType(tags, stringTable)(tree);
    }

private:

    // Specifies default AST evaluator behaviour.
    template <typename T>
    struct Evaluator
    {
        typedef T result_type;

        Evaluator(const std::vector<utymap::entities::Tag>& tags,
            utymap::index::StringTable& stringTable)
            : tags_(tags), stringTable_(stringTable)  { }

    protected:
        std::string throwException() const { throw std::domain_error("Evaluator: unsupported operation."); }

        const std::vector<utymap::entities::Tag>& tags_;
        utymap::index::StringTable& stringTable_;
    };

    // Evaluates double from AST.
    struct DoubleEvaluator : public Evaluator<double>
    {
        DoubleEvaluator(const std::vector<utymap::entities::Tag>& tags,
            utymap::index::StringTable& stringTable)
            : Evaluator(tags, stringTable) { }

        double operator()(Nil) const { return 0; }
        double operator()(double n) const { return n; }

        double operator()(const Operation& o, double lhs) const
        {
            double rhs = boost::apply_visitor(*this, o.operand);
            switch (o.operator_) {
            case '+': return lhs + rhs;
            case '-': return lhs - rhs;
            case '*': return lhs * rhs;
            case '/': return lhs / rhs;
            }
            return 0;
        }

        double operator()(const std::string& tagKey) const
        {
            auto keyId = stringTable_.getId(tagKey);
            return utymap::utils::parseDouble(utymap::utils::getTagValue(keyId, tags_, stringTable_));
        }

        double operator()(const Signed& s) const
        {
            double rhs = boost::apply_visitor(*this, s.operand);
            switch (s.sign) {
            case '-': return -rhs;
            case '+': return +rhs;
            }
            return 0;
        }

        double operator()(const Tree& tree) const
        {
            double state = boost::apply_visitor(*this, tree.first);
            for (const Operation& oper : tree.rest)
                state = (*this)(oper, state);
            return state;
        }
    };

    // Evaluates string from AST.
    struct StringEvaluator : public Evaluator<std::string>
    {
        StringEvaluator(const std::vector<utymap::entities::Tag>& tags,
            utymap::index::StringTable& stringTable)
            : Evaluator(tags, stringTable) { }

        std::string operator()(Nil) const { return throwException(); }
        std::string operator()(double n) const { return throwException(); }

        std::string operator()(const Operation& o, double lhs) const { return throwException(); }

        std::string operator()(const std::string& tagKey) const
        {
            return utymap::utils::getTagValue(stringTable_.getId(tagKey), tags_, stringTable_);
        }

        std::string operator()(const Signed& s) const { return throwException(); }

        std::string operator()(const Tree& tree) const
        {
            return boost::apply_visitor(*this, tree.first);
        }
    };

    std::shared_ptr<std::string> value_;
    std::shared_ptr<Tree> tree_;
};

} }
#endif  // MAPCSS_STYLEEVALUATOR_HPP_INCLUDED
