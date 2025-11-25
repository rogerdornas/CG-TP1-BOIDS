#include "Boid.h"
#include <GL/glut.h>
#include "Random.h"
#include "World.h"
#include <vector>
#include <cmath>

Boid::Boid(World* world)
    :mWorld(world)
    ,mYaw(0.0f)
    ,mPitch(0.0f)
    ,mSpeed(0.0f)
	,mMaxSpeed(20.0f)
{
    // Inicialização aleatória para dar variedade ao bando inicial
    mPosition = Vector3(Random::GetFloatRange(-10.0f, 10.0f), Random::GetFloatRange(25.0f, 35.0f), Random::GetFloatRange(-10.0f, 10.0f));
	mYaw = Random::GetFloatRange(0.0f, 360.0f);
    mPrevYaw = mYaw;
	mPitch = Random::GetFloatRange(-20.0f, 20.0f);
    mRoll = 0.0f;
    mColor = Vector3(0.5f, 0.5f, 1.0f); // Cor padrão azulada para o bando

    // Se este boid for criado e já houver um objetivo, define uma velocidade inicial
    if (mWorld->GetGoal() && this != mWorld->GetGoal()) {
        mSpeed = mMaxSpeed * 0.8f;
    }

    // Inicializa animação dessincronizada [cite: 26, 27]
    mAnimPhase = Random::GetFloatRange(0.0f, Math::TwoPi);
    mFlapSpeed = Random::GetFloatRange(15.0f, 25.0f); 

    mWorld->AddBoid(this);
}

void Boid::Update(float deltaTime) {
    
    bool isGoal = (this == mWorld->GetGoal());

    // --- LÓGICA DE FLOCKING (Bando) ---
    if (!isGoal) {
        const float separationWeight = 1.5f;
        const float alignmentWeight = 1.0f;
        const float cohesionWeight = 0.8f;
        const float goalWeight = 1.2f; // Peso forte para não perderem o líder
        
        const float perceptionRadius = 20.0f;
        const float separationRadius = 8.0f;

        Vector3 separation(0,0,0);
        Vector3 alignment(0,0,0);
        Vector3 cohesion(0,0,0);
        Vector3 goalForce(0,0,0);

        Vector3 centerOfMass(0,0,0);
        int neighborCount = 0;

        std::vector<Boid*> boids = mWorld->GetBoids();
        Boid* goalBoid = mWorld->GetGoal();

        for (auto other : boids) {
            if (other == this) continue;

            float dist = Vector3::Distance(mPosition, other->GetPosition());

            // Check vital: evita processar boids na mesma exata posição (dist=0)
            if (dist > 0.001f && dist < perceptionRadius) {
                
                // 1. Separação
                if (dist < separationRadius) {
                    Vector3 push = mPosition - other->GetPosition();
                    // PROTEÇÃO: Só normaliza se vetor não for nulo
                    if (push.LengthSq() > 0.001f) {
                        push.Normalize();
                        separation += push * (1.0f / dist); 
                    }
                }

                // 2. Alinhamento
                alignment += other->GetVelocity();

                // 3. Coesão
                centerOfMass += other->GetPosition();

                neighborCount++;
            }
        }

        if (neighborCount > 0) {
            // Alinhamento
            // PROTEÇÃO: Só normaliza se a média das velocidades não for nula
            if (alignment.LengthSq() > 0.001f) {
                alignment.Normalize();
            }

            // Coesão
            centerOfMass *= (1.0f / static_cast<float>(neighborCount));
            Vector3 directionToCenter = centerOfMass - mPosition;
            // PROTEÇÃO
            if (directionToCenter.LengthSq() > 0.001f) {
                directionToCenter.Normalize();
                cohesion = directionToCenter;
            }
        }

        // 4. Busca do Objetivo
        if (goalBoid) {
            Vector3 directionToGoal = goalBoid->GetPosition() - mPosition;
            // PROTEÇÃO
            if (directionToGoal.LengthSq() > 0.001f) {
                directionToGoal.Normalize();
                goalForce = directionToGoal;
            }
        }

        // Soma vetorial
        Vector3 steering = (separation * separationWeight) +
                           (alignment * alignmentWeight) +
                           (cohesion * cohesionWeight) +
                           (goalForce * goalWeight);

        // Aplica forças
        // PROTEÇÃO CRÍTICA: Se steering for zero, não fazemos nada para evitar NaN
        if (steering.LengthSq() > 0.001f) {
            steering.Normalize();
            Vector3 targetVelocity = steering * mMaxSpeed;
            
            float turnSpeed = 5.0f * deltaTime; // Resposta mais rápida
            mVelocity = Vector3::Lerp(mVelocity, targetVelocity, turnSpeed);
        }

        // Velocidade mínima para evitar parar e gerar erros de normalização no futuro
        if (mVelocity.LengthSq() < 0.1f) {
             // Se parou, dá um empurrãozinho para frente (ou eixo Z se for zero total)
             if (mVelocity.LengthSq() < 0.0001f) mVelocity = Vector3(0,0,1);
             
             Vector3 vNorm = mVelocity;
             vNorm.Normalize();
             mVelocity = vNorm * 2.0f; // Velocidade mínima
        }
        
        mPosition += mVelocity * deltaTime;

        // Atualiza Yaw/Pitch baseado na velocidade
        if (mVelocity.LengthSq() > 0.001f) {
            Vector3 dir = mVelocity;
            dir.Normalize();
            mYaw = Math::ToDegrees(atan2f(dir.x, dir.z));
            mPitch = Math::ToDegrees(asinf(Math::Clamp(dir.y, -1.0f, 1.0f)));
        }
        
        mSpeed = mVelocity.Length();
    }
    // --- LÓGICA DO LÍDER (Objetivo) ---
    else {
        float yawRad = Math::ToRadians(mYaw);
        float pitchRad = Math::ToRadians(mPitch);

        Vector3 forward(
            cosf(pitchRad) * sinf(yawRad),
            sinf(pitchRad),
            cosf(pitchRad) * cosf(yawRad)
        );
        // PROTEÇÃO
        if (forward.LengthSq() > 0.001f) forward.Normalize();
        
        mVelocity = forward * mSpeed;
        mPosition += mVelocity * deltaTime;
    }

    // --- ANIMAÇÃO E BANKING (Comum a todos) ---
    float yawDiff = mYaw - mPrevYaw;
    if (yawDiff > 180.0f) yawDiff -= 360.0f;
    if (yawDiff < -180.0f) yawDiff += 360.0f;

    float targetRoll = yawDiff * -8.0f;
    targetRoll = Math::Clamp(targetRoll, -60.0f, 60.0f);

    float bankSpeed = (fabs(yawDiff) < 0.1f) ? 2.0f : 5.0f;
    mRoll += (targetRoll - mRoll) * bankSpeed * deltaTime;

    float flapFactor = 1.0f + (mSpeed / mMaxSpeed); 
    mAnimPhase += (mFlapSpeed * flapFactor) * deltaTime;
    if (mAnimPhase > Math::TwoPi) mAnimPhase -= Math::TwoPi;

    mPrevYaw = mYaw;
}

// Substitua também o CalculateNormal para evitar NaN na renderização
Vector3 Boid::CalculateNormal(Vector3 v1, Vector3 v2, Vector3 v3) {
    Vector3 edge1 = v2 - v1;
    Vector3 edge2 = v3 - v1;
    Vector3 normal = Vector3::Cross(edge1, edge2);
    
    // PROTEÇÃO: Se o triângulo for degenerado (área zero), retorna Up vector padrão
    if (normal.LengthSq() > 0.0001f) {
        normal.Normalize();
    } else {
        return Vector3(0, 1, 0); 
    }
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
    // Somamos o wingOffset no eixo Y das pontas das asas [cite: 25]
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

     // Aplica as rotações baseadas no movimento calculado no Update
     glRotatef(mYaw, 0.0f, 1.0f, 0.0f);   // Direção horizontal [cite: 39]
     glRotatef(-mPitch, 1.0f, 0.0f, 0.0f);// Direção vertical (inverso no OpenGL) [cite: 38]
     glRotatef(mRoll, 0.0f, 0.0f, 1.0f);  // Inclinação nas curvas [cite: 40]

     // --- CÁLCULO DA ANIMAÇÃO DA ASA ---
    // sin(mAnimPhase) vai de -1 a 1. 
    // Multiplicamos por 0.5f para definir a amplitude (altura) da batida.
     float wingOffset = sinf(mAnimPhase) * 0.5f;

     DrawBirdModel(wingOffset);

     glPopMatrix();
}

void Boid::HandleKey(std::map<unsigned char, bool> keyStates, std::map<unsigned char, bool> prevKeyStates) {
    // Apenas o boid objetivo deve processar inputs diretamente
    // No entanto, como o World chama HandleKey no mGoal, esta lógica está correta aqui.
    // O Update() do boid objetivo usará os valores alterados aqui.
    
    const float rotSpeed = 3.0f;   // graus por tecla
    const float accel = 0.5f;      // aceleração

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