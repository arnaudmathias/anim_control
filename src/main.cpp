#include <iomanip>
#include <list>
#include "env.hpp"
#include "game.hpp"
#include "loader.hpp"
#include "renderer.hpp"

int main(int argc, char **argv) {
  Env env(1280, 720);
  Renderer renderer(env.width, env.height);
  renderer.loadCubeMap(
      "shaders/skybox.vert", "shaders/skybox.frag",
      {"textures/skybox_side.png", "textures/skybox_side.png",
       "textures/skybox_up.png", "textures/skybox_bottom.png",
       "textures/skybox_side.png", "textures/skybox_side.png"});
  MeshLoader loader;
  GL_DUMP_ERROR("renderer init");
  Model *model = loader.loadScene("anims/Walking.dae");
  Model *model2 = loader.loadScene("anims/Jumping.dae");
  Game game;
  bool wireframe = false;
  while (!glfwWindowShouldClose(env.window)) {
    env.update();
    glfwPollEvents();
    game.update(env);
    renderer.update(env);
    renderer.clearScreen();
    model->update(env.getAbsoluteTime());
    model2->update(env.getAbsoluteTime());
    model->pushRenderAttribs(renderer);
    model2->pushRenderAttribs(renderer);
    game.render(env, renderer);
    glfwSwapBuffers(env.window);
    GL_DUMP_ERROR("draw loop");
    if (env.inputHandler.keys[GLFW_KEY_ESCAPE]) {
      glfwSetWindowShouldClose(env.window, 1);
    }
    if (env.inputHandler.keys[GLFW_KEY_M]) {
      env.inputHandler.keys[GLFW_KEY_M] = false;
      wireframe = !wireframe;
      renderer.switchPolygonMode(wireframe ? PolygonMode::Line
                                           : PolygonMode::Fill);
    }
    if (env.inputHandler.keys[GLFW_KEY_F]) {
      env.inputHandler.keys[GLFW_KEY_F] = false;
      env.toggleFullscreen();
    }
  }
  delete model;
  delete model2;
  return (EXIT_SUCCESS);
}
