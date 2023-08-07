#include "topic.h"

void initializeMapOfTopics()
{    
    int tag = 0;
    for (QString t : topics) {
        mapTopicWithTag[t] = static_cast<TopicName>(tag);
        tag++;
    }
}
