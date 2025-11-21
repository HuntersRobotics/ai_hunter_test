//
// Created by kaylor on 11/21/25.
//
#include "can/driver_can.h"
#include "kaylordut/log/logger.h"
#include "sstream"
int main() {
  auto driver_can = std::make_shared<DriverCan>("can0");
  std::vector<struct can_filter> filters;
  filters.emplace_back(0x123, CAN_SFF_MASK);
  driver_can->CanFilterConfig(filters);
  while (true) {
    auto frame = driver_can->get_next_can_frame(100);
    if (frame) {
      std::stringstream ss;
      ss << "can id: " << std::hex << frame->can_id;
      ss << ", size = " << static_cast<int>(frame->size) << ", data: ";
      for (size_t i = 0; i < frame->size; i++) {
        ss << std::hex << std::uppercase << static_cast<int>(frame->data[i])
           << " ";
      }
      ss << std::endl;
      KAYLORDUT_LOG_INFO("{}", ss.str());
    } else {
      KAYLORDUT_LOG_INFO("frame is null");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    KAYLORDUT_LOG_INFO("running");
  }
  return 0;
}
