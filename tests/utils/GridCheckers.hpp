#pragma once
#include <set>
#include <unordered_set>

#include "snaps/Grid.hpp"

namespace snaps
{
class Grid;
}

struct CheckResult {
    bool Success = true;
    std::string Summary;
    std::set<std::pair<int, int>> Positions;

    static CheckResult Ok(const std::string& summary) {
        return CheckResult{true, summary};
    }
    static CheckResult OkAt(const int x, const int y, const std::string& summary) {
        return CheckResult{true, summary, {{x, y}}};
    }

    static CheckResult Fail(const std::string& error) {
        return CheckResult{false, error};
    }

    static CheckResult FailAt(const int x, const int y, const std::string& error) {
        return CheckResult{false, error, {{x, y}}};
    }
};

class GridChecker
{
public:
    virtual ~GridChecker() = default;
    virtual CheckResult Check(const snaps::Grid& grid) const = 0;
};

struct EmptyBlockAt : GridChecker
{
    EmptyBlockAt(const int gridX, const int gridY) : m_GridX(gridX), m_GridY(gridY) {}
    CheckResult Check(const snaps::Grid& grid) const override
    {
        const auto& block = grid[m_GridX, m_GridY];
        const std::string summary = std::format("Expected empty block at ({}, {})", m_GridX, m_GridY);
        if (not block) {
            return CheckResult::OkAt(m_GridX, m_GridY, summary);
        }
        return CheckResult::FailAt(m_GridX, m_GridY, summary);
    }

private:
    const int m_GridX;
    const int m_GridY;
};


struct SingleGridChecker : GridChecker {
    SingleGridChecker(const int gridX, const int gridY) : m_GridX(gridX), m_GridY(gridY) {}
    CheckResult Check(const snaps::Grid& grid) const override {
        const auto& block = grid[m_GridX, m_GridY];
        const std::string summary = std::format("Expected {} at ({}, {})", Expectation(), m_GridX, m_GridY);
        if (block and Check(block.value())) {
            return CheckResult::OkAt(m_GridX, m_GridY, summary);
        }
        return CheckResult::FailAt(m_GridX, m_GridY, summary);
    }

    virtual bool Check(const snaps::Block& block) const = 0;
    virtual std::string Expectation() const = 0;

protected:
    const int m_GridX;
    const int m_GridY;
};

struct DynamicBlockAt : SingleGridChecker
{
    DynamicBlockAt(const int gridX, const int gridY) : SingleGridChecker(gridX, gridY) {}
    bool Check(const snaps::Block& block) const override
    {
        return block.IsDynamic;
    }
    std::string Expectation() const override { return "dynamic block"; }
};

struct StaticBlockAt : SingleGridChecker
{
    StaticBlockAt(const int gridX, const int gridY) : SingleGridChecker(gridX, gridY) {}
    bool Check(const snaps::Block& block) const override
    {
        return not block.IsDynamic;
    }
    std::string Expectation() const override { return "static block"; }
};

struct BlockAlignedAt : SingleGridChecker
{
    BlockAlignedAt(const int gridX, const int gridY)
        : SingleGridChecker(gridX, gridY), m_ExpectedX(gridX * snaps::BOX_SIZE), m_ExpectedY(gridY * snaps::BOX_SIZE) {}
    bool Check(const snaps::Block& block) const override
    {
        const Vector2 expectedPos = {static_cast<float>(m_ExpectedX), static_cast<float>(m_ExpectedY)};
        return block.WorldPosition == expectedPos;
    }
    std::string Expectation() const override {
        return std::format("block aligned ({}, {}) to grid", m_ExpectedX, m_ExpectedY);
    }

    const int m_ExpectedX;
    const int m_ExpectedY;
};
