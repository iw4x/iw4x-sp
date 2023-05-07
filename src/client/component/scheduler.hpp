#pragma once

using namespace std::literals;

namespace scheduler {
enum pipeline {
  // Asynchronuous pipeline, disconnected from the game
  async = 0,

  // The game's rendering pipeline
  renderer,

  // The game's server thread
  server,

  // The game's main thread
  main,

  count,
};

static constexpr auto cond_continue = false;
static constexpr auto cond_end = true;

void clear_tasks(pipeline type);

void schedule(const std::function<bool()>& callback, pipeline type,
              std::chrono::milliseconds delay = 0ms);
void loop(const std::function<void()>& callback, pipeline type,
          std::chrono::milliseconds delay = 0ms);
void once(const std::function<void()>& callback, pipeline type,
          std::chrono::milliseconds delay = 0ms);
} // namespace scheduler
