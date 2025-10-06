#pragma once
#include "Grid.hpp"
#include <stack>


namespace snaps {
class SnapsEngine {
public:
    SnapsEngine(Grid& grid);

    void Step(float deltaTime);

private:
    void SimulatePhysics();
    void Integrate(Block&);
    void SolveGridPhysics(int gridX, int gridY);

    void SolveMovementRight(int& gridX, int& gridY, Block& block);
    void SolveMovementLeft(int& gridX, int& gridY, Block& block);
    void SolveMovementUp(int& gridX, int& gridY, Block& block);
    void SolveMovementDown(int& gridX, int& gridY, Block& block);

    Grid& m_Grid;

    struct SecondPassContact {
        int x;
        int y;
    };

    float m_DeltaTime = 0.0f;
    bool m_SecondPass = false;
    std::stack<SecondPassContact> m_RightMovementContacts;
    std::stack<SecondPassContact> m_UpMovementContacts;
};
}
