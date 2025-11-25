#pragma once
#include "Math.h"
#include <map>

class Boid {
public:
    Boid(class World* world);

    virtual void Update(float deltaTime);
    void Draw(bool isShadow = false);

    Vector3 GetPosition() { return mPosition; }
    void SetPosition(Vector3 pos) { mPosition = pos; }

    Vector3 GetVelocity() { return mVelocity; }
    void SetVelocity(Vector3 velocity) { mVelocity = velocity; }

    void SetColor(Vector3 color) { mColor = color; }

    void HandleKey(std::map<unsigned char, bool> keyStates, std::map<unsigned char, bool> prevKeyStates);

protected:
	Vector3 CalculateNormal(Vector3 v1, Vector3 v2, Vector3 v3);
	void DrawBirdModel(float wingOffset, bool isShadow);


    class World* mWorld;

    Vector3 mPosition;
    Vector3 mVelocity;
    Vector3 mColor;

    float mYaw;      // ângulo de rotação em torno do eixo Y
    float mPrevYaw;
    float mPitch;
    float mRoll;
    float mSpeed;    // velocidade atual
	float mMaxSpeed; // velocidade máxima

    // Variáveis de Animação
    float mAnimPhase; // Posição atual no ciclo da animação (0 a 2*PI)
    float mFlapSpeed; // Velocidade da batida de asas
};
