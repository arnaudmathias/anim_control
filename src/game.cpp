#include "game.hpp"

Game::Game(void) {
  _camera =
      new Camera(glm::vec3(0.0f, 5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f));
  MeshLoader loader;
  _models.push_back(loader.loadScene("anims/Idle.dae"));
  _models.push_back(loader.loadScene("anims/Walking.dae"));
  std::vector<glm::vec3> floor_plan = {{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f},
                                       {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                                       {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
  _models.push_back(loader.loadModel(floor_plan));

  for (auto& model : _models) {
    for (auto it = model->animations.begin(); it != model->animations.end();
         it++) {
      _animations.emplace(it->first, it->second);
    }
    model->animations.clear();
  }

  _entities.push_back(new GameObject(nullptr, _models[0], &_animations));

  _player_handle = _entities[0];
  _player_handle->setAsContralable();
  _entities.push_back(new GameObject(
      nullptr, _models[2], &_animations, glm::vec3(-25.0f, 0.0f, -25.0f),
      glm::vec3(0.0f), glm::vec3(50.0f, 1.0f, 50.0f)));
  // Extract animations data from models
}

Game::Game(Game const& src) { *this = src; }

Game::~Game(void) {
  delete _camera;
  for (auto& model : _models) {
    delete model;
  }
  for (auto& go : _entities) {
    delete go;
  }
}

Game& Game::operator=(Game const& rhs) {
  if (this != &rhs) {
    this->_debugMode = rhs._debugMode;
    this->_camera = new Camera(*rhs._camera);
  }
  return (*this);
}

void Game::update(Env& env) {
  _camera->update(env, env.getDeltaTime());
  if (env.inputHandler.keys[GLFW_KEY_I]) {
    env.inputHandler.keys[GLFW_KEY_I] = false;
    _debugMode = !_debugMode;
  }
  for (auto& entity : _entities) {
    entity->update(env);
  }
}

void Game::render(const Env& env, render::Renderer& renderer) {
  float fwidth = static_cast<float>(renderer.getScreenWidth());
  float fheight = static_cast<float>(renderer.getScreenHeight());
  renderer.uniforms.view = _camera->view;
  renderer.uniforms.proj = _camera->proj;
  renderer.uniforms.view_proj = _camera->proj * _camera->view;

  for (auto& entity : _entities) {
    renderer.addAttrib(entity->getRenderAttrib());
    if (_debugMode) {
      renderer.addAttrib(entity->getDebugRenderAttrib());
    }
  }
  renderer.draw();

  renderer.flushAttribs();
  if (_debugMode) {
    print_debug_info(env, renderer, *_camera);
  }
}

std::string float_to_string(float f, int prec) {
  std::ostringstream out;
  out << std::setprecision(prec) << std::fixed << f;
  return out.str();
}

void Game::print_debug_info(const Env& env, render::Renderer& renderer,
                            Camera& camera) {
  float fheight = static_cast<float>(renderer.getScreenHeight());
  float fwidth = static_cast<float>(renderer.getScreenWidth());
  renderer.renderText(10.0f, fheight - 25.0f, 0.35f,
                      "x: " + float_to_string(camera.pos.x, 2) +
                          " y: " + float_to_string(camera.pos.y, 2) +
                          " z: " + float_to_string(camera.pos.z, 2),
                      glm::vec3(1.0f, 1.0f, 1.0f));
}
