cmake_minimum_required(VERSION 3.5)

project(projeto-final LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE SOURCES
    "src/**/*.cpp"
)

# Adicionar diretórios de shaders
file(GLOB SHADERS
    "shaders/*.glsl"
)

add_executable(projeto-final
    ${SOURCES}
    ${SHADERS}
    main.cpp
)

INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} )

target_sources(projeto-final
  PRIVATE
    main.cpp
    main.cpp
)

target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} glm)
