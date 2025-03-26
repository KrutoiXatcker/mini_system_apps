#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

int System(const char* cmd) {
    pid_t pid;
    int status;

    if ((pid = fork()) == -1) {
        return -1;
    }
    else if (pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
        perror("execl");
        exit(errno);
    }

    // Parent process
    if (waitpid(pid, &status, 0) == pid && WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    
    return -1;
}

int main() {
    int rc = 0;
    char buf[256];

    do {
        printf("sh> "); 
        fflush(stdout);
        
        if (!fgets(buf, sizeof(buf), stdin)) {
            break;  // EOF or error
        }
        
        // Remove newline and check for empty input
        buf[strcspn(buf, "\n")] = '\0';
        if (strlen(buf) == 0) {
            continue;
        }
        
        rc = System(buf);
    } while (!rc);

    return rc;
}