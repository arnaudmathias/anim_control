#pragma once
#include <array>
#include "model.hpp"
#include "renderer.hpp"
//#include "run.hpp"

class GameObject;
// class PhysicsComponent;
// class InputComponent;

struct Transform {
  glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

  glm::mat4 toModelMatrix() {
    glm::mat4 mat_translation = glm::translate(position);
    glm::mat4 mat_rotation =
        glm::eulerAngleYXZ(rotation.y, rotation.x, rotation.z);
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
             glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
             glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));
  GameObject(GameObject const& src);
  ~GameObject(void);
  GameObject& operator=(GameObject const& rhs);

  const render::Attrib getRenderAttrib() const;
  const render::Attrib getDebugRenderAttrib() const;
  void update(const Env& env);
  void setTransform(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));
  glm::mat4 getWorldTransform();
  glm::vec4 getWorldPosition();

  void setTexture(Texture* texture);
  Transform transform;
  GameObject* parent = nullptr;
  void updateAnimDebug(const render::Renderer& renderer);

  // InputComponent* inputComponent;
  // PhysicsComponent* physicsComponent;

 private:
  GameObject(void);
  void animate(float timestamp);
  render::Attrib _renderAttrib;

  Skeleton* _skeleton = nullptr;
  glm::mat4 _global_inverse;
  render::Attrib _animAttrib;
  std::unordered_map<std::string, unsigned short> node_ids;
};

/*class InputComponent {
 public:
  InputComponent(void);
  InputComponent(InputComponent const& src);
  virtual ~InputComponent(void);
  InputComponent& operator=(InputComponent const& rhs);
  void update(GameObject& gameObject, World& world, InputHandler& inputHandler,
              PhysicsComponent* physicsComponent);
  Rail targetRail;
};*/

/*class PhysicsComponent {
 public:
  PhysicsComponent(void);
  PhysicsComponent(PhysicsComponent const& src);
  virtual ~PhysicsComponent(void);
  PhysicsComponent& operator=(PhysicsComponent const& rhs);
  void update(GameObject& gameObject, World& world);

  glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
  float speed;
};*/
