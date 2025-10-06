#include "TestGrid.hpp"

namespace {

constexpr Color STONE_COLOR = {128, 128, 128, 255};

Snaps::Block CopyAtPos(Snaps::Block block, int x, int y) {
    block.WorldPosition = {static_cast<float>(x) * Snaps::BOX_SIZE, static_cast<float>(y) * Snaps::BOX_SIZE};
    return block;
}

void AddBorder(Snaps::Grid& grid, Snaps::Block block) {
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

Snaps::Grid MakeTestGrid(int width, int height) {
    Snaps::Grid grid(width, height);
    Snaps::Block block {
        .FillColor = STONE_COLOR,
        .IsDynamic = false
    };
    AddBorder(grid, block);
    return grid;
}
