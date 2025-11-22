cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "boids")
project(${PROJECT_NAME})

find_package(OpenGL REQUIRED)

set(FREEGLUT_INCLUDE_DIRS "C:/Arquivos de Programas/freeglut/include")
set(FREEGLUT_LIBRARIES "C:/Arquivos de Programas/freeglut/lib/x64/freeglut.lib")
include_directories(${FREEGLUT_INCLUDE_DIRS})
link_libraries(${FREEGLUT_LIBRARIES})


add_executable(${PROJECT_NAME}
        Source/Main.cpp
        Source/Math.cpp
        Source/Math.h
        Source/Random.cpp
        Source/Random.h
        Source/World.cpp
        Source/World.h
        Source/Boid.cpp
        Source/Boid.h
        Source/GoalBoid.cpp
        Source/GoalBoid.h 
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${FREEGLUT_LIBRARIES}
        OpenGL::GL
        glu32
)
