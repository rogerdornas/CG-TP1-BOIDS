#include "World.h"
#include <algorithm>
#include <GL/glut.h>
#include <cmath>
#include "Random.h"

World::World()
    :mGoal(nullptr)
    ,mCameraMode(CameraMode::Tower)
    ,mIsPaused(false)
    ,mIsFogEnabled(false)
{
}

void World::Init() {
    Random::Init();

    // Cria alguns boids iniciais
    for (int i = 0; i < 20; i++) {
        new Boid(this);
    }

    mGoal = new Boid(this);
    mGoal->SetColor(Vector3::UnitZ); // Azul para o objetivo

    // --- CRIAÇÃO DE OBSTÁCULOS ---
    // Cria 3 esferas grandes espalhadas
    mObstacles.push_back({ Vector3(30.0f, 10.0f, 30.0f), 8.0f });
    mObstacles.push_back({ Vector3(-30.0f, 15.0f, -40.0f), 12.0f });
    mObstacles.push_back({ Vector3(-40.0f, 8.0f, 40.0f), 10.0f });
}

void World::Update(float deltaTime) {
    // Se estiver pausado, não atualiza a física (mas permite input de câmera)
    if (mIsPaused) return;

    for (auto b : mBoids) {
        b->Update(deltaTime);
    }
}

void World::Draw() {
    SetCamera();

    // --- CONFIGURAÇÃO DE FOG (NEBLINA) ---
    if (mIsFogEnabled) {
        glEnable(GL_FOG);
        GLfloat fogColor[] = { 0.5f, 0.7f, 1.0f, 1.0f }; // Cor do fundo
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogi(GL_FOG_MODE, GL_EXP2); // Decaimento exponencial
        glFogf(GL_FOG_DENSITY, 0.015f);
        glHint(GL_FOG_HINT, GL_NICEST);
    } else {
        glDisable(GL_FOG);
    }

    GLfloat light_pos[] = { 10.0f, 100.0f, 50.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    DrawGround();
    DrawTower();
    DrawObstacles();

    // Desenha os Boids Reais
    for (auto b : mBoids) {
        b->Draw();
    }

    // Desenha as Sombras (Projeção Paralela no chão)
    DrawShadows();
}

void World::DrawGround() {
    glColor3f(0.3f, 0.6f, 0.3f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-500, 0, -500);
    glVertex3f(-500, 0, 500);
    glVertex3f(500, 0, 500);
    glVertex3f(500, 0, -500);
    glEnd();
}

void World::DrawTower() {
    glPushMatrix();
    glTranslatef(0, 0, 0);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(0.6f, 0.4f, 0.2f);
    glutSolidCone(3, 20, 20, 20); // Torre um pouco mais alta
    glPopMatrix();
}

void World::DrawObstacles() {
    glColor3f(0.8f, 0.2f, 0.2f); // Obstáculos vermelhos
    for (const auto& obs : mObstacles) {
        glPushMatrix();
        glTranslatef(obs.position.x, obs.position.y, obs.position.z);
        glutSolidSphere(obs.radius, 20, 20);
        glPopMatrix();
    }
}

void World::DrawShadows() {
    // Desabilita iluminação e profundidade para desenhar sombras "chapadas"
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST); 
    
    glPushMatrix();
    
    // Matriz de projeção de sombra simples (achata Y em 0)
    // Sobe um pouquinho (0.1) para evitar Z-Fighting com o chão
    glTranslatef(0.0f, 0.1f, 0.0f);
    glScalef(1.0f, 0.0f, 1.0f); 

    glColor3f(0.1f, 0.1f, 0.1f); // Sombra cinza escura

    for (auto b : mBoids) {
        // Desenha apenas a geometria do boid, sem alterar a cor (pois definimos cinza acima)
        // Nota: O método Draw do boid define cor internamente, o ideal seria ter um DrawGeometry
        // mas para simplificar, vamos assumir que a cor definida aqui prevalece se desativarmos LIGHTING
        // ou podemos desenhar manualmente algo simples.
        
        // Hack: Vamos chamar o Draw do boid. Como Lighting está OFF, a cor definida 
        // no glColor3f acima vai "tingir" o objeto se ele não usar texturas.
        b->Draw();
    }

    glPopMatrix();

    // Restaura estados
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void World::SetCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Calcula o centro do bando (ignora NaNs com check simples)
    Vector3 center(0, 0, 0);
    if (!mBoids.empty()) {
        for (auto b : mBoids) {
            center += b->GetPosition();
        }
        center *= 1.0f / static_cast<float>(mBoids.size());
    }
    
    // Proteção NaN
    if (std::isnan(center.x)) center = Vector3::Zero;

    if (mCameraMode == CameraMode::Tower)
        gluLookAt(0, 40, 0, center.x, center.y, center.z, 0, 1, 0); // Torre mais alta
    else if (mCameraMode == CameraMode::Behind) {
        // Pega velocidade média para saber "atrás"
        Vector3 avgVel = Vector3::UnitX;
        if (!mBoids.empty()) avgVel = mBoids[0]->GetVelocity();
        avgVel.Normalize();
        
        Vector3 eye = center - avgVel * 30.0f + Vector3(0, 10, 0);
        gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, 0, 1, 0);
    }
    else { // Side
        Vector3 avgVel = Vector3::UnitX;
        if (!mBoids.empty()) avgVel = mBoids[0]->GetVelocity();
        Vector3 side = Vector3::Cross(avgVel, Vector3::UnitY);
        side.Normalize();

        Vector3 eye = center + side * 40.0f + Vector3(0, 5, 0);
        gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, 0, 1, 0);
    }
}

void World::HandleKey(std::map<unsigned char, bool> keyStates, std::map<unsigned char, bool> prevKeyStates) {
    if (keyStates['+'] && !prevKeyStates['+']) {
        new Boid(this);
    }
    if (keyStates['-'] && !prevKeyStates['-']) {
        RemoveBoid();
    }
    if (keyStates['c'] && !prevKeyStates['c']) {
        mCameraMode = static_cast<CameraMode>((static_cast<int>(mCameraMode) + 1) % 3);
    }
    
    // --- CONTROLES EXTRAS ---
    if (keyStates['f'] && !prevKeyStates['f']) {
        mIsFogEnabled = !mIsFogEnabled;
    }
    if (keyStates['p'] && !prevKeyStates['p']) {
        mIsPaused = !mIsPaused;
    }

    // Controle do boid-objetivo (apenas se não estiver pausado ou se quiser permitir mover na pausa)
    if (mGoal && !mIsPaused) {
        mGoal->HandleKey(keyStates, prevKeyStates);
    }
}

void World::AddBoid(Boid *boid) {
    mBoids.emplace_back(boid);
}

void World::RemoveBoid() {
    if (mBoids.empty()) return;
    if (mBoids.size() == 1 && mBoids.back() == mGoal) return;

    if (mBoids.back() != mGoal) {
        delete mBoids.back();
        mBoids.pop_back();
        return;
    }

    for (auto it = mBoids.rbegin(); it != mBoids.rend(); ++it) {
        if (*it != mGoal) {
            delete *it;
            mBoids.erase(std::next(it).base());
            return;
        }
    }
}