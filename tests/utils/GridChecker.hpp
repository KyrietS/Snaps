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

struct DynamicBlockAt : GridChecker
{
    DynamicBlockAt(const int gridX, const int gridY) : m_GridX(gridX), m_GridY(gridY) {}
    CheckResult Check(const snaps::Grid& grid) const override
    {
        const auto& block = grid[m_GridX, m_GridY];
        const std::string summary = std::format("Expected dynamic block at: ({}, {})", m_GridX, m_GridY);
        if (block.has_value() and block->IsDynamic) {
            return CheckResult::OkAt(m_GridX, m_GridY, summary);
        }

        return CheckResult::FailAt(m_GridX, m_GridY, summary);
    }

private:
    const int m_GridX;
    const int m_GridY;
};
