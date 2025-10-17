#pragma once
#include "Grid.hpp"
#include <stack>


namespace snaps {

struct Config {
    float Gravity = 200.0f;
    float Drag = 0.0f; // For now
};

class SnapsEngine {
public:
    explicit SnapsEngine(Grid& grid);

    void Step(float deltaTime);

    Config& GetConfig() { return m_Config; }

private:
    void SimulatePhysics();
    void Integrate(Block&);
    void SolveGridPhysics(int gridX, int gridY);

    void SolveMovementRight(int& gridX, int& gridY, Block& block);
    void SolveMovementLeft(int& gridX, int& gridY, Block& block);
    void SolveMovementUp(int& gridX, int& gridY, Block& block);
    void SolveMovementDown(int& gridX, int& gridY, Block& block);

    void ApplyGravity(Block& block);
    void ApplyFriction(Block& block, float multiplier);

    Grid& m_Grid;

    struct SecondPassContact {
        int x;
        int y;
    };

    Config m_Config;
    float m_DeltaTime = 0.0f;
    bool m_SecondPass = false;
    std::stack<SecondPassContact> m_RightMovementContacts;
    std::stack<SecondPassContact> m_UpMovementContacts;
};
}
