#include <GL/glut.h>
#include "World.h"
#include <map>

World world;
std::map<unsigned char, bool> keyStates;      // estado atual
std::map<unsigned char, bool> prevKeyStates;  // estado anterior

// Tamanho da janela
int windowWidth = 800;
int windowHeight = 600;

// Inicialização do OpenGL
void initGL() {
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_NORMALIZE);

    glShadeModel(GL_SMOOTH);
}

// Função de renderização
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world.Draw();

    glutSwapBuffers();
}

// Atualização da simulação
void update(int value) {
    world.HandleKey(keyStates, prevKeyStates);

    // Atualiza estados anteriores
    prevKeyStates = keyStates;

    world.Update(0.016f); // 60 FPS
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// Redimensionamento
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 1.0, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    keyStates[key] = true; // marca como pressionada
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false; // marca como solta
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Boids 3D");

    initGL();
    world.Init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}