#include "renderer.hpp"

namespace render {

Renderer::Renderer(int width, int height) : _width(width), _height(height) {
  _shader = new Shader("shaders/anim.vert", "shaders/anim.frag");
  _billboardShader =
      new Shader("shaders/billboard.vert", "shaders/billboard.frag");
}

Renderer::Renderer(Renderer const &src) { *this = src; }

Renderer::~Renderer(void) {
  delete _shader;
  delete _cubeMapVao;
  delete _cubeMapShader;
  delete _cubeMapTexture;
}

Renderer &Renderer::operator=(Renderer const &rhs) {
  if (this != &rhs) {
    this->_attribs = rhs._attribs;
  }
  return (*this);
}

void Renderer::renderText(float pos_x, float pos_y, float scale,
                          std::string text, glm::vec3 color) {
  RenderState backup_state = _state;

  switchPolygonMode(PolygonMode::Fill);
  switchDepthTestState(false);
  switchBlendingFunc(BlendFunc::OneMinusSrcAlpha);
  switchBlendingState(true);

  TextProperties prop = {};
  prop.color = color;
  _textRenderer.renderText(pos_x, pos_y, scale, text, prop);

  setState(backup_state);
}

void Renderer::renderUI(std::string filename, float pos_x, float pos_y,
                        float scale, bool centered) {
  RenderState backup_state = _state;

  switchPolygonMode(PolygonMode::Fill);
  switchDepthTestState(false);
  switchBlendingFunc(BlendFunc::OneMinusSrcAlpha);
  switchBlendingState(true);

  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->_width),
                                    0.0f, static_cast<float>(this->_height));
  _uiRenderer.renderUI(filename, pos_x, pos_y, scale, projection, centered);

  setState(backup_state);
}

void Renderer::addAttrib(const Attrib &attrib) {
  this->_attribs.push_back(attrib);
}

void Renderer::switchShader(GLuint shader_id, int &current_shader_id) {
  if (shader_id > 0 && shader_id != current_shader_id) {
    glUseProgram(shader_id);
    setUniform(glGetUniformLocation(shader_id, "P"), this->uniforms.proj);
    setUniform(glGetUniformLocation(shader_id, "V"), this->uniforms.view);
    current_shader_id = shader_id;
  }
}

void Renderer::updateUniforms(const Attrib &attrib, const int shader_id) {
  if (shader_id > 0 && attrib.vaos.size() > 0) {
    glm::mat4 mvp = uniforms.view_proj * attrib.model;
    setUniform(glGetUniformLocation(shader_id, "MVP"), mvp);
    setUniform(glGetUniformLocation(shader_id, "M"), attrib.model);
    setUniform(glGetUniformLocation(shader_id, "B"), attrib.bones);
  }
}

void Renderer::draw() {
  _shader->reload();
  RenderState backup_state = _state;
  // std::sort(_renderAttribs.begin(), _renderAttribs.end());
  int shader_id = -1;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, _width, _height);
  switchDepthTestState(true);
  for (const auto &attrib : this->_attribs) {
    setState(attrib.state);
    if (attrib.shader == nullptr) {
      switchShader(_shader->id, shader_id);
      updateUniforms(attrib, _shader->id);
    } else {
      switchShader(attrib.shader->id, shader_id);
      updateUniforms(attrib, attrib.shader->id);
    }
    GLenum mode = getGLRenderMode(attrib.state.primitiveMode);
    for (const auto &vao : attrib.vaos) {
      if (vao != nullptr) {
        if (vao->indices_size != 0) {
          glBindVertexArray(vao->vao);
          glDrawElements(mode, vao->indices_size, GL_UNSIGNED_INT, 0);
        } else if (vao->vertices_size != 0) {
          glBindVertexArray(vao->vao);
          glDrawArrays(mode, 0, vao->vertices_size);
        }
      }
    }
  }

  if (this->_cubeMapVao != nullptr) {
    switchDepthTestFunc(DepthTestFunc::LessEqual);
    switchShader(this->_cubeMapShader->id, shader_id);
    setUniform(glGetUniformLocation(shader_id, "P"), this->uniforms.proj);
    setUniform(glGetUniformLocation(shader_id, "V"),
               glm::mat4(glm::mat3(this->uniforms.view)));
    setUniform(glGetUniformLocation(shader_id, "skybox"), 0);
    glBindVertexArray(this->_cubeMapVao->vao);
    if (this->_cubeMapTexture) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, this->_cubeMapTexture->id);
    }
    glDrawArrays(GL_TRIANGLES, 0, 36);
    switchDepthTestFunc(DepthTestFunc::Less);
  }
  setState(_state);
  glBindVertexArray(0);
}

void Renderer::update(const Env &env) {
  if (env.width != _width || env.height != _height) {
    _width = env.width;
    _height = env.height;
  }
  _textRenderer.update(env);
}

void Renderer::flushAttribs() { this->_attribs.clear(); }

int Renderer::getScreenWidth() { return (this->_width); }

int Renderer::getScreenHeight() { return (this->_height); }

void Renderer::loadCubeMap(std::string vertex_sha, std::string fragment_sha,
                           const std::vector<std::string> &textures) {
  std::vector<glm::vec3> skyboxVertices = {
      {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {-1.0f, 1.0f, -1.0f},
      {-1.0f, -1.0f, 1.0f},  {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
      {-1.0f, 1.0f, -1.0f},  {-1.0f, 1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},
      {1.0f, -1.0f, -1.0f},  {1.0f, -1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {1.0f, 1.0f, -1.0f},   {1.0f, -1.0f, -1.0f},
      {-1.0f, -1.0f, 1.0f},  {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {1.0f, -1.0f, 1.0f},   {-1.0f, -1.0f, 1.0f},
      {-1.0f, 1.0f, -1.0f},  {1.0f, 1.0f, -1.0f},   {1.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, -1.0f},
      {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f},
      {1.0f, -1.0f, -1.0f},  {-1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, 1.0f}};
  try {
    this->_cubeMapTexture = new Texture(textures);
  } catch (std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
  }
  this->_cubeMapShader = new Shader(vertex_sha, fragment_sha);
  this->_cubeMapVao = new VAO(skyboxVertices);
}

void Renderer::clearScreen() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLenum Renderer::getGLRenderMode(enum PrimitiveMode mode) {
  GLenum gl_primitive_modes[12] = {GL_POINTS,
                                   GL_LINE_STRIP,
                                   GL_LINE_LOOP,
                                   GL_LINES,
                                   GL_LINE_STRIP_ADJACENCY,
                                   GL_LINES_ADJACENCY,
                                   GL_TRIANGLE_STRIP,
                                   GL_TRIANGLE_FAN,
                                   GL_TRIANGLES,
                                   GL_TRIANGLE_STRIP_ADJACENCY,
                                   GL_TRIANGLES_ADJACENCY,
                                   GL_PATCHES};
  unsigned index_mode = static_cast<unsigned int>(mode);
  return (gl_primitive_modes[index_mode]);
}

void Renderer::setState(const RenderState &new_state) {
  switchPolygonMode(new_state.polygonMode);
  switchDepthTestFunc(new_state.depthTestFunc);
  switchDepthTestState(new_state.depthTest);
  switchBlendingState(new_state.blending);
  switchBlendingFunc(new_state.blendFunc);
}

void Renderer::switchPolygonMode(enum PolygonMode mode) {
  GLenum gl_polygon_modes[3] = {GL_POINT, GL_LINE, GL_FILL};
  if (mode != _state.polygonMode) {
    unsigned int index_mode = static_cast<unsigned int>(mode);
    glPolygonMode(GL_FRONT_AND_BACK, gl_polygon_modes[index_mode]);
    _state.polygonMode = mode;
  }
}

void Renderer::switchDepthTestFunc(enum DepthTestFunc mode) {
  GLenum gl_depth_funcs[8] = {GL_LESS,    GL_NEVER,    GL_EQUAL,  GL_LEQUAL,
                              GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};
  if (mode != _state.depthTestFunc) {
    unsigned int index_func = static_cast<unsigned int>(mode);
    glDepthFunc(gl_depth_funcs[index_func]);
    _state.depthTestFunc = mode;
  }
}

void Renderer::switchBlendingFunc(enum BlendFunc mode) {
  GLenum gl_blend_funcs[14] = {GL_ZERO,           GL_ONE,
                               GL_SRC_COLOR,      GL_ONE_MINUS_SRC_COLOR,
                               GL_DST_COLOR,      GL_ONE_MINUS_DST_COLOR,
                               GL_SRC_ALPHA,      GL_ONE_MINUS_SRC_ALPHA,
                               GL_DST_ALPHA,      GL_ONE_MINUS_DST_ALPHA,
                               GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR,
                               GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA};
  if (mode != _state.blendFunc) {
    unsigned int index_func = static_cast<unsigned int>(mode);
    glBlendFunc(GL_ONE, gl_blend_funcs[index_func]);
    _state.blendFunc = mode;
  }
}

void Renderer::switchDepthTestState(bool depth_test) {
  if (depth_test != _state.depthTest) {
    if (depth_test) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
    _state.depthTest = depth_test;
  }
}

void Renderer::switchBlendingState(bool blending) {
  if (blending != _state.blending) {
    if (blending) {
      glEnable(GL_BLEND);
    } else {
      glDisable(GL_BLEND);
    }
    _state.blending = blending;
  }
}

bool Attrib::operator<(const struct Attrib &rhs) const {
  if (this->vaos[0] != nullptr && rhs.vaos[0] != nullptr) {
    return (this->state.depthTest == false);
  } else {
    return (true);
  }
}

UiRenderer::UiRenderer(void) {
  Shader shader("shaders/ui.vert", "shaders/ui.frag");
  this->_shader_id = shader.id;

  glGenVertexArrays(1, &this->_vao);
  glGenBuffers(1, &this->_vbo);

  glBindVertexArray(this->_vao);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

UiRenderer::UiRenderer(UiRenderer const &src) { *this = src; }

UiRenderer::~UiRenderer(void) {
  auto it = _texture_cache.begin();
  while (it != _texture_cache.end()) {
    delete it->second;
    it++;
  }
}

UiRenderer &UiRenderer::operator=(UiRenderer const &rhs) {
  if (this != &rhs) {
  }
  return (*this);
}

void UiRenderer::renderUI(std::string texture_name, float pos_x, float pos_y,
                          float scale, glm::mat4 ortho, bool centered) {
  Texture *texture = nullptr;
  auto it = _texture_cache.find(texture_name);
  if (it == _texture_cache.end()) {
    try {
      texture = new Texture(texture_name);
      _texture_cache.emplace(texture_name, texture);
    } catch (std::runtime_error &e) {
      std::cerr << e.what() << std::endl;
    }
  } else {
    texture = it->second;
  }
  if (texture == nullptr) return;
  glUseProgram(_shader_id);
  glUniformMatrix4fv(glGetUniformLocation(_shader_id, "proj"), 1, GL_FALSE,
                     glm::value_ptr(ortho));
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(this->_vao);
  GLfloat w = texture->width * scale;
  GLfloat h = texture->height * scale;

  GLfloat xpos = pos_x;
  GLfloat ypos = pos_y;

  if (centered) {
    xpos -= w / 2;
    ypos -= h / 2;
  }
  pos_x *= scale;
  pos_y *= scale;
  GLfloat vertices[6][4] = {
      {xpos, ypos + h, 0.0, 0.0},    {xpos, ypos, 0.0, 1.0},
      {xpos + w, ypos, 1.0, 1.0},

      {xpos, ypos + h, 0.0, 0.0},    {xpos + w, ypos, 1.0, 1.0},
      {xpos + w, ypos + h, 1.0, 0.0}};
  glBindTexture(GL_TEXTURE_2D, texture->id);
  glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glBindVertexArray(0);
}
}  // namespace render
