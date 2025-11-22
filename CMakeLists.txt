cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "boids")
project(${PROJECT_NAME})

find_package(SDL2 2.32.2 EXACT REQUIRED)
find_package(SDL2_image REQUIRED)
#find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(ASSIMP_INCLUDE_DIRS "C:/Arquivos de Programas/Assimp/include")
set(ASSIMP_LIBRARIES "C:/Arquivos de Programas/Assimp/lib/x64/assimp-vc143-mt.lib")
include_directories(${ASSIMP_INCLUDE_DIRS})
link_libraries(${ASSIMP_LIBRARIES})

set(FREEGLUT_INCLUDE_DIRS "C:/Arquivos de Programas/freeglut/include")
set(FREEGLUT_LIBRARIES "C:/Arquivos de Programas/freeglut/lib/x64/freeglut.lib")
include_directories(${FREEGLUT_INCLUDE_DIRS})
link_libraries(${FREEGLUT_LIBRARIES})

set(GLEW_INCLUDE_DIRS "C:/Arquivos de Programas/Glew/include")
set(GLEW_LIBRARIES "C:/Arquivos de Programas/Glew/lib/Release/x64/glew32.lib")
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})


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
        ${ASSIMP_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${FREEGLUT_LIBRARIES}
        SDL2::SDL2
        SDL2_image::SDL2_image
        OpenGL::GL
        glu32
)
