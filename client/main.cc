#include "chatClient.h"
#include <burger/net/EventLoopThread.h>

int main() {
    LOGGER;
    EventLoopThread loopThread;
    InetAddress serverAddr("127.0.0.1", 8888);
    ChatClient client(loopThread.startLoop(), serverAddr); // todo : 这里要用startLoop -- 不然主线程一直在loop

    client.start();
}
