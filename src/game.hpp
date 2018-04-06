#pragma once
#include <iomanip>
#include "anim.hpp"
#include "camera.hpp"
#include "gameobject.hpp"
#include "loader.hpp"
#include "model.hpp"
#include "renderer.hpp"

class Game {
 public:
  Game(void);
  Game(Game const& src);
  ~Game(void);
  Game& operator=(Game const& rhs);
  void update(Env& env);
  void render(const Env& env, render::Renderer& renderer);

 private:
  bool _debugMode = false;
  Camera* _camera;
  std::vector<Model*> _models;
  std::vector<GameObject*> _entities;
  std::map<std::string, AnimData> _animations;
  GameObject* _player_handle;  // Only an handle, DO NOT DELETE

  void print_debug_info(const Env& env, render::Renderer& renderer,
                        Camera& camera);
};
