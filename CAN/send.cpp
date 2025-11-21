#include "kaylordut/log/logger.h"
#include "can/driver_can.h"
int main() {
    auto driver_can = std::make_shared<DriverCan>("can0");
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7};
    driver_can->CanWrite(0x123, data, 8);
    return 0;
}
