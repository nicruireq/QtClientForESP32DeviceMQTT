#ifndef TOPIC_H
#define TOPIC_H

#include <qstring.h>
#include <map>

using namespace std;

// Keep synchronized enum TopicName
// with array topics
typedef enum TopicName {
    COMMAND,
    LED,
    BUTTONS,
    ADC,
    BOARD_STATUS
} TopicName;

const QString BASE = "/mseei/imicro/nrr/";

const QString topics[] = {
    BASE + "COMMAND",
    BASE + "LED",
    BASE + "BUTTONS",
    BASE + "ADC",
    BASE + "BOARD/STATUS"
};

typedef enum Commands {
    PING,
    PING_RESPONSE,
    POLL_BUTTONS,
    MODE_LEDS_PWM,
    ACK_MODE_LEDS_PWM,
    MODE_LEDS_GPIO,
    ACK_MODE_LEDS_GPIO,
    MODE_PUSH_BUTTONS_ASYNC,
    ACK_MODE_PUSH_BUTTONS_ASYNC,
    MODE_PUSH_BUTTONS_POLL,
    ACK_MODE_PUSH_BUTTONS_POLL
} Commands;

const QString topicCommandCmds[] = {
    "ping",
    "ping_response",
    "poll_buttons",
    "mode_leds_pwm",
    "ack_mode_leds_pwm",
    "mode_leds_gpio",
    "ack_mode_leds_gpio",
    "async_buttons",
    "ack_async_buttons",
    "no_async_buttons",
    "ack_no_async_buttons"
};

// Map to associate the strings topics with the enum values TopicName
// to use switch to be able to take actions by topic of received messages
extern std::map<QString, TopicName> mapTopicWithTag;

void initializeMapOfTopics();

#endif // TOPIC_H
