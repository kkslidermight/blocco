#include "engine.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include "camera.hpp"
#include "config.hpp"
#include <chrono>
#include <thread>
#include <stdexcept>

Engine::Engine(bool headless):m_headless(headless){init();}
Engine::~Engine(){shutdown();}

void Engine::init(){
  m_config = std::make_unique<Config>();
  m_input = std::make_unique<InputSystem>();
  m_camera = std::make_unique<Camera>();
  m_renderer = std::make_unique<Renderer>(m_headless);
  m_running = true;
}

void Engine::run(){
  using clock = std::chrono::steady_clock;
  auto last = clock::now();
  while(m_running){
    auto now = clock::now();
    float dt = std::chrono::duration<float>(now-last).count();
    last = now;
    update(dt);
    render();
  }
}

void Engine::headlessCapture(int frames){
  for(int i=0;i<frames;++i){
    update(1.f/60.f);
    render();
  }
}

void Engine::update(float dt){
  m_time += dt;
  // TODO input, camera, physics
  if(m_time>2.0f && m_headless){ m_running=false; }
}

void Engine::render(){
  m_renderer->drawFrame();
}

void Engine::shutdown(){
  m_renderer.reset();
}
