cmake_minimum_required(VERSION 3.26)

# Configurando o padrão C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "boids")
project(${PROJECT_NAME})

# --- Dependências ---
find_package(OpenGL REQUIRED)

find_package(GLUT REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

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
    OpenGL::GL
    OpenGL::GLU     
    ${GLUT_LIBRARIES} 
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLUT_ROOT_PATH}/bin/x64/freeglut.dll" 
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copiando freeglut.dll para o diretório do executável..."
    )
endif()