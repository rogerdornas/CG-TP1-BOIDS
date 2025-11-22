#pragma once
#include "Boid.h"
#include <vector>
#include <map>

class World {
public:
    World();

    void Init();
    void Update(float dt);
    void Draw();
    void HandleKey(std::map<unsigned char, bool> keyStates, std::map<unsigned char, bool> prevKeyStates);
    void AddBoid(Boid* boid);
    void RemoveBoid();

    Boid* GetGoal() { return mGoal; }

    std::vector<Boid*> GetBoids() { return mBoids; }

private:
    enum class CameraMode {
        Tower,
        Behind,
        Side
    };
    std::vector<Boid*> mBoids;
    Boid* mGoal; // boid-objetivo
    CameraMode mCameraMode; // 0=tower, 1=behind flock, 2=side view

    void SetCamera();
    void DrawGround();
    void DrawTower();
};
