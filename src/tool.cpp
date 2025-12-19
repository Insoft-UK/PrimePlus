// The MIT License (MIT)
//
// Copyright (c) 2024-2025 Insoft.
//
// Created: 2025-12-19
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "tool.hpp"

#if __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#include <unistd.h>
#include <cstdlib>
#include <spawn.h>
#include <sys/wait.h>
#include <cstring>

using namespace tool;

static std::string executableDir(void)
{
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::string path(size, '\0');
    _NSGetExecutablePath(path.data(), &size);

    char resolved[PATH_MAX];
    realpath(path.c_str(), resolved);

    std::string fullPath(resolved);
    return fullPath.substr(0, fullPath.find_last_of('/'));
}

extern char **environ;



static std::string readFd(int fd)
{
    std::string result;
    char buffer[4096];

    ssize_t n;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, n);
    }
    return result;
}

result_t tool::runTool(const std::string& command, const std::vector<std::string>& arguments)
{
    std::string dir  = executableDir();
    std::string tool = dir + "/" + command;

    // Pipes for stdout and stderr
    int outPipe[2];
    int errPipe[2];
    pipe(outPipe);
    pipe(errPipe);

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);

    // Redirect child's stdout -> outPipe[1]
    posix_spawn_file_actions_adddup2(&actions, outPipe[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, errPipe[1], STDERR_FILENO);

    // Close unused fds in child
    posix_spawn_file_actions_addclose(&actions, outPipe[0]);
    posix_spawn_file_actions_addclose(&actions, errPipe[0]);

    // Build argv
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(tool.c_str()));
    for (const auto& arg : arguments)
        argv.push_back(const_cast<char*>(arg.c_str()));
    argv.push_back(nullptr);

    pid_t pid;
    int rc = posix_spawn(&pid, tool.c_str(), &actions, nullptr,
                         argv.data(), environ);

    // Parent doesn't write
    close(outPipe[1]);
    close(errPipe[1]);

    result_t result{};

    if (rc == 0) {
        result.out = readFd(outPipe[0]);
        result.err = readFd(errPipe[0]);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            result.exitCode = WEXITSTATUS(status);
        else
            result.exitCode = -1;
    } else {
        result.exitCode = rc;
        result.err = std::strerror(rc);
    }

    close(outPipe[0]);
    close(errPipe[0]);

    posix_spawn_file_actions_destroy(&actions);
    return result;
}
#endif // __APPLE__
