#pragma once

#include <cassert>
#include <ostream>
#include <string>
#include <unordered_set>

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

class VReduceConstants : public Visitor
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
    /// 0=0 -> T          1=0 -> F
    ///
    /// <=(0,a) -> T      <=(a,1) -> T
    ///
    /// C(0,0) -> F       C(1,1) -> T
    /// C(a,0) -> F       C(0,a) -> F
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

    ~VReduceConstants() override = default;
};

class VConvertContactsWithConstantTerms : public Visitor
{
public:
    /// NOTE: better use it after the contacts are reduced, via VReduceConstants
    /// C(a,1)-> ~(a=0) C(1,a)-> ~(a=0)
    void visit(NFormula& f) override;
    void visit(NTerm&) override {}

    ~VConvertContactsWithConstantTerms() override = default;
};

class VConvertLessEqContactWithEqualTerms : public Visitor
{
public:
    /// <=(a,a) -> T because (a * -a = 0)
    /// C(a,a) -> ~(a=0)
    void visit(NFormula& f) override;
    void visit(NTerm&) override {}

    ~VConvertLessEqContactWithEqualTerms() override = default;
};

class VReduceDoubleNegation : public Visitor
{
public:
    /// NOTE: better use it after all visitors which might add additional negations!
    /// --g -> g
    void visit(NFormula& f) override;
    /// --t -> t
    void visit(NTerm& t) override;

    ~VReduceDoubleNegation() override = default;
};

class VConvertImplicationEqualityToConjDisj : public Visitor
{
public:
    /// Converts all formula nodes of type implication and equality to nodes which areusing just conjuction and disjunction
    /// E.g. (f -> g)  => (~f | g)
    ///      (f <-> g) => ((f & g) | (~f & ~g))
    void visit(NFormula& f) override;
    void visit(NTerm&) override {}

    ~VConvertImplicationEqualityToConjDisj() override = default;
};

class VConvertLessEqToEqZero : public Visitor
{
public:
    /// Converts the <=(a,b) operations to (a * -b) = 0 operations
    void visit(NFormula& f) override;
    void visit(NTerm&) override {}

    ~VConvertLessEqToEqZero() override = default;
};

class VSplitDisjInLessEqAndContacts : public Visitor
{
public:
    /// Splits C(a + b, c) to C(a,c) | C(b,c)
    /// Splits <=(a + b, c) -> <=(a,c) & <=(b,c)
    void visit(NFormula& f) override;
    void visit(NTerm&) override {}

    ~VSplitDisjInLessEqAndContacts() override = default;
};

class VVariablesGetter : public Visitor
{
public:
    using variables_set_t = std::unordered_set<std::string>;

    VVariablesGetter(variables_set_t& out_variables);

    void visit(NFormula& f) override;
    void visit(NTerm& t) override;

    ~VVariablesGetter() override = default;
private:
    variables_set_t& variables;
};
