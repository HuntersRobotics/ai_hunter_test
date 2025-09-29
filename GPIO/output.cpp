#include <iostream>
#include <thread>

#include "gpiod.hpp"

int main() {
  try {
    gpiod::chip chip("gpiochip4");
    gpiod::line line6 = chip.get_line(6);
    line6.request({"Test", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
    for (int i = 0; i < 10; ++i) {
      line6.set_value(1);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      line6.set_value(0);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    line6.set_value(0);
    line6.release();
  } catch (const std::exception& e) {
    std::cerr << "Error" << e.what() << std::endl;
    return 1;
  }
  return 0;
}
