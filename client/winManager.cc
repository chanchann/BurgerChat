#include "winManager.h"
#include "chatClient.h"
#include <burger/base/Log.h>
#include <boost/lexical_cast.hpp>  

#include "cmdLine.h"
#include "color.h"

WinManager::WinManager(ChatClient* chatClient)
    : chatClient_(chatClient),
      interface_(util::make_unique<InterFace>()),
      quit_(false) {
    DEBUG("WinManager ctor.");
    interface_->runHeader();
    interface_->runInput();
    interface_->runOutput();
}

WinManager::~WinManager() {
    assert(quit_ == true);
}

// 主逻辑
void WinManager::start() {
    int notifyTimes = 0;
    while(!quit_) {
        // 如果还没登录
        if(chatClient_->logInState_ == ChatClient::LogInState::kNotLoggedIn) {
            interface_->changeHeader();
            outputMsg(">> Enter Your Choice: 1. login 2. signup 3. exit");

            std::string input = getInput();
            int choice = 0;
            try {  
                choice = boost::lexical_cast<int>(input);  
            } catch(boost::bad_lexical_cast& e) {  
                ERROR("{}", e.what());
            }
            switch (choice) {
                case 1: {
                    login();
                    break;
                }
                case 2: {
                    signup();
                    break;
                }
                case 3: {
                    outputMsg(">> Bye!");
                    chatClient_->getClient()->disconnect();
                    quit_ = true;
                    interface_.reset();
                    exit(0);
                    break;
                } 
                default: {
                    outputMsg(">> Invalid input!", "RED");
                    break;
                }
            }
        } else if(chatClient_->logInState_ == ChatClient::LogInState::kLogging) {
            // 登录界面
            if(notifyTimes < 10) {    
                outputMsg(">> Logging IN/OUT.....Please Wait......");
                ++notifyTimes;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            } else {
                outputMsg(">> Timeout, Press any key to go back to start menu......");
                std::cin.get();
                notifyTimes= 0;
                chatClient_->logInState_ = ChatClient::LogInState::kNotLoggedIn;
            }
        } else {
            notifyTimes = 0;
            mainMenu();
        }
    }
}

void WinManager::signup() {
    interface_->changeHeader("Sign Up");
    outputMsg(">> Username");    
    std::string name = getInput();  // todo : 此处用sstream是否更好
    // check name validation
    if(name.size() <= 2 || name.size() > 20) {
        outputMsg("\n Name is invalid, You must input 3 to 20 characters");
        outputMsg("Please enter any key to continue", true);
        std::cin.get();
        signup();
    }
    outputMsg(">> Password");   // todo : 如何变成密文
    std::string pwd = getInput();
    // check password validation
    // todo : 更加细化密码检查
    if(pwd.size() <= 2 || pwd.size() > 20) {
        outputMsg("\n Password is invalid, You must input 3 to 20 characters");
        outputMsg("Please enter any key to continue", true);
        std::cin.get();
        signup();
    }
    json js;
    js["msgid"] = REG_MSG;
    js["name"] = name;
    js["password"] = pwd;
    std::string request = js.dump();
    TRACE("request : {}", request);
    chatClient_->send(request);
}


void WinManager::login() {
    chatClient_->setLogInState_(ChatClient::LogInState::kLogging);
    interface_->changeHeader("Log In");
    std::string idStr;
    int id = 0;
    std::string pwd;

    while(true) {
        outputMsg(">> Input Your ID number");
        idStr = getInput();
        try {  
            id = boost::lexical_cast<int>(idStr);  
        } catch(boost::bad_lexical_cast& e) {  
            ERROR("{}", e.what());
        }
        if (id <= 0) {
            outputMsg("Invalid ID! Press any Key to try again", "RED");
            std::cin.get();
            continue;
        }
        break;
    }

    outputMsg(">> Input Your Password");

    noecho();  // 输入密码不显示 // todo : **** 形式? 
    pwd = getInput();
    echo();

    chatClient_->info_->setId(id);
    chatClient_->info_->setPwd(pwd); 

    json js;
    js["msgid"] = LOGIN_MSG;
    js["id"] = id;
    js["password"] = pwd;
    std::string request = js.dump();
    // std::cout << request << std::endl; // for test
    TRACE("request : {}", request);
    chatClient_->send(request);
}

// command need more tests
void WinManager::mainMenu() {
    interface_->changeHeader("Main Menu (Enter 'help' to get help)");
    while(chatClient_->logInState_ == ChatClient::kLoggedIn) {

        std::string input = getInput();
        std::string action;
        std::string flag = "";
        std::string content = "";
        
        std::vector<std::string> args;
        StringUtil::split(input, args);
        if(args.size() < 1) {
            continue;
        } else {
            // more eff?
            if(args.size() > 1) {
                if(args[1].size() > 0 && args[1][0] == '-') {
                    flag = args[1];
                    if(args.size() > 2) content = args[2];
                } else {
                    content = args[1];
                } 
            }
            
        } 
        action = args[0];
        if(CmdHandler::commandMap.find(action) == CmdHandler::commandMap.end()) {
           outputMsg(">> Invalid input!!!!", "RED"); 
        } else {
            auto func = CmdHandler::commandHandlerMap[action];
            func(chatClient_, flag, content);
        }
    }
}
