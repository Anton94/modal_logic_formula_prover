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

    ~VPrinter() = default;
private:
    std::ostream& out_;
};
