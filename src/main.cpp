#include "env.hpp"
#include "game.hpp"
#include "renderer.hpp"

int main(int argc, char **argv) {
  Env env(1280, 720);
  render::Renderer renderer(env.width, env.height);
  renderer.loadCubeMap(
      "shaders/skybox.vert", "shaders/skybox.frag",
      {"textures/skybox_side.png", "textures/skybox_side.png",
       "textures/skybox_up.png", "textures/skybox_bottom.png",
       "textures/skybox_side.png", "textures/skybox_side.png"});
  GL_DUMP_ERROR("renderer init");
  Game game;
  float lag = 0.0f;
  while (!glfwWindowShouldClose(env.window)) {
    env.update();
    glfwPollEvents();
    game.update(env);
    renderer.update(env);
    renderer.clearScreen();
    game.render(env, renderer);
    GL_DUMP_ERROR("draw loop");
    glfwSwapBuffers(env.window);
  }
  return (EXIT_SUCCESS);
}
