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
    enum class CollisionPass { First, Second };
    void SimulatePhysics();
    void SimulateMovement(int x, int y, Block& block);
    void Integrate(Block&);
    void SolveGridPhysics(int gridX, int gridY, CollisionPass);
    void SolveGridPhysics(int gridX, int gridY, Block& block, CollisionPass);
    void SecondPassGridPhysicsHorizontal();
    void SecondPassGridPhysicsVertical();

    struct MovementResolution {
        MovementResolution(const int x, const int y, CollisionPass pass) : X(x), Y(y), CollisionPass(pass) {}
        int X;
        int Y;
        CollisionPass CollisionPass;
        bool Resolved = false;
    };

    void SolveMovementHorizontal(Block&, MovementResolution&);
    void SolveMovementRight(Block&, MovementResolution&);
    void SolveMovementLeft(Block&, MovementResolution&);

    void SolveMovementVertical(Block& block, MovementResolution&);
    void SolveMovementUp(Block& block, MovementResolution&);
    void SolveMovementDown(Block& block, MovementResolution&);

    void ApplyGravity(Block& block);
    void ApplyFriction(Block& block, float multiplier);

    Grid& m_Grid;

    struct SecondPassContact {
        int x;
        int y;
    };

    Config m_Config;
    float m_DeltaTime = 0.0f;
    std::stack<SecondPassContact> m_RightMovementContacts;
    std::stack<SecondPassContact> m_UpMovementContacts;
};
}
