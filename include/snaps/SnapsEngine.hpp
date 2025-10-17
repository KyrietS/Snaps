#pragma once
#include "Grid.hpp"
#include <stack>


namespace snaps {
class SnapsEngine {
public:
    explicit SnapsEngine(Grid& grid);

    void Step(float deltaTime);

    void SetGravity(const float gravity) { m_Gravity = gravity; }
    float GetGravity() const { return m_Gravity; }

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

    float m_Gravity = 200.0f;
    float m_DeltaTime = 0.0f;
    bool m_SecondPass = false;
    std::stack<SecondPassContact> m_RightMovementContacts;
    std::stack<SecondPassContact> m_UpMovementContacts;
};
}
