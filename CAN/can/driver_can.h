//
// Created by Kaylor on 22-11-21.
//

#ifndef MOTORDRIVER_DRIVER_DRIVERCAN_H_
#define MOTORDRIVER_DRIVER_DRIVERCAN_H_

#include <kaylordut/log/logger.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>

#include "linux/can/error.h"
#include "memory"
#include "mutex"
#include "queue"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "thread"

typedef struct {
  uint8_t data[8] = {0};
  uint8_t size = 0;
  uint32_t can_id = 0;
  double timestamp = 0.0;
}CanFrame;


class DriverCan {
 public:
  DriverCan(const char *str, const int loopback = 0);
  ~DriverCan();

  double get_now() const {
    auto now = std::chrono::system_clock::now();
    auto duration_since_epoch = now.time_since_epoch();
    return std::chrono::duration<double>(duration_since_epoch).count();
  };

  void CanFilterConfig(const std::vector<struct can_filter> &filters);

  bool CanWrite(uint32_t can_id, const uint8_t *data, uint8_t size);

  void CanRead(uint32_t *can_id, uint8_t *data, uint8_t *size);

  std::shared_ptr<CanFrame> get_next_can_frame(uint8_t timeout_ms = 10);

  bool DropOneCanFrame(void);

  void ClearReadQueue();

  size_t GetReadQueueSize(void);

  void set_stop_flag(bool stop) { stop_ = stop; }

 private:
  void ReceiveCallback();
  void CanFiltersConfig(const void *rfilter, int size);
  std::vector<struct can_filter> can_filters_;
  char device_string_[32];
  std::queue<std::shared_ptr<CanFrame>> read_queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread receive_thread_;
  bool stop_{false};

 protected:
  int can_fd_;
};

#endif //MOTORDRIVER_DRIVER_DRIVERCAN_H_
