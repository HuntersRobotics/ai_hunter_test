//
// Created by Kaylor on 22-11-21.
//

#include "driver_can.h"

void DriverCan::ReceiveCallback() {
  pthread_setname_np(pthread_self(), "can_receive");
  while (!stop_) {
    auto frame = std::make_shared<CanFrame>();
    CanRead(&frame->can_id, frame->data, &frame->size);
    frame->timestamp = this->get_now();
    std::lock_guard<std::mutex> lock(this->mutex_);
    for (size_t i = 0; i < can_filters_.size(); ++i) {
      if ((frame->can_id & can_filters_.at(i).can_mask) ==
          (can_filters_.at(i).can_id & can_filters_.at(i).can_mask)) {
        this->read_queue_.push(frame);
      }
    }
  }
  KAYLORDUT_LOG_INFO("exit can received thread");
}

DriverCan::DriverCan(const char *str, const int loopback) {
  memset(device_string_, 0, sizeof(device_string_));
  strcpy(device_string_, str);
  KAYLORDUT_LOG_DEBUG("device is {}\r\n", device_string_);
  can_fd_ = socket(AF_CAN, SOCK_RAW, CAN_RAW);
  if (can_fd_ < 0) {
    KAYLORDUT_LOG_ERROR("socket can create error!\n");
    exit(EXIT_FAILURE);
  }
  struct ifreq ifr;
  strcpy(ifr.ifr_name, str);
  ioctl(can_fd_, SIOCGIFINDEX, &ifr);

  struct sockaddr_can addr;
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  /*关闭回环模式*/
  //  int loopback = 0; /* 0 = disabled, 1 = enabled (default) */
  setsockopt(can_fd_, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback,
             sizeof(loopback));

  /*关闭自收自发*/
  int recv_own_msgs = 0; /* 0 = disabled (default), 1 = enabled */
  setsockopt(can_fd_, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs,
             sizeof(recv_own_msgs));

  // 将套接字与 can_fd_ 绑定
  int bind_res = bind(can_fd_, (struct sockaddr *) &addr, sizeof(addr));
  if (bind_res < 0) {
    KAYLORDUT_LOG_ERROR("bind can device error!");
    exit(EXIT_FAILURE);
  }
  receive_thread_ = std::thread([this] { this->ReceiveCallback(); });
}

DriverCan::~DriverCan() {
  if (receive_thread_.joinable()) {
    receive_thread_.join();
  }
  KAYLORDUT_LOG_INFO("~DriverCan() is called");
}

void DriverCan::CanFiltersConfig(const void *rfilter, int size) {
  if (setsockopt(can_fd_, SOL_CAN_RAW, CAN_RAW_FILTER, rfilter, size) < 0) {
    KAYLORDUT_LOG_ERROR("Can filter configuration failed");
  }else{
    KAYLORDUT_LOG_DEBUG("Can filter configuration succeeded");
  }
}

void DriverCan::CanFilterConfig(const std::vector<struct can_filter> &filters) {
  CanFiltersConfig(filters.data(), filters.size() * sizeof(struct can_filter));
  std::lock_guard<std::mutex> lock(mutex_);
  can_filters_ = filters;
}

bool DriverCan::CanWrite(uint32_t can_id, const uint8_t *data, uint8_t size) {
  struct can_frame frame;
  memcpy(frame.data, data, size);
  frame.can_dlc = size;
  frame.can_id = can_id;
  int nbytes = write(can_fd_, &frame, sizeof(struct can_frame));
  if (nbytes != sizeof(frame)) {
    KAYLORDUT_LOG_ERROR("write error, nbytes = {}\n", nbytes);
    return false;
  } else {
    //    KAYLORDUT_LOG_DEBUG("write successful, id = {:X} [{}] ", can_id,
    //    size); std::string data_str; for (int i = 0; i < size; ++i) {
    //      char tmp[10];
    //      sprintf(tmp, "%02X", data[i]);
    //      data_str += tmp;
    //    }
    //    KAYLORDUT_LOG_DEBUG("{}", data_str);
    return true;
  }
}

void DriverCan::CanRead(uint32_t *can_id, uint8_t *data, uint8_t *size) {
  struct can_frame can_frame;
  auto nbytes = read(can_fd_, &can_frame, sizeof(struct can_frame));
  if (nbytes < 0) {
    KAYLORDUT_LOG_ERROR("can raw socket read");
    close(can_fd_);
    _exit(-1);
  }

  if (static_cast<size_t>(nbytes) < sizeof(struct can_frame)) {
    KAYLORDUT_LOG_DEBUG("read: incomplete CAN frame\n");
    close(can_fd_);
    _exit(-1);
  }
  *can_id = can_frame.can_id;
  *size = can_frame.can_dlc;
  memcpy(data, can_frame.data, can_frame.can_dlc);
  //  KAYLORDUT_LOG_DEBUG("read successful id = {:X} [{}] ", *can_id, *size);
  //  std::string data_str;
  //  for (int i = 0; i < *size; ++i) {
  //    char tmp[10];
  //    sprintf(tmp, "%02X", *(data + i));
  //    data_str += tmp;
  //  }
  //  KAYLORDUT_LOG_DEBUG("{}", data_str);
}

std::shared_ptr<CanFrame> DriverCan::get_next_can_frame(uint8_t timeout_ms) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                   [this] { return !this->read_queue_.empty(); })) {
    auto ret = read_queue_.front();
    read_queue_.pop();
    return ret;
  } else {
    KAYLORDUT_LOG_WARN("read receive queue time out");
  }
  return nullptr;
}

void DriverCan::ClearReadQueue() {
  std::queue<std::shared_ptr<CanFrame>> empty;
  std::lock_guard<std::mutex> lock(mutex_);
  std::swap(empty, read_queue_);
}

size_t DriverCan::GetReadQueueSize(void) {
  std::lock_guard<std::mutex> lock(mutex_);
  return read_queue_.size();
}

bool DriverCan::DropOneCanFrame(void) {
  bool ret = false;
  mutex_.lock();
  if (!read_queue_.empty()) {
    read_queue_.pop();
    ret = true;
  }
  mutex_.unlock();
  return ret;
}
