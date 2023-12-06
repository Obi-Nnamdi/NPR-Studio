#include <iostream>
#include <chrono>

#include "ToonViewerApp.hpp"

using namespace GLOO;

int main() {
  auto app = make_unique<ToonViewerApp>("Assignment5", glm::ivec2(1440, 900));

  app->SetupScene();

  using Clock = std::chrono::high_resolution_clock;
  using TimePoint =
      std::chrono::time_point<Clock, std::chrono::duration<double>>;
  TimePoint last_tick_time = Clock::now();
  TimePoint start_tick_time = last_tick_time;
  while (!app->IsFinished()) {
    TimePoint current_tick_time = Clock::now();
    double delta_time = (current_tick_time - last_tick_time).count();
    double total_elapsed_time = (current_tick_time - start_tick_time).count();
    app->Tick(delta_time, total_elapsed_time);
    last_tick_time = current_tick_time;
  }
  return 0;
}
