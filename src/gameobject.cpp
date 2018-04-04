#include "gameobject.hpp"

GameObject::GameObject(void){};

GameObject::GameObject(GameObject* parent, Model* model, glm::vec3 pos,
                       glm::vec3 rot, glm::vec3 sca)
    : parent(parent) {
  _renderAttrib.model = model->attrib.model;
  _renderAttrib.shader_key = model->attrib.shader_key;
  _renderAttrib.vaos = model->attrib.vaos;
  _renderAttrib.bones = model->attrib.bones;
  _renderAttrib.state = model->attrib.state;
  if (model->skeleton != nullptr) {
    _skeleton = new Skeleton(*model->skeleton);
    _global_inverse = model->global_inverse;
  }

  transform.position = pos;
  transform.rotation = rot;
  transform.scale = sca;
}

GameObject::GameObject(GameObject const& src) { *this = src; }

GameObject::~GameObject(void) {
  /*if (this->inputComponent != nullptr) {
    delete this->inputComponent;
  }
  if (this->physicsComponent != nullptr) {
    delete this->physicsComponent;
  }*/
}

GameObject& GameObject::operator=(GameObject const& rhs) {
  if (this != &rhs) {
    /*
    this->_renderAttrib = rhs._renderAttrib;
    this->_renderAttrib.shader = rhs._renderAttrib.shader;
    this->_renderAttrib.vao = rhs._renderAttrib.vao;
    this->_renderAttrib.transforms = rhs._renderAttrib.transforms;
    this->_renderAttrib.texture = rhs._renderAttrib.texture;
    this->inputComponent =
        rhs.inputComponent ? new InputComponent(*rhs.inputComponent) : nullptr;
    this->physicsComponent = rhs.physicsComponent
                                 ? new PhysicsComponent(*rhs.physicsComponent)
                                 : nullptr;*/
    transform.position = rhs.transform.position;
    transform.rotation = rhs.transform.rotation;
    transform.scale = rhs.transform.scale;
    parent = rhs.parent;
  }
  return (*this);
}

void GameObject::update(const Env& env) {
  // animate(env.getAbsoluteTime());
  /*if (inputComponent != nullptr) {
    inputComponent->update(*this, world, inputHandler, this->physicsComponent);
  }
  if (physicsComponent != nullptr) {
    physicsComponent->update(*this, world);
  }*/
  // this->positionRelative = this->transform.position - oldPosition;
  _renderAttrib.model = getWorldTransform();
}

glm::mat4 GameObject::getWorldTransform() {
  glm::mat4 worldTransform = transform.toModelMatrix();
  if (this->parent != nullptr) {
    worldTransform = this->parent->getWorldTransform() * worldTransform;
  }
  return (worldTransform);
}

void GameObject::updateAnimation(float timestamp, AnimData* data) {
  if (_skeleton == nullptr) return;
  for (auto node_it : _skeleton->node_ids) {
    std::string node_name = node_it.first;
    unsigned short bone_index = node_it.second;
    _skeleton->local_poses[bone_index] = animate(data, node_name, timestamp);
  }

  _skeleton->local_to_global();
  _renderAttrib.bones.resize(_skeleton->joint_count);
  for (unsigned short i = 0; i < _skeleton->joint_count; i++) {
    _renderAttrib.bones[i] =
        _global_inverse * _skeleton->global_poses[i] * _skeleton->offsets[i];
  }
}

void GameObject::updateAnimDebug(const render::Renderer& renderer) {
  if (_skeleton == nullptr) return;
  _animAttrib.state.primitiveMode = render::PrimitiveMode::Lines;
  _animAttrib.state.depthTestFunc = render::DepthTestFunc::Always;
  _animAttrib.model = _renderAttrib.model;
  std::vector<glm::vec3> positions;
  for (unsigned short i = 0; i < _skeleton->joint_count; i++) {
    const unsigned short parent_joint = _skeleton->hierarchy[i];
    glm::mat4 bone_offset = _global_inverse * _skeleton->global_poses[i];
    glm::mat4 parent_bone_offset =
        _global_inverse * _skeleton->global_poses[parent_joint];

    glm::vec3 point_offset =
        glm::vec3(bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    glm::vec3 parent_point_offset =
        glm::vec3(parent_bone_offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    /* Experiment: draw bones as quad so we can control the thickness of lines
     * The quad is facing the camera, but the multiplication by the model matrix
     * in the shader give different thickness for each bones
     * FIXME
    glm::vec3 bone_dir = glm::normalize(parent_point_offset - point_offset);
    glm::vec3 bone_axis = glm::cross(
        bone_dir, glm::normalize(glm::vec3(-renderer.uniforms.view[2])));
    bone_axis =
        glm::vec3(glm::inverse(attrib.model) * glm::vec4(bone_axis, 1.0f));
    bone_axis = glm::normalize(bone_axis);
    bone_axis *= 0.01f;
    glm::vec3 p0 = point_offset - bone_axis;
    glm::vec3 p1 = point_offset + bone_axis;
    glm::vec3 p2 = parent_point_offset - bone_axis;
    glm::vec3 p3 = parent_point_offset + bone_axis;

    const std::vector<glm::vec3> quad = {{p0}, {p1}, {p2}, {p2}, {p3}, {p0}};
    positions.insert(positions.end(), quad.begin(), quad.end());*/

    positions.push_back(point_offset);
    positions.push_back(parent_point_offset);
  }
  if (_animAttrib.vaos.size() && _animAttrib.vaos[0] != nullptr) {
    _animAttrib.vaos[0]->update(positions);
  } else {
    _animAttrib.vaos.push_back(new VAO(positions));
  }
}

const render::Attrib GameObject::getRenderAttrib() const {
  return (this->_renderAttrib);
}

const render::Attrib GameObject::getDebugRenderAttrib() const {
  return (this->_animAttrib);
}

/*PhysicsComponent::PhysicsComponent(void) : speed(5.0f){};

PhysicsComponent::~PhysicsComponent(void){};

PhysicsComponent::PhysicsComponent(PhysicsComponent const& src) { *this = src; }

PhysicsComponent& PhysicsComponent::operator=(PhysicsComponent const& rhs) {
  if (this != &rhs) {
    this->velocity = rhs.velocity;
    this->speed = rhs.speed;
    this->has_collide = rhs.has_collide;
  }
  return (*this);
}

void PhysicsComponent::update(GameObject& gameObject, World& world) {
  has_collide = false;  // Reset every frame
  glm::vec3 backupPosition = gameObject.transform.position;
  this->speed += 0.5f * world.deltaTime;
  gameObject.transform.position.z += log(this->speed) * 7.0f * world.deltaTime;
  this->velocity.y -= 0.81f * 4.0f * world.deltaTime;
  gameObject.transform.position.y += velocity.y * world.deltaTime;

  gameObject.transform.position.y =
      glm::clamp(gameObject.transform.position.y, 0.0f, 5.0f);
  this->velocity.y = glm::clamp(this->velocity.y, -10.0f, 3.0f);

  glm::vec3 target = gameObject.transform.position;
  if (gameObject.inputComponent->targetRail == Rail::left) {
    target.x = 1.25f;
  } else if (gameObject.inputComponent->targetRail == Rail::center) {
    target.x = 0.0f;
  } else if (gameObject.inputComponent->targetRail == Rail::right) {
    target.x = -1.25f;
  }
  gameObject.transform.position +=
      (target - gameObject.transform.position) * world.deltaTime * 5.0f;
  if (world.collide(gameObject)) {
    this->has_collide = true;
  }
}

InputComponent::InputComponent(void) : targetRail(Rail::center){};

InputComponent::~InputComponent(void){};

InputComponent::InputComponent(InputComponent const& src) { *this = src; }

InputComponent& InputComponent::operator=(InputComponent const& rhs) {
  if (this != &rhs) {
    this->targetRail = rhs.targetRail;
  }
  return (*this);
}

void InputComponent::update(GameObject& gameObject, World& world,
                            InputHandler& inputHandler,
                            PhysicsComponent* physicsComponent) {
  if (physicsComponent != nullptr) {
    bool left = false;
    bool right = false;
    if (inputHandler.keys[GLFW_KEY_A] || inputHandler.keys[GLFW_KEY_LEFT]) {
      left = true;
      targetRail = Rail::left;
    }
    if (inputHandler.keys[GLFW_KEY_D] || inputHandler.keys[GLFW_KEY_RIGHT]) {
      right = true;
      targetRail = Rail::right;
    }
    if ((right && left) || (!right && !left)) {
      targetRail = Rail::center;
    }
    if (inputHandler.keys[GLFW_KEY_SPACE]) {
      if (gameObject.transform.position.y == 0.0f) {
        physicsComponent->velocity.y = 2.8f;
      }
    }
  }
}*/
