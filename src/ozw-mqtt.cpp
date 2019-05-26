
#include <signal.h>
#include <stdio.h>
#include <openzwave/Options.h>
#include <openzwave/Manager.h>
#include <openzwave/Driver.h>
#include <openzwave/platform/Log.h>

#include "node_value.h"
#include "mqtt.h"
#include "process_notification.h"
#include "options.h"

using namespace std;


// Callback for MQTT include devices topic
void
start_inclusion(const string& topic, const string& value)
{
    OpenZWave::Manager::Get()->AddNode(home_id, false);
    printf("OZW inclusion mode activated.\n");
}

// Callback for MQTT heal network topic
void
heal_network(const string& topic, const string& value)
{
    OpenZWave::Manager::Get()->HealNetwork(home_id, true);
    printf("OZW heal network activated.\n");
}

void
signal_handler(int s)
{
    OpenZWave::Manager::Get()->Destroy();
    printf("Caught signal %d\n",s);
    exit(1);
}

//-----------------------------------------------------------------------------
// <main>
//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
    // Parse command line options
    try {
        options opt;
        opt.parse_argv(argc, argv);

        // Parse topics filter list, if desired
        if (!opt.topics_file.empty()) {
            opt.parse_topics_file();
            printf("Loaded %u topics to publish to.\n", opt.topic_overrides.size());
        }

        // Setup signal handling
        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = signal_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);

        // Make a connection to MQTT broker
        if (!opt.print_topics_only) {
            printf("Connecting to MQTT Broker %s:%d...", opt.mqtt_host.c_str(), opt.mqtt_port);
            mqtt_connect(opt.mqtt_client_id, opt.mqtt_host, opt.mqtt_port, opt.mqtt_user, opt.mqtt_passwd);
            // Register save config mqtt topic
            mqtt_subscribe(opt.mqtt_prefix, "ozw/heal_network", heal_network);
            mqtt_subscribe(opt.mqtt_prefix, "ozw/start_inclusion", start_inclusion);
        } else {
            printf("Print MQTT topics only mode selected, gathering nodes information.\n");
            printf("This will take awhile (up to few minutes, depending on amount of nodes)...\n");
        }

        // Create the OpenZWave Manager.
        // The first argument is the path to the config files (where the manufacturer_specific.xml file is located
        // The second argument is the path for saved Z-Wave network state and the log file.  If you leave it NULL
        // the log file will appear in the program's working directory.
        OpenZWave::Options::Create(opt.system_config, opt.user_config, "");
        OpenZWave::Options::Get()->AddOptionInt("SaveLogLevel", opt.log_level);
        OpenZWave::Options::Get()->AddOptionInt("QueueLogLevel", opt.log_level);
        OpenZWave::Options::Get()->AddOptionInt("DumpTrigger", OpenZWave::LogLevel_Error);
        OpenZWave::Options::Get()->AddOptionBool("AppendLogFile", true);
        OpenZWave::Options::Get()->Lock();

        OpenZWave::Manager::Create();

        // Add a callback handler to the manager.
        OpenZWave::Manager::Get()->AddWatcher(process_notification, &opt);
        // Add a Z-Wave Driver
        OpenZWave::Manager::Get()->AddDriver(opt.device);
        // Default poll interval is 2s.
        // NOTE: only devices explicitly enabled for polling will be polled.
        OpenZWave::Manager::Get()->SetPollInterval(500, true);

        // Main loop
        mqtt_loop();
    } catch (param_error e) {
        printf("Param error: %s\n", e.what());
    } catch (exception e) {
        printf("Error: %s\n", e.what());
    }
    return 0;
}
