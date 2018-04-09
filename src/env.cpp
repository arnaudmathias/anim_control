#include "env.hpp"

Env::Env() : Env(0, 0) {}

Env::Env(unsigned short width, unsigned short height)
    : width(width), height(height) {
  if (!glfwInit()) return;
  setupWindowHint();
  GLFWmonitor *primary_monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(primary_monitor);
  if (width == 0 && height == 0) {
    this->width = mode->width;
    this->height = mode->height;
    window = glfwCreateWindow(this->width, this->height, "anim_control",
                              primary_monitor, NULL);
    glfwSetWindowMonitor(window, primary_monitor, 0, 0, mode->width,
                         mode->height, mode->refreshRate);
  } else {
    _windowed_width = width;
    _windowed_height = height;
    window = glfwCreateWindow(width, height, "anim_control", NULL, NULL);
    glfwSetWindowMonitor(window, NULL,
                         (mode->width / 2) - (_windowed_width / 2),
                         (mode->height / 2) - (_windowed_height / 2),
                         _windowed_width, _windowed_height, 0);
    inputHandler.mousex = _windowed_width / 2;
    inputHandler.mousey = _windowed_height / 2;
  }
  if (!window) {
    std::cout << "Could not create window\n";
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize OpenGL context" << std::endl;
  }
  int max_uniform_vec = 0;
  glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &max_uniform_vec);
  std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
  std::cout << "GLSL " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
  std::cout << "Max uniform vec: " << max_uniform_vec << std::endl;

  inputHandler.mouseDisabled = false;

  setupWindow();
  setupContext();
}

Env::~Env() { glfwTerminate(); }

void Env::toggleFullscreen() {
  _fullscreen = !_fullscreen;
  GLFWmonitor *primary_monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(primary_monitor);
  if (_fullscreen) {
    glfwSetWindowMonitor(window, primary_monitor, 0, 0, mode->width,
                         mode->height, mode->refreshRate);
    this->width = mode->width;
    this->height = mode->height;
    // The mouse virtual position reported by curposPosCallback promptly jump
    // after a window -> fullscreen transition
    // Doesn't happen on fullscreen -> window
    // GLFW bug ?
    inputHandler.mousex -= (this->_windowed_width / 2);
    inputHandler.mousey -= (this->_windowed_height / 2);
    this->has_resized = true;
  } else {
    glfwSetWindowMonitor(window, NULL,
                         (mode->width / 2) - (_windowed_width / 2),
                         (mode->height / 2) - (_windowed_height / 2),
                         _windowed_width, _windowed_height, 0);
    this->width = _windowed_width;
    this->height = _windowed_height;
  }
  // Query and update framebuffer size
  int wframe, hframe;
  glfwGetFramebufferSize(window, &wframe, &hframe);
  glViewport(0, 0, wframe, hframe);
}

void Env::setupWindowHint() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
}

void Env::setupWindow() {
  if (window != nullptr) {
    glfwSetWindowUserPointer(window, &inputHandler);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseKeyCallback);
  }
}

void Env::setupContext() {
  glfwSwapInterval(0);
  glEnable(GL_DEBUG_OUTPUT);
  while (glGetError() != GL_NO_ERROR)
    ;  // Flush gl_error
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Env::update() {
  updateFpsCounter();
  static double previousTime = glfwGetTime();
  static int frame_count;
  double currentTime = glfwGetTime();
  double deltaTime = (currentTime - previousTime) * _time_modifier;
  previousTime = currentTime;
  this->_deltaTime = static_cast<float>(deltaTime);
  this->_absoluteTime += this->_deltaTime;
  this->_frame++;
  if (inputHandler.keys[GLFW_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(window, 1);
  }
  if (inputHandler.keys[GLFW_KEY_F]) {
    inputHandler.keys[GLFW_KEY_F] = false;
    toggleFullscreen();
  }
  if (inputHandler.keys[GLFW_KEY_COMMA]) {
    inputHandler.keys[GLFW_KEY_COMMA] = false;
    _time_modifier -= 0.1f;
  }
  if (inputHandler.keys[GLFW_KEY_PERIOD]) {
    inputHandler.keys[GLFW_KEY_PERIOD] = false;
    _time_modifier += 0.1f;
  }
  _time_modifier = _time_modifier < 0.0f ? 0.0f : _time_modifier;
  _time_modifier = _time_modifier > 2.0f ? 2.0f : _time_modifier;
}

void Env::updateFpsCounter() {
  static double previous_seconds = glfwGetTime();
  static int frame_count;
  double current_seconds = glfwGetTime();
  double elapsed_seconds = current_seconds - previous_seconds;
  if (elapsed_seconds > 1.0) {
    previous_seconds = current_seconds;
    this->_fps =
        static_cast<float>(frame_count) / static_cast<float>(elapsed_seconds);
    std::ostringstream tmp;
    tmp << _fps << " fps";
    glfwSetWindowTitle(window, tmp.str().c_str());
    frame_count = 0;
  }
  frame_count++;
}

float Env::getDeltaTime() const { return (this->_deltaTime); }

float Env::getAbsoluteTime() const { return (this->_absoluteTime); }

float Env::getFrame() const { return (this->_frame); }

float Env::getFPS() const { return (this->_fps); }

float Env::getTimeModifier() const { return (this->_time_modifier); }

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mode) {
  (void)scancode;
  (void)mode;
  InputHandler *inputHandler =
      reinterpret_cast<InputHandler *>(glfwGetWindowUserPointer(window));
  if (action == GLFW_PRESS) {
    inputHandler->keys[key] = true;
  } else if (action == GLFW_RELEASE) {
    inputHandler->keys[key] = false;
  }
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  InputHandler *inputHandler =
      reinterpret_cast<InputHandler *>(glfwGetWindowUserPointer(window));
  inputHandler->mousex = static_cast<float>(xpos);
  inputHandler->mousey = static_cast<float>(ypos);
}

void mouseKeyCallback(GLFWwindow *window, int button, int action, int mods) {
  (void)mods;
  InputHandler *inputHandler =
      reinterpret_cast<InputHandler *>(glfwGetWindowUserPointer(window));
  if (action == GLFW_PRESS) {
    inputHandler->mouse_keys[button] = true;
  } else if (action == GLFW_RELEASE) {
    inputHandler->mouse_keys[button] = false;
  }
}
