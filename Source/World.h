#pragma once
#include "Boid.h"
#include <vector>
#include <map>

// Definição de obstáculo
struct Obstacle {
    Vector3 position;
    float radius;
};

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
    std::vector<Obstacle>& GetObstacles() { return mObstacles; } 

private:
    enum class CameraMode {
        Tower,
        Behind,
        Side
    };
    
    std::vector<Boid*> mBoids;
    std::vector<Obstacle> mObstacles; 
    Boid* mGoal;
    CameraMode mCameraMode;

    // Estados Globais
    bool mIsPaused;
    bool mIsFogEnabled;

    void SetCamera();
    void DrawGround();
    void DrawTower();
    void DrawObstacles();
    void DrawShadows(); 
};