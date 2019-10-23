#pragma once

#include <cassert>
#include <ostream>

class NFormula;
class NTerm;

class Visitor
{
public:
    virtual void visit(NFormula& f) = 0;
    virtual void visit(NTerm& t) = 0;
    virtual ~Visitor() = default;
};

class VPrinter : public Visitor
{
public:
    VPrinter(std::ostream& out);

    void visit(NFormula& f) override;
    void visit(NTerm& t) override;

    ~VPrinter() override = default;
private:
    std::ostream& out_;
};

class VReduceTrivialAndOrNegOperations : public Visitor
{
public:
    /// Removes all unnecessary childs of And/Or/Negation operations of the following type:
    /// (formulas):
    ///      -T -> F           -F -> T
    /// (T & T) -> T      (F | F) -> F
    ///
    /// (g & T) -> g      (g | T) -> T
    /// (T & g) -> g      (T | g) -> T
    /// (g & F) -> F      (g | F) -> g
    /// (F & g) -> F      (F | g) -> g
    ///
    /// (terms):
    ///      -1 -> 0            -0 -> 1
    /// (1 * 1) -> 1       (0 + 0) -> 0
    ///
    /// (t * 1) -> t       (t + 1) -> 1
    /// (1 * t) -> t       (1 + t) -> 1
    /// (t * 0) -> 0       (t + 0) -> t
    /// (0 * t) -> 0       (0 + t) -> t
    void visit(NFormula& f) override;
    void visit(NTerm& t) override;

    ~VReduceTrivialAndOrNegOperations() override = default;
};

class VConvertImplicationEqualityToConjDisj : public Visitor
{
public:
    /// Converts all formula nodes of type implication and equality to nodes which areusing just conjuction and disjunction
    /// E.g. (f -> g)  => (~f | g)
    ///      (f <-> g) => ((f & g) | (~f & ~g))
    void visit(NFormula& f) override;
    void visit(NTerm&) override;

    ~VConvertImplicationEqualityToConjDisj() override = default;
};

class VConvertLessEqToEqZero : public Visitor
{
public:
    /// Converts the <=(a,b) operations to (a * -b) = 0 operations
    void visit(NFormula& f) override;
    void visit(NTerm&) override;

    ~VConvertLessEqToEqZero() override = default;
};
