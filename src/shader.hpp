#pragma once
#include <sys/stat.h>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
#include "env.hpp"

const int shader_types[3] = {GL_VERTEX_SHADER, GL_GEOMETRY_SHADER,
                             GL_FRAGMENT_SHADER};

const std::string shader_extensions[3] = {".vert", ".geom", ".frag"};

struct ShaderFile {
  std::string filename;
  std::time_t last_modification;
};

class Shader {
 public:
  Shader(std::string shader, std::string = "", std::string = "");
  Shader(Shader const &src);
  ~Shader(void);
  Shader &operator=(Shader const &rhs);

  GLuint id;
  void use() const;
  void reload();

 private:
  Shader(void);
  GLuint loadShader(std::string shader);
  GLuint loadVertex(std::string filename);
  GLuint loadFragment(std::string filename);
  GLuint compileShader(const std::string source, std::string filename,
                       GLuint shaderType);
  GLuint linkShaders(const std::array<GLuint, 3> shader_ids);
  GLuint linkShaders(GLuint vertexID, GLuint fragID);
  const std::string getShaderSource(std::string filename);
  ShaderFile _shaders[3];
};

void printShaderError(GLuint shade, std::string filename);
void printLinkError(GLuint program);
std::time_t getLastModificationTime(std::string filename);
