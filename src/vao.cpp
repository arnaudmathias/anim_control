#include "vao.hpp"

VAO::VAO(const std::vector<Vertex> &vertices) {
  this->_vbo = 0;
  this->vao = 0;
  this->vertices_size = vertices.size();
  this->indices_size = 0;
  glGenBuffers(1, &this->_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices_size * sizeof(Vertex),
               vertices.data(), GL_DYNAMIC_DRAW);

  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)offsetof(Vertex, position));
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)offsetof(Vertex, attribs));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

VAO::VAO(const std::vector<glm::vec3> &positions) {
  this->_vbo = 0;
  this->vao = 0;
  this->vertices_size = positions.size();
  this->indices_size = 0;

  glGenBuffers(1, &this->_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices_size * sizeof(glm::vec3),
               positions.data(), GL_DYNAMIC_DRAW);

  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (GLvoid *)0);

  glEnableVertexAttribArray(0);
}

VAO::VAO(const std::vector<glm::vec4> &positions) {
  this->_vbo = 0;
  this->vao = 0;
  this->vertices_size = positions.size();
  this->indices_size = 0;

  glGenBuffers(1, &this->_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices_size * sizeof(glm::vec4),
               positions.data(), GL_DYNAMIC_DRAW);

  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                        (GLvoid *)0);

  glEnableVertexAttribArray(0);
}

void VAO::update(const std::vector<Vertex> &vertices) {
  this->vertices_size = vertices.size();
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_DYNAMIC_DRAW);
}

void VAO::update(const std::vector<glm::vec3> &positions) {
  this->vertices_size = positions.size();
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3),
               positions.data(), GL_DYNAMIC_DRAW);
}

VAO::~VAO() {
  if (this->_vbo != 0) glDeleteBuffers(1, &this->_vbo);
  if (this->vao != 0) glDeleteVertexArrays(1, &this->vao);
}
