#pragma once
#include <memory>
#include <string>
#include <vector>
class Renderer;
class InputSystem;
class Camera;
struct Config;
class Engine {
public:
  Engine(bool headless=false);
  ~Engine();
  void run();
  void headlessCapture(int frames);
private:
  void init();
  void update(float dt);
  void render();
  void shutdown();
  bool m_headless{false};
  bool m_running{false};
  float m_time{0.f};
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<InputSystem> m_input;
  std::unique_ptr<Camera> m_camera;
  std::unique_ptr<Config> m_config;
};
