#include "TestGrid.hpp"

namespace {
constexpr Color STONE_COLOR = {128, 128, 128, 255};

snaps::Block CopyAtPos(snaps::Block block, int x, int y) {
    block.WorldPosition = {static_cast<float>(x) * snaps::BOX_SIZE, static_cast<float>(y) * snaps::BOX_SIZE};
    return block;
}

void AddBorder(snaps::Grid& grid, const snaps::Block& block) {
    // Top and bottom
    for (int x = 0; x < grid.Width(); x++) {
        grid[x, 0] = CopyAtPos(block, x, 0);
        grid[x, grid.Height() - 1] = CopyAtPos(block, x, grid.Height() - 1);
    }
    // Left and right
    for (int y = 0; y < grid.Height(); y++) {
        grid[0, y] = CopyAtPos(block, 0, y);
        grid[grid.Width() - 1, y] = CopyAtPos(block, grid.Width() - 1, y);
    }
}
}

snaps::Grid MakeTestGrid(const int width, const int height) {
    snaps::Grid grid(width, height);
    snaps::Block block{
        .FillColor = STONE_COLOR,
        .IsDynamic = false
    };
    AddBorder(grid, block);
    return grid;
}
