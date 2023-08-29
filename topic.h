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
    BOARD_STATUS,
    TEMP,
    WEATHER
} TopicName;

const QString BASE = "/mseei/imicro/nrr/";

const QString topics[] = {
    BASE + "COMMAND",
    BASE + "LED",
    BASE + "BUTTONS",
    BASE + "ADC",
    BASE + "BOARD/STATUS",
    BASE + "TEMP",
    BASE + "WEATHER"
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
    ACK_MODE_PUSH_BUTTONS_POLL,
    START_TEMP_SAMPLING,
    ACK_START_TEMP_SAMPLING,
    STOP_TEMP_SAMPLING,
    ACK_STOP_TEMP_SAMPLING
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
    "ack_no_async_buttons",
    "start_temp",
    "ack_start_temp",
    "stop_temp",
    "ack_stop_temp"
};

class CommandParams
{
public:
        // Constructor that takes an initializer list
    CommandParams(std::initializer_list<initializer_list<QString>> values)
    {
        for (initializer_list<QString> pair : values) {
            if (pair.size() == 2)
            {
                QString param, val;
                int i = 1;
                for (QString s : pair)
                {
                    switch (i)
                    {
                    case 1:
                        param = s;
                        break;
                    case 2:
                        val = s;
                        break;
                    }
                    i++;
                }
                i = 1;
                mapParamValue[param] = val;
            }
        }
    }

    // Make the class iterable
    map<QString,QString>::iterator begin()
    {
        return mapParamValue.begin();
    }

    map<QString,QString>::iterator end()
    {
        return mapParamValue.end();
    }

private:
    std::map<QString, QString> mapParamValue;
};

// Map to associate the strings topics with the enum values TopicName
// to use switch to be able to take actions by topic of received messages
extern std::map<QString, TopicName> mapTopicWithTag;

void initializeMapOfTopics();

#endif // TOPIC_H
