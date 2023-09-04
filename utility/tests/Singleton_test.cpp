//
// Created by zhengqi on 2023/9/2.
//

#include "utility/noncopyable.h"
#include "utility/Singleton.h"
#include <string>
#include <thread>

class Test : zhengqi::utility::noncopyable
{
public:
    Test() {}
    ~Test() {}

    const std::string& name() const { return name_; }
    void setName(const std::string& n) { name_ = n; }
private:
    std::string name_;
};

int main()
{
    zhengqi::utility::Singleton<Test>::instance().setName("only one");
    printf("%s\n", zhengqi::utility::Singleton<Test>::instance().name().c_str());
    std::thread thread_([](){
        zhengqi::utility::Singleton<Test>::instance().setName("only one change");
        printf("%s\n", zhengqi::utility::Singleton<Test>::instance().name().c_str());
    });
    thread_.join();
    printf("%s\n", zhengqi::utility::Singleton<Test>::instance().name().c_str());
}