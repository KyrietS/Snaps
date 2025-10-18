#pragma once
#include <cassert>

#include "Block.hpp"
#include <optional>
#include <vector>


namespace snaps {
class Grid {
public:
    Grid(const int width, const int height) : m_Width(width), m_Height(height), m_Blocks(width * height, std::nullopt)
    {
    }

    bool InBounds(const int x, const int y) const {
        return (x >= 0 and y >= 0 and x < m_Width and y < m_Height);
    }

    void Remove(const int x, const int y) {
        At(x, y) = std::nullopt;
    }

    std::optional<Block>& At(const int x, const int y) {
        return m_Blocks[GetIndex(x, y)];
    }
    const std::optional<Block>& At(const int x, const int y) const {
        return m_Blocks[GetIndex(x, y)];
    }
    std::optional<Block>& At(const std::size_t index) {
        return m_Blocks[index];
    }
    const std::optional<Block>& At(const std::size_t index) const {
        return m_Blocks[index];
    }

    int Width() const { return m_Width; }
    int Height() const { return m_Height; }
    std::vector<std::optional<Block>>& Data() { return m_Blocks; }
    const std::vector<std::optional<Block>>& Data() const { return m_Blocks; }

    std::size_t GetIndex(const int x, const int y) const {
        return y * m_Width + x;
    }
    std::pair<int, int> GetXY(const std::size_t index) const {
        return {index % m_Width, index / m_Width};
    }

private:
    const int m_Width;
    const int m_Height;
    std::vector<std::optional<Block>> m_Blocks;
};
}
