#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define NUM_BONES_PER_VERTEX 4
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

struct VertexBoneData {
  unsigned int ids[NUM_BONES_PER_VERTEX];
  float weights[NUM_BONES_PER_VERTEX];
};

struct Vertex {
  glm::vec4 position = {0, 0, 0, 0};
  Vertex() : position({0.0f, 0.0f, 0.0f, 0.0f}){};
  Vertex(Vertex const& src) { *this = src; }
  Vertex& operator=(Vertex const& rhs) {
    if (this != &rhs) {
      this->position = rhs.position;
    }
    return (*this);
  };
};
