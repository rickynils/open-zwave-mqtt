
#include <string>
#include <map>

struct options {
    options();

    bool parse_argv(int argc, const char* argv[]);

    std::string system_config;
    std::string user_config;
    std::string device;
    std::string mqtt_host;
    std::string mqtt_client_id;
    std::string mqtt_user;
    std::string mqtt_passwd;
    std::string mqtt_prefix;
    uint16_t mqtt_port;
    uint32_t log_level;
};

void print_help();
