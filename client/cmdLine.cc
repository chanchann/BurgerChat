#include "cmdLine.h"
#include "chatClient.h"
#include "color.h"

// 命令行参数
// 比如 add 默认添加还好友
// add -g 添加group
// ls 显示在线好友
// ls -a 显示所有好友

std::unordered_map<std::string, std::string> CmdHandler::commandMap = {
    // {"help", "显示所有支持的命令，格式:help"},
    {"help", "Show command list, usage: help"},
    {"chat", "Chat to one friend, usage: chat:friendid:message"},
    {"addFriend", "Add new friend, usage: addFriend:friendid"},
    {"confirmFriendRequest", "Handle friend requests, usage: confirmFriendRequest"},
    // {"creategroup", "创建群组，格式:creategroup:groupname:groupdesc"},
    // {"addgroup", "加入群组，格式:addgroup:groupid"},
    // {"groupchat", "群聊，格式:groupchat:groupid:message"},
    {"logout", "LogOut, usage: logout"},
    {"clear", "Clear the Output win, usage: clear"},
    // {"ls", "Show online friend, usage : ls"}, 暂时把ls 查看所有好友
    {"ls", "Show all friend, usage : ls"}
};

// 注册系统支持的客户端命令处理
std::unordered_map<std::string, std::function<void(ChatClient* , const std::string&, const std::string&)> > CmdHandler::commandHandlerMap = {
    {"help", CmdHandler::help},
    {"chat", CmdHandler::chat},
    {"addFriend", CmdHandler::addFriend},
    {"confirmFriendRequest", CmdHandler::confirmFriendRequest},
    // {"creategroup", CmdHandler::creategroup},
    // {"addgroup", CmdHandler::addgroup},
    // {"groupchat", CmdHandler::groupchat},
    {"logout", CmdHandler::logout},
    {"clear", CmdHandler::clear},
    {"ls", CmdHandler::showFriendList}
};

void CmdHandler::help(ChatClient* client, const std::string& flag, const std::string& str) {
    client->outputMsg(">> Show Command List: ");
    for (auto &p : commandMap) {
        std::string msg = p.first + " >>>> " + p.second;
        client->outputMsg(msg, true, "GREEN");
    }
}

void CmdHandler::chat(ChatClient* client, const std::string& flag, const std::string& msg) {
    size_t idx = msg.find(":"); // friendid:message
    if (idx == msg.npos) {
        client->outputMsg(">> Chat Command Invalid!", "RED");
        return;
    }

    UserId friendid = atoi(msg.substr(0, idx).c_str());  // boost::lexical_cast

    if(!client->info_->hasFriend(friendid)) {
        client->outputMsg(">> This User is not your friend yet!!", "RED");
        return;
    }

    std::string message = msg.substr(idx + 1, msg.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;

    js["id"] = client->getInfo()->getId();
    js["from"] = client->getInfo()->getName();
    js["to"] = friendid;
    js["msg"] = message;

    // TODO: add time
    // js["time"] = getCurrentTime();
    std::string content = js.dump();
    // std::cout << content << std::endl; // for test
    client->send(std::move(content));
}

void CmdHandler::logout(ChatClient* client, const std::string& flag, const std::string& msg) {
    assert(client->getLogInState() == ChatClient::kLoggedIn);
    client->setLogInState_(ChatClient::kLogging);

    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = client->getInfo()->getId();
    // js["name"] = client->getInfo()->getName();

    std::string content = js.dump();
    // std::cout << content << std::endl; // for test
    client->send(std::move(content));
}

void CmdHandler::addFriend(ChatClient* client, const std::string& flag, const std::string& msg) {
    
    UserId friendid = atoi(msg.c_str()); 

    // 如果已经是好友，直接返回提示消息即可
    if(client->info_->hasFriend(friendid)) {
        std::string m("User [");
        m += std::to_string(friendid) + "] is your friend already!";

        client->outputMsg(m, "RED");
        return;
    }   

    json js;
    js["msgid"] = ADD_FRIEND_MSG;

    js["id"] = client->getInfo()->getId();
    js["name"] = client->getInfo()->getName();
    js["friendid"] = friendid;
    js["addFriendRequestState"] = kApply;

    std::string content = js.dump();
    // std::cout << content << std::endl; // for test
    client->send(std::move(content));
}

void CmdHandler::confirmFriendRequest(ChatClient* client, const std::string& flag, const std::string&) {
    if(client->friendRequests_.empty()) {
        client->outputMsg(">> You don't have any new friend requests!!!", "YELLOW");
        return;
    }

    std::string msg(">> You have ");
    msg += std::to_string(client->friendRequests_.size()) + " new friend requests in total!";
    client->outputMsg(msg, "YELLOW");

    auto request = std::move(client->friendRequests_.front());
    client->friendRequests_.pop();

    UserId userid = request["id"].get<UserId>();
    std::string name = request["name"];

    json response(request);
    bool hasChoosed = false;
    while(!hasChoosed) {    
        msg = std::to_string(userid) + " (" + name + ")" + " wants to MAKE FRIEND with you!!!";

        client->outputMsg(msg, false, "YELLOW");
        client->outputMsg(">> Input : 1.Agree  2.Refuse  3.Handle it later... ", true, "YELLOW");

        std::string input = client->winManager_->getInput();
        int choice = atoi(input.c_str());
        switch (choice) {
            case 1: {
                response["addFriendRequestState"] = kAgree;
                User newFriend(name, "", "online", userid);
                client->info_->setFriends(std::move(newFriend));
                hasChoosed = true;
                break;
            }
            case 2: {
                response["addFriendRequestState"] = kRefuse;
                hasChoosed = true;
                break;
            }
            case 3: {
                response["addFriendRequestState"] = kPending;
                hasChoosed = true;
                break;
            } 
            default: {
                response["addFriendRequestState"] = kPending;
                hasChoosed = false;
                break;
            }
        }
    }
    std::string content = response.dump();
    client->send(std::move(content));
}

void CmdHandler::clear(ChatClient* client, const std::string& flag, const std::string&) {
    client->winManager_->clearOutPut();
}

void CmdHandler::showFriendList(ChatClient* client, const std::string& flag, const std::string&) {
    if(flag == "-a") {
        client->outputMsg("Show all friends");
        showAllFriendList(client);
    } else if(flag == "-o") {
        // todo : 暂时都显示全部好友
        client->outputMsg("Show friends offline(todo)");
        showAllFriendList(client);
    } else {
        client->outputMsg("Show friends online(todo)");
        showAllFriendList(client);
    }
}

void CmdHandler::showAllFriendList(ChatClient* client) {
    auto friendList = client->getInfo()->getFriendList();
    int num = 1;
    std::string friendStr = "";
    client->outputMsg("num\tUserId\t\tUserName", true);
    for(auto it = friendList.begin(); it != friendList.end(); it++) {
        friendStr += std::to_string(num++);
        friendStr += "\t";
        friendStr += std::to_string(it->first);
        friendStr += "\t\t";
        friendStr += it->second.getName();
        client->outputMsg(friendStr, true);
    }
}