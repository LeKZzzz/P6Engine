cmake_minimum_required(VERSION 3.25)
project(P6Engine)

set(CMAKE_CXX_STANDARD 11)

add_executable(P6Engine types.h PWorld.h Camera.h CameraManager.h EventManager.h PObjectManager.h Anchor.h PUI.h Scene.h PObject.h utils.h CollisionSystem.h Particle.h demo3.cpp Basic.h demo3.cpp)
