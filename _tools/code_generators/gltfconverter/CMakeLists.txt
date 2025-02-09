cmake_minimum_required(VERSION 2.6)
project(GLTFCONVERTER)

include_directories(../libs/tinygltf)
add_executable(gltfconverter main.cpp geometry.cpp gltf_helper.cpp ../libs/tinygltf/tiny_gltf.cc)
