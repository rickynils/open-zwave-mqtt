
#ifndef MOCK_MANAGER_H
#define MOCK_MANAGER_H

#include <openzwave/Manager.h>

void mock_manager_set_value_readonly(const OpenZWave::ValueID&);

void mock_manager_cleanup();

#endif