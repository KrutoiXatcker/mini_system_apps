#include <iostream>
#include <pwd.h>


int main(){

    setpwent();

    for(struct passwd *pwd;pwd = getpwent();){
        std::cout <<pwd->pw_name << ", UID: " << pwd->pw_uid << ", GID:" << pwd->pw_gid << std::endl;
    }

    endpwent();

    return 0;
}