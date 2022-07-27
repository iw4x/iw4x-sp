#include <std_include.hpp>

namespace game {
namespace environment {
launcher::mode mode = launcher::mode::none;

bool is_sp() { return get_mode() == launcher::mode::singleplayer; }

launcher::mode get_mode() {
  if (mode == launcher::mode::none) {
    throw std::runtime_error(
        "Launcher mode not valid. Something must be wrong.");
  }

  return mode;
}

void set_mode(const launcher::mode _mode) { mode = _mode; }
} // namespace environment

int FS_FOpenFileReadForThread(const char* filename, void** file,
                              FsThread thread) {
  const static DWORD func = 0x630380;
  int answer{};

  __asm {
    pushad;

    mov eax, file;
    push thread;
    push filename;
    call func;
    add esp, 0x8;
    mov answer, eax;

    popad;
  }

  return answer;
}
} // namespace game
