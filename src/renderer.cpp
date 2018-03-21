#include "renderer.hpp"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

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

std::vector<glm::vec4> billboardVertices = {  // xy=pos, zw = uv
    {-1.0f, 1.0f, 0.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, -1.0f, 1.0f, 0.0f},  {1.0f, -1.0f, 1.0f, 0.0f},
    {-1.0f, -1.0f, 0.0f, 0.0f}, {-1.0f, 1.0f, 0.0f, 1.0f}};

Renderer::Renderer(void) : Renderer(0, 0) {}

Renderer::Renderer(int width, int height)
    : _width(width),
      _height(height),
      _cubeMapTexture(nullptr),
      _cubeMapVao(nullptr),
      _cubeMapShader(nullptr),
      _polygonMode(PolygonMode::Fill) {
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
    this->_renderAttribs = rhs._renderAttribs;
  }
  return (*this);
}

void Renderer::renderText(float pos_x, float pos_y, float scale,
                          std::string text, glm::vec3 color) {
  enum PolygonMode backup_mode = _polygonMode;
  switchPolygonMode(PolygonMode::Fill);
  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->_width),
                                    0.0f, static_cast<float>(this->_height));
  _textRenderer.renderText(pos_x, pos_y, scale, text, color, projection);
  switchPolygonMode(backup_mode);
}

void Renderer::renderUI(std::string filename, float pos_x, float pos_y,
                        float scale, bool centered) {
  enum PolygonMode backup_mode = _polygonMode;
  switchPolygonMode(PolygonMode::Fill);
  glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->_width),
                                    0.0f, static_cast<float>(this->_height));
  _uiRenderer.renderUI(filename, pos_x, pos_y, scale, projection, centered);
  switchPolygonMode(backup_mode);
}

void Renderer::renderbillboard(const std::vector<glm::vec3> vertices,
                               glm::mat4 model, glm::mat4 view_proj) {
  VAO vao(vertices);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(_billboardShader->id);
  glm::mat4 mvp = view_proj * model;
  glUniformMatrix4fv(glGetUniformLocation(_billboardShader->id, "MVP"), 1,
                     GL_FALSE, glm::value_ptr(mvp));
  glBindVertexArray(vao.vao);
  glDrawArrays(GL_TRIANGLES, 0, vao.vertices_size);

  glBindVertexArray(0);
  glDisable(GL_BLEND);
}

void Renderer::addRenderAttrib(const RenderAttrib &renderAttrib) {
  this->_renderAttribs.push_back(renderAttrib);
}

void Renderer::switchShader(GLuint shader_id, int &current_shader_id) {
  if (shader_id > 0 && shader_id != current_shader_id) {
    glUseProgram(shader_id);
    setUniform(glGetUniformLocation(shader_id, "P"), this->uniforms.proj);
    setUniform(glGetUniformLocation(shader_id, "V"), this->uniforms.view);
    setUniform(glGetUniformLocation(shader_id, "texture_array"), 0);
    current_shader_id = shader_id;
  }
}

void Renderer::updateUniforms(const RenderAttrib &attrib, const int shader_id) {
  if (shader_id > 0 && attrib.vaos.size() > 0) {
    glm::mat4 mvp = uniforms.view_proj * attrib.model;
    setUniform(glGetUniformLocation(shader_id, "MVP"), mvp);
    setUniform(glGetUniformLocation(shader_id, "M"), attrib.model);
  }
}

void Renderer::draw() {
  _shader->reload();
  // std::sort(_renderAttribs.begin(), _renderAttribs.end());
  int shader_id = -1;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, _width, _height);
  switchShader(_shader->id, shader_id);
  for (const auto &attrib : this->_renderAttribs) {
    updateUniforms(attrib, _shader->id);
    for (const auto &vao : attrib.vaos) {
      if (vao != nullptr && vao->vertices_size > 0) {
        glBindVertexArray(vao->vao);
        glDrawArrays(GL_TRIANGLES, 0, vao->vertices_size);
      }
    }
  }
  if (this->_cubeMapVao != nullptr) {
    glDepthFunc(GL_LEQUAL);
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
    glDepthFunc(GL_LESS);
  }
  glBindVertexArray(0);
}

void Renderer::update(const Env &env) {
  if (env.width != _width || env.height != _height) {
    _width = env.width;
    _height = env.height;
  }
}

void Renderer::flush() { this->_renderAttribs.clear(); }

void Renderer::reset() { this->_renderAttribs.clear(); }

int Renderer::getScreenWidth() { return (this->_width); }

int Renderer::getScreenHeight() { return (this->_height); }

void Renderer::loadCubeMap(std::string vertex_sha, std::string fragment_sha,
                           const std::vector<std::string> &textures) {
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

void Renderer::switchPolygonMode(enum PolygonMode mode) {
  if (mode != _polygonMode) {
    switch (mode) {
      case PolygonMode::Point:
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
      case PolygonMode::Line:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
      case PolygonMode::Fill:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
      default:
        break;
    }
    _polygonMode = mode;
  }
}

bool RenderAttrib::operator<(const struct RenderAttrib &rhs) const {
  if (this->vaos[0] != nullptr && rhs.vaos[0] != nullptr) {
    return (this->vaos[0]->vao < rhs.vaos[0]->vao);
  } else {
    return (true);
  }
}

TextRenderer::TextRenderer(void) {
  Shader shader("shaders/text.vert", "shaders/text.frag");
  this->_shader_id = shader.id;

  unsigned int filesize = io::get_filesize("fonts/minecraft.ttf");
  unsigned char *ttf_buffer = new unsigned char[filesize + 1];
  FILE *font_fp = fopen("fonts/minecraft.ttf", "rb");
  if (font_fp == nullptr) {
    return;
  }

  fread(ttf_buffer, filesize + 1, 1, font_fp);
  int font_offset = stbtt_GetFontOffsetForIndex(ttf_buffer, 0);
  if (font_offset == -1) {
    std::cout << "wrong font offset" << std::endl;
    return;
  }
  stbtt_fontinfo font;
  if (stbtt_InitFont(&font, ttf_buffer, font_offset) == 0) {
    std::cout << "Failed to load font" << std::endl;
    return;
  }
  float font_scale = stbtt_ScaleForPixelHeight(&font, 48.0f);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (GLubyte c = 0; c < 128; c++) {
    int w, h, xoff, yoff;
    unsigned char *bitmap =
        stbtt_GetCodepointBitmap(&font, 0, font_scale, c, &w, &h, &xoff, &yoff);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE,
                 bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int advance_width;
    stbtt_GetCodepointHMetrics(&font, c, &advance_width, 0);
    Character character = {texture, glm::ivec2(w, h), glm::ivec2(xoff, -yoff),
                           static_cast<GLuint>(advance_width * font_scale)};
    this->_characters.insert(std::pair<GLchar, Character>(c, character));
    stbtt_FreeBitmap(bitmap, NULL);
  }

  fclose(font_fp);
  delete[] ttf_buffer;

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

TextRenderer::~TextRenderer() {
  for (auto &character : this->_characters) {
    glDeleteTextures(1, &character.second.textureID);
  }
}

void TextRenderer::renderText(float pos_x, float pos_y, float scale,
                              std::string text, glm::vec3 color,
                              glm::mat4 ortho) {
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(_shader_id);
  glUniformMatrix4fv(glGetUniformLocation(_shader_id, "proj"), 1, GL_FALSE,
                     glm::value_ptr(ortho));
  glUniform3fv(glGetUniformLocation(_shader_id, "text_color"), 1,
               glm::value_ptr(color));
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(this->_vao);
  for (char &c : text) {
    if (static_cast<int>(c) > 0 && static_cast<int>(c) < 128) {
      Character ch = this->_characters[static_cast<int>(c)];
      GLfloat xpos = pos_x + ch.bearing.x * scale;
      GLfloat ypos = pos_y - (ch.size.y - ch.bearing.y) * scale;

      GLfloat w = ch.size.x * scale;
      GLfloat h = ch.size.y * scale;
      GLfloat vertices[6][4] = {
          {xpos, ypos + h, 0.0, 0.0},    {xpos, ypos, 0.0, 1.0},
          {xpos + w, ypos, 1.0, 1.0},

          {xpos, ypos + h, 0.0, 0.0},    {xpos + w, ypos, 1.0, 1.0},
          {xpos + w, ypos + h, 1.0, 0.0}};
      glBindTexture(GL_TEXTURE_2D, ch.textureID);
      glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      pos_x += ch.advanceOffset * scale;
    }
  }
  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
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
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
  glEnable(GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
}
