#pragma once
#include "utils/Printers.hpp"
#include "snaps/Grid.hpp"
#include <set>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


namespace snaps
{
class Grid;
}

namespace check {
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

struct Block : GridChecker {
    Block(const int gridX, const int gridY, const testing::Matcher<const snaps::Block&>& blockMatcher)
        : m_GridX(gridX), m_GridY(gridY), m_BlockMatcher(blockMatcher) {}
    CheckResult Check(const snaps::Grid& grid) const override {
        const auto& block = grid[m_GridX, m_GridY];
        const bool matches = block and m_BlockMatcher.Matches(block.value());

        std::ostringstream matchSummary;
        m_BlockMatcher.DescribeTo(&matchSummary);

        if (block.has_value() and not matches) {
            std::stringstream explanation;
            m_BlockMatcher.ExplainMatchResultTo(block.value(), &explanation);
            if (not explanation.str().empty()) {
                matchSummary << ". Actual block " << explanation.str();
            }
        } else if (not block.has_value()) {
            matchSummary << ". Actual block is empty";
        }

        CheckResult result;
        result.Success = matches;
        result.Summary = std::format("Expected block {} at ({}, {})", matchSummary.str(), m_GridX, m_GridY);
        result.Positions.insert({m_GridX, m_GridY});
        return result;
    }
    const int m_GridX;
    const int m_GridY;
    const testing::Matcher<const snaps::Block&> m_BlockMatcher;
};

struct EmptyBlock : GridChecker
{
    EmptyBlock(const int gridX, const int gridY) : m_GridX(gridX), m_GridY(gridY) {}
    CheckResult Check(const snaps::Grid& grid) const override
    {
        const auto& block = grid[m_GridX, m_GridY];
        const std::string summary = std::format("Expected block is empty at ({}, {})", m_GridX, m_GridY);
        if (not block) {
            return CheckResult::OkAt(m_GridX, m_GridY, summary);
        }
        return CheckResult::FailAt(m_GridX, m_GridY, summary);
    }

private:
    const int m_GridX;
    const int m_GridY;
};

inline auto BlockIsEmptyAt(const int gridX, const int gridY) {
    return EmptyBlock(gridX, gridY);
}

MATCHER(IsDynamic, "") {
    return arg.IsDynamic;
}
inline auto BlockIsDynamicAt(const int gridX, const int gridY) {
    return Block(gridX, gridY, IsDynamic());
}

MATCHER(IsStatic, "") {
    return not arg.IsDynamic;
}
inline auto BlockIsStaticAt(const int gridX, const int gridY) {
    return Block(gridX, gridY, IsStatic());
}

MATCHER_P2(IsAligned, x, y, std::format("is aligned to ({}, {})", x, y)){
    const Vector2 expectedPos = {static_cast<float>(x), static_cast<float>(y)};
    return arg.WorldPosition == expectedPos;
}
inline auto BlockIsAlignedAt(const int gridX, const int gridY) {
    return Block(gridX, gridY, IsAligned(gridX * snaps::BOX_SIZE, gridY * snaps::BOX_SIZE));
}

inline testing::Matcher<Vector2> Vector(const testing::Matcher<float>& x, const testing::Matcher<float>& y) {
    return testing::AllOf(
        testing::Field("x", &Vector2::x, x),
        testing::Field("y", &Vector2::y, y)
    );
}
inline auto BlockVelocityIsAt(const int gridX, const int gridY, const testing::Matcher<Vector2>& velocity) {
    return Block(gridX, gridY, Field("Velocity", &snaps::Block::Velocity, velocity));
}

MATCHER_P2(Velocity, x, y, std::format("velocity ({}, {})", x, y)) {
    *result_listener << "velocity " << arg.Velocity;
    return arg.Velocity == Vector2{ x, y };
}
inline auto BlockVelocityAt(const int gridX, const int gridY, const Vector2 vector) {
    return Block(gridX, gridY, Velocity(vector.x, vector.y));
}

MATCHER(IsNotMoving, "") {
    *result_listener << "velocity " << arg.Velocity;
    return arg.Velocity == Vector2{ 0, 0 };
}
inline auto BlockIsNotMovingAt(const int gridX, const int gridY) {
    return Block(gridX, gridY, IsNotMoving());
}

MATCHER(IsMovingDown, "") {
    *result_listener << "velocity " << arg.Velocity;
    return arg.Velocity.y > 0.0f;
}
inline auto BlockIsMovingDownAt(const int gridX, const int gridY) {
    return Block(gridX, gridY, IsMovingDown());
}

}