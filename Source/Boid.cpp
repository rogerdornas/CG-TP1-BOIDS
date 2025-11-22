#include "Boid.h"
#include <GL/glut.h>
#include "Random.h"
#include "World.h"

Boid::Boid(World* world)
    :mWorld(world)
    ,mYaw(0.0f)
    ,mPitch(0.0f)
    ,mSpeed(0.0f)
	,mMaxSpeed(20.0f)
{
    mPosition = Vector3(Random::GetFloatRange(-10.0f, 10.0f), Random::GetFloatRange(5.0f, 15.0f), Random::GetFloatRange(-10.0f, 10.0f));
	mYaw = Random::GetFloatRange(0.0f, 360.0f);
    mPrevYaw = mYaw;
	mPitch = Random::GetFloatRange(-20.0f, 20.0f);
    mRoll = 0.0f;
    mColor = Vector3(0.0f, 1.0f, 0.0f);

    // Inicializa animação dessincronizada
    mAnimPhase = Random::GetFloatRange(0.0f, Math::TwoPi);
    mFlapSpeed = Random::GetFloatRange(15.0f, 25.0f); // Alguns batem mais rápido que outros

    mWorld->AddBoid(this);
}

void Boid::Update(float deltaTime) {
    // Calcula o quanto o boid girou neste frame
    float yawDiff = mYaw - mPrevYaw;

    // Lida com a virada de 360 para 0 graus
    if (yawDiff > 180.0f) yawDiff -= 360.0f;
    if (yawDiff < -180.0f) yawDiff += 360.0f;

    // Define o Roll Alvo
    float targetRoll = yawDiff * -10.0f;

    // Limita o ângulo máximo de inclinação
    if (targetRoll > 60.0f) targetRoll = 60.0f;
    if (targetRoll < -60.0f) targetRoll = -60.0f;

    // Suavização (Interpolation)

    float bankSpeed = 10.0f;

    // Se não estiver virando (yawDiff perto de 0), volta ao centro mais devagar (efeito inércia)
    if (fabs(yawDiff) < 0.1f) {
        bankSpeed = 3.0f;
    }

    mRoll += (targetRoll - mRoll) * bankSpeed * deltaTime;

    // Atualiza o ciclo de animação
    mAnimPhase += mFlapSpeed * deltaTime;

    if (mAnimPhase > Math::TwoPi) {
        mAnimPhase -= Math::TwoPi;
	}

    float yawRad = mYaw * Math::Pi / 180.0f;
    float pitchRad = mPitch * Math::Pi / 180.0f;

    // Direção frente considerando yaw e pitch
    Vector3 forward(
        cosf(pitchRad) * sinf(yawRad),  // x
        sinf(pitchRad),                 // y
        cosf(pitchRad) * cosf(yawRad)   // z
    );

    forward.Normalize();
    mVelocity = forward * mSpeed;
    mPosition += mVelocity * deltaTime;

    mPrevYaw = mYaw;

}

Vector3 Boid::CalculateNormal(Vector3 v1, Vector3 v2, Vector3 v3) {
    Vector3 edge1 = v2 - v1;
    Vector3 edge2 = v3 - v1;
    Vector3 normal = Vector3::Cross(edge1, edge2);
    normal.Normalize();
    return normal;
}

void Boid::DrawBirdModel(float wingOffset) {
    const float s = 0.5f;

    // Vértices fixos do corpo
    Vector3 vTip(0.0f * s, 0.0f * s, 2.5f * s);
    Vector3 vBeakBase(0.0f * s, 0.3f * s, 1.5f * s);
    Vector3 vNeck(0.0f * s, 0.5f * s, 0.5f * s);
    Vector3 vTailTip(0.0f * s, 0.2f * s, -1.5f * s);
    Vector3 vBelly(0.0f * s, -0.3f * s, 0.0f * s);
    Vector3 vBodySideR(0.4f * s, 0.0f * s, 0.5f * s);
    Vector3 vBodySideL(-0.4f * s, 0.0f * s, 0.5f * s);

    // --- APLICANDO A ANIMAÇÃO NAS ASAS ---
    // Somamos o wingOffset no eixo Y das pontas das asas
    Vector3 vWingR(2.5f * s, 0.2f * s + wingOffset, -0.5f * s);
    Vector3 vWingL(-2.5f * s, 0.2f * s + wingOffset, -0.5f * s);

    Vector3 normal;

    glBegin(GL_TRIANGLES);

    // --- BICO ---
    glColor3f(0.8f, 0.1f, 0.1f);

    // Bico Superior Dir
    normal = CalculateNormal(vTip, vBodySideR, vBeakBase);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vTip.x, vTip.y, vTip.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);
    glVertex3f(vBeakBase.x, vBeakBase.y, vBeakBase.z);

    // Bico Superior Esq
    normal = CalculateNormal(vTip, vBeakBase, vBodySideL);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vTip.x, vTip.y, vTip.z);
    glVertex3f(vBeakBase.x, vBeakBase.y, vBeakBase.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);

    // Bico Inferior Dir
    normal = CalculateNormal(vTip, vBelly, vBodySideR);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vTip.x, vTip.y, vTip.z);
    glVertex3f(vBelly.x, vBelly.y, vBelly.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);

    // Bico Inferior Esq
    normal = CalculateNormal(vTip, vBodySideL, vBelly);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vTip.x, vTip.y, vTip.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);
    glVertex3f(vBelly.x, vBelly.y, vBelly.z);

    // --- CORPO ---
    glColor3f(mColor.x, mColor.y, mColor.z);

    // Costas
    normal = CalculateNormal(vNeck, vBodySideR, vTailTip);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vNeck.x, vNeck.y, vNeck.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);
    glVertex3f(vTailTip.x, vTailTip.y, vTailTip.z);

    normal = CalculateNormal(vNeck, vTailTip, vBodySideL);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vNeck.x, vNeck.y, vNeck.z);
    glVertex3f(vTailTip.x, vTailTip.y, vTailTip.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);

    // Conexões Pescoço
    normal = CalculateNormal(vBeakBase, vNeck, vBodySideR);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBeakBase.x, vBeakBase.y, vBeakBase.z);
    glVertex3f(vNeck.x, vNeck.y, vNeck.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);

    normal = CalculateNormal(vBeakBase, vBodySideL, vNeck);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBeakBase.x, vBeakBase.y, vBeakBase.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);
    glVertex3f(vNeck.x, vNeck.y, vNeck.z);

    // Barriga
    normal = CalculateNormal(vBelly, vTailTip, vBodySideR);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBelly.x, vBelly.y, vBelly.z);
    glVertex3f(vTailTip.x, vTailTip.y, vTailTip.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);

    normal = CalculateNormal(vBelly, vBodySideL, vTailTip);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBelly.x, vBelly.y, vBelly.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);
    glVertex3f(vTailTip.x, vTailTip.y, vTailTip.z);

    // --- ASAS (Vértices recalculados dinamicamente) ---

    // Asa Direita Cima
    normal = CalculateNormal(vBodySideR, vWingR, vNeck);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);
    glVertex3f(vWingR.x, vWingR.y, vWingR.z);
    glVertex3f(vNeck.x, vNeck.y, vNeck.z);

    // Asa Direita Baixo
    normal = CalculateNormal(vBodySideR, vBelly, vWingR);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBodySideR.x, vBodySideR.y, vBodySideR.z);
    glVertex3f(vBelly.x, vBelly.y, vBelly.z);
    glVertex3f(vWingR.x, vWingR.y, vWingR.z);

    // Asa Esquerda Cima
    normal = CalculateNormal(vBodySideL, vNeck, vWingL);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);
    glVertex3f(vNeck.x, vNeck.y, vNeck.z);
    glVertex3f(vWingL.x, vWingL.y, vWingL.z);

    // Asa Esquerda Baixo
    normal = CalculateNormal(vBodySideL, vWingL, vBelly);
    glNormal3f(normal.x, normal.y, normal.z);
    glVertex3f(vBodySideL.x, vBodySideL.y, vBodySideL.z);
    glVertex3f(vWingL.x, vWingL.y, vWingL.z);
    glVertex3f(vBelly.x, vBelly.y, vBelly.z);

    glEnd();
}


void Boid::Draw() {
     glPushMatrix();
     glTranslatef(mPosition.x, mPosition.y, mPosition.z);

     glRotatef(mYaw, 0.0f, 1.0f, 0.0f);
     glRotatef(-mPitch, 1.0f, 0.0f, 0.0f);
     glRotatef(mRoll, 0.0f, 0.0f, 1.0f);

     // --- CÁLCULO DA ANIMAÇÃO DA ASA ---
    // sin(mAnimPhase) vai de -1 a 1. 
    // Multiplicamos por 0.5f para definir a amplitude (altura) da batida.
     float wingOffset = sinf(mAnimPhase) * 0.5f;

     DrawBirdModel(wingOffset);

     glPopMatrix();
}

void Boid::HandleKey(std::map<unsigned char, bool> keyStates, std::map<unsigned char, bool> prevKeyStates) {
    const float rotSpeed = 3.0f;   // graus por tecla
    const float accel = 0.2f;      // aceleração

    // Rotação horizontal (Yaw)
    if (keyStates['a']) {
        mYaw += rotSpeed;
    }
    if (keyStates['d']) {
        mYaw -= rotSpeed;
    }

    // Rotação vertical (Pitch)
    if (keyStates['i']) {          // subir
        mPitch += rotSpeed;
        if (mPitch > 89.0f) mPitch = 89.0f; // evita travar no topo
    }
    if (keyStates['k']) {          // descer
        mPitch -= rotSpeed;
        if (mPitch < -89.0f) mPitch = -89.0f;
    }

    // Controle de velocidade
    if (keyStates['w']) {
        mSpeed += accel;
        if (mSpeed > mMaxSpeed) {
            mSpeed = mMaxSpeed;
        }
    }
    if (keyStates['s']) {
        mSpeed -= accel;
        if (mSpeed < 0.0f) {
            mSpeed = 0.0f;
        }
    }

    // Parar
    if (keyStates[' ']) {
        mSpeed = 0.0f;
    }
}
