#include "World.h"
#include <algorithm>
#include <GL/glut.h>
#include <cmath>
#include "Random.h"

World::World()
    :mGoal(nullptr)
    ,mCameraMode(CameraMode::Tower)
{
}


void World::Init() {
    Random::Init();

    // cria alguns boids iniciais
    for (int i = 0; i < 20; i++) {
        new Boid(this);
    }

    mGoal = new Boid(this);
    mGoal->SetColor(Vector3::UnitZ);
}

void World::Update(float deltaTime) {
    for (auto b : mBoids) {
        b->Update(deltaTime);
    }
}

void World::Draw() {
    SetCamera();

    GLfloat light_pos[] = { 10.0f, 100.0f, 50.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    DrawGround();
    DrawTower();

    for (auto b : mBoids) {
        b->Draw();
    }
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
    glutSolidCone(3, 15, 20, 20);
    glPopMatrix();
}

void World::SetCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Calcula o centro do bando
    Vector3 center(0, 0, 0);
    for (auto b : mBoids) {
        center += b->GetPosition();
    }
    center *= 1 / static_cast<float>(mBoids.size());

    if (mCameraMode == CameraMode::Tower)
        gluLookAt(0, 20, 0, center.x, center.y, center.z, 0, 1, 0);
    else if (mCameraMode == CameraMode::Behind)
        gluLookAt(center.x - 20, center.y + 5, center.z - 20, center.x, center.y, center.z, 0, 1, 0);
    else
        gluLookAt(center.x + 30, center.y + 10, center.z, center.x, center.y, center.z, 0, 1, 0);
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

    // Controle do boid-objetivo
    if (mGoal) {
        mGoal->HandleKey(keyStates, prevKeyStates);
    }
}

void World::AddBoid(Boid *boid) {
    mBoids.emplace_back(boid);
}

void World::RemoveBoid() {
    if (mBoids.empty())
        return;

    // Caso haja apenas 1 boid, e ele for o goal, não pode remover
    if (mBoids.size() == 1 && mBoids.back() == mGoal)
        return;

    // Caso o último boid não seja o goal, remove ele normalmente
    if (mBoids.back() != mGoal)
    {
        delete mBoids.back();
        mBoids.pop_back();
        return;
    }

    // Caso o último boid seja o goal, procura outro para remover
    for (auto it = mBoids.rbegin(); it != mBoids.rend(); ++it)
    {
        if (*it != mGoal)
        {
            delete *it;
            // remove usando erase (precisamos converter reverse_iterator)
            mBoids.erase(std::next(it).base());
            return;
        }
    }
}
