#ifndef USER_H
#define USER_H

#include <string>

using UserId = int64_t;
// User表的ORM类
class User {
public:
    User(std::string name = "", std::string pwd = "", std::string state = "offline", UserId id = -1)
        : name_(name),
        password_(pwd),
        state_(state),
        id_(id) {
    }
    ~User() = default;
    void setId(UserId id) { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setPwd(std::string pwd) { password_ = pwd; }
    void setState(std::string state) { state_ = state; }

    UserId getId() const { return id_; }
    std::string getName() const { return name_; }
    std::string getPwd() const { return password_; }
    std::string getState() { return state_; }

protected:
    std::string name_;
    std::string password_;
    std::string state_;
    UserId id_;
};


#endif // USER_H