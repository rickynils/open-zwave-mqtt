#include <openzwave/Notification.h>

extern uint32_t home_id;

void
set_config_param(const std::string& topic, const std::string& value);

void
process_notification(const OpenZWave::Notification*, void*);
