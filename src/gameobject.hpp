#pragma once
#include <array>
#include <queue>
#include "model.hpp"
#include "renderer.hpp"

class GameObject;
class AnimationComponent;
class InputComponent;
class PhysicsComponent;

enum class InputDirection { North, East, South, West, None };

struct Transform {
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::quat rotation = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

  glm::mat4 toModelMatrix() {
    glm::mat4 mat_translation = glm::translate(position);
    glm::mat4 mat_rotation = glm::mat4_cast(rotation);
    /*glm::mat4 mat_rotation =
        glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);*/
    glm::mat4 mat_scale = glm::scale(scale);
    _model = mat_translation * mat_rotation * mat_scale;
    return (_model);
  }

 private:
  glm::mat4 _model = glm::mat4(1.0f);
};

class GameObject {
 public:
  GameObject(GameObject* parent, Model* model,
             glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3 vec3 = glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));
  GameObject(GameObject const& src);
  ~GameObject(void);
  GameObject& operator=(GameObject const& rhs);

  const render::Attrib getRenderAttrib() const;
  const render::Attrib getDebugRenderAttrib() const;
  void update(Env& env, AnimData* data);
  glm::mat4 getWorldTransform();

  void setAsContralable();

  Transform transform;
  GameObject* parent = nullptr;

  std::unique_ptr<InputComponent> inputComponent;
  std::unique_ptr<PhysicsComponent> physicsComponent;
  std::unique_ptr<AnimationComponent> animationComponent;

 private:
  GameObject(void);
  render::Attrib _renderAttrib;

  Skeleton* _skeleton = nullptr;
  render::Attrib _animAttrib;
};

class AnimationComponent {
 public:
  AnimationComponent(void);
  AnimationComponent(AnimationComponent const& src);
  ~AnimationComponent(void);
  AnimationComponent& operator=(AnimationComponent const& rhs);
  void updateBones(float timestamp, std::vector<glm::mat4>& bones,
                   AnimData* data);
  void updateAnimDebugAttrib(glm::mat4 parent_model);

  Skeleton skeleton;
  render::Attrib skel_attrib;
};

class InputComponent {
 public:
  InputComponent(void);
  InputComponent(InputComponent const& src);
  ~InputComponent(void);
  InputComponent& operator=(InputComponent const& rhs);
  void update(GameObject& gameObject, InputHandler& inputHandler);

  std::queue<InputDirection> directions;
};

class PhysicsComponent {
 public:
  enum class State { Idling, Rotating, Moving };
  PhysicsComponent(void);
  PhysicsComponent(PhysicsComponent const& src);
  ~PhysicsComponent(void);
  PhysicsComponent& operator=(PhysicsComponent const& rhs);
  void update(GameObject& gameObject, float dt);

  glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 target_position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::quat start_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::quat target_rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  float rotation_factor = 0.0f;
  float speed = 1.0f;
  enum State state = State::Idling;
};
