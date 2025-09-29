#include <iostream>
#include <thread>

#include "gpiod.hpp"

int main() {
  try {
    gpiod::chip chip("gpiochip4");
    gpiod::line line6 = chip.get_line(6);
    line6.request({"Test", gpiod::line_request::DIRECTION_INPUT, 0});
    for (int i = 0; i < 10; ++i) {
      auto value = line6.get_value();
      std::cout << "GPIO4_A6 = " << value << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    line6.release();
  } catch (const std::exception& e) {
    std::cerr << "Error" << e.what() << std::endl;
    return 1;
  }
  return 0;
}
