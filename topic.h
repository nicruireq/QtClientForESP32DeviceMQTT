#ifndef TOPIC_H
#define TOPIC_H

#include <qstring.h>
#include <map>

using namespace std;

typedef enum TopicName {
    COMMAND,
    LED,
    BUTTONS
} TopicName;

const QString BASE = "/mseei/imicro/nrr/";

const QString topics[] = {
    BASE + "COMMAND",
    BASE + "LED",
    BASE + "BUTTONS"
};

typedef enum Commands {
    PING,
    PING_RESPONSE,
    POLL_BUTTONS,
    MODE_LEDS_PWM,
    ACK_MODE_LEDS_PWM,
    MODE_LEDS_GPIO,
    ACK_MODE_LEDS_GPIO
} Commands;

const QString topicCommandCmds[] = {
    "ping",
    "ping_response",
    "poll_buttons",
    "mode_leds_pwm",
    "ack_mode_leds_pwm",
    "mode_leds_gpio",
    "ack_mode_leds_gpio"
};

// Map to associate the strings topics with the enum values TopicName
// to use switch to be able to take actions by topic of received messages
extern std::map<QString, TopicName> mapTopicWithTag;

void initializeMapOfTopics();

#endif // TOPIC_H
