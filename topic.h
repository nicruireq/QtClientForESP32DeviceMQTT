#ifndef TOPIC_H
#define TOPIC_H

#include <qstring.h>

using namespace std;

enum TopicName {
    COMMAND
};

const QString BASE = "/mseei/imicro/nrr/";

const QString topics[] = {
  BASE + "COMMAND"
};

enum Commands {
    PING,
    PING_RESPONSE
};

const QString topicCommandCmds[] = {
    "ping",
    "ping_response"
};

#endif // TOPIC_H
