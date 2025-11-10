#pragma once
#include "Grid.hpp"
#include <stack>


namespace snaps {

struct Config {
    float Gravity = 200.0f;
    float Drag = 0.2f;
    float SmoothSnappingMinVelocity = 10.0f;
};

class SnapsEngine {
public:
    explicit SnapsEngine(Grid& grid);

    void Step(float deltaTime);

    Config& GetConfig() { return m_Config; }

private:
    enum class CollisionPass { First, Secondary, Third };
    void SimulatePhysics();
    void SimulateMovement(int x, int y, Block& block);
    void Integrate(Block&);
    void SolveGridPhysics(int gridX, int gridY, CollisionPass);
    void SolveGridPhysics(int gridX, int gridY, Block& block, CollisionPass);
    void SecondPassGridPhysicsHorizontal();
    void ThirdPassGridPhysicsVertical();

    struct MovementResolution {
        MovementResolution(const int x, const int y, const CollisionPass pass) : X(x), Y(y), Pass(pass) {}
        int X;
        int Y;
        CollisionPass Pass;
        bool Resolved = false;
    };

    void SolveMovementHorizontal(Block&, MovementResolution&);
    void SolveMovementRight(Block&, MovementResolution&);
    void SolveMovementLeft(Block&, MovementResolution&);

    void SolveMovementVertical(Block&, MovementResolution&);
    void SolveMovementUp(Block&, MovementResolution&);
    void SolveMovementDown(Block&, MovementResolution&);

    void ApplyGravity(Block& block);
    void ApplyFriction(int x, int y, Block& block);
    void ApplyFrictionBetween(Block& block, const Block& surface);
    void ApplyDrag(Block& block);
    void DiscardResistanceForcesIfNecessary(int x, int y, Block& block);

    bool AreTouching(const Block&, const Block&) const;

    Grid& m_Grid;

    struct CollisionPassCandidate {
        int x;
        int y;
    };

    Config m_Config;
    float m_DeltaTime = 0.0f;
    std::stack<CollisionPassCandidate> m_SecondPassCandidates;
    std::stack<CollisionPassCandidate> m_ThirdPassCandidates;
};
}
