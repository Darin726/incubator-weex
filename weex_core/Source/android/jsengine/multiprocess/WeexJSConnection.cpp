/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "WeexJSConnection.h"

#include "ashmem.h"
#include "ExtendJSApi.h"
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <IPC/IPCFutexPageQueue.h>
#include <IPC/IPCException.h>
#include <IPC/IPCSender.h>
#include <unistd.h>
#include <android/base/log_utils.h>
#include <errno.h>
#include <android/utils/so_utils.h>
#include <IPCListener.h>
#include <android/bridge/impl/android_bridge_in_multi_process.h>
#include <core/manager/weex_core_manager.h>
#include <core/bridge/script_bridge_in_multi_process.h>

static void doExec(int fdClient, int fdServer, bool traceEnable, bool startupPie);

static int copyFile(const char *SourceFile, const char *NewFile);

static void closeAllButThis(int fd, int fd2);

static void printLogOnFile(const char *log);

#if PRINT_LOG_CACHEFILE
static std::string logFilePath = "/data/data/com.taobao.taobao/cache";
#endif

struct WeexJSConnection::WeexJSConnectionImpl {
    std::unique_ptr<IPCSender> serverSender;
    std::unique_ptr<IPCFutexPageQueue> futexPageQueue;
    pid_t child{0};
};

WeexJSConnection::WeexJSConnection()
        : m_impl(new WeexJSConnectionImpl) {
}

WeexJSConnection::~WeexJSConnection() {
  end();
}

struct ThreadData {
    int ipcServerFd;
    IPCHandler *ipcServerHandler;
};
pthread_t ipcServerThread;
static volatile bool finish = false;


static void *newIPCServer(void *_td) {
    ThreadData *td = static_cast<ThreadData *>(_td);
    void *base = mmap(nullptr, IPCFutexPageQueue::ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                      td->ipcServerFd, 0);
    if (base == MAP_FAILED) {
        int _errno = errno;
        close(td->ipcServerFd);
        throw IPCException("failed to map ashmem region: %s", strerror(_errno));
    }

    IPCHandler *handler = td->ipcServerHandler;
    std::unique_ptr<IPCFutexPageQueue> futexPageQueue(
            new IPCFutexPageQueue(base, IPCFutexPageQueue::ipc_size, 0));
    const std::unique_ptr<IPCHandler> &testHandler = createIPCHandler();
    std::unique_ptr<IPCSender> sender(createIPCSender(futexPageQueue.get(), handler));
    std::unique_ptr<IPCListener> listener =std::move(createIPCListener(futexPageQueue.get(), handler)) ;
    finish = true;

    try {
      futexPageQueue->spinWaitPeer();
      listener->listen();
    } catch (IPCException &e) {
        LOGE("server died");
//        killIpcServer();
        close(td->ipcServerFd);
    }
}


IPCSender *WeexJSConnection::start(IPCHandler *handler, IPCHandler *serverHandler, bool reinit) {
  int fd = ashmem_create_region("WEEX_IPC_CLIENT", IPCFutexPageQueue::ipc_size);
  if (-1 == fd) {
    throw IPCException("failed to create ashmem region: %s", strerror(errno));
  }
  void *base = mmap(nullptr, IPCFutexPageQueue::ipc_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd, 0);
  if (base == MAP_FAILED) {
    int _errno = errno;
    close(fd);
    throw IPCException("failed to map ashmem region: %s", strerror(_errno));
  }
  std::unique_ptr<IPCFutexPageQueue> futexPageQueue(
          new IPCFutexPageQueue(base, IPCFutexPageQueue::ipc_size, 0));
  std::unique_ptr<IPCSender> sender(createIPCSender(futexPageQueue.get(), handler));
  m_impl->serverSender = std::move(sender);
  m_impl->futexPageQueue = std::move(futexPageQueue);

  int fd2 = ashmem_create_region("WEEX_IPC_SERVER", IPCFutexPageQueue::ipc_size);
  if (-1 == fd2) {
    throw IPCException("failed to create ashmem region: %s", strerror(errno));
  }
  ThreadData td = { static_cast<int>(fd2), static_cast<IPCHandler *>(serverHandler) };

  pthread_attr_t threadAttr;
  finish = false;
  if (ipcServerThread != 0)
    pthread_kill(ipcServerThread, 0);

  pthread_attr_init(&threadAttr);
  int i = pthread_create(&ipcServerThread, &threadAttr, newIPCServer, &td);
  while (!finish) {
    continue;
  }


#if PRINT_LOG_CACHEFILE
  if (s_cacheDir) {
    logFilePath = s_cacheDir;
  }
  logFilePath.append("/jsserver_start.log");
  std::ofstream mcfile;
  if (reinit) {
    mcfile.open(logFilePath, std::ios::app);
    mcfile << "restart fork a process" << std::endl;
  } else {
    mcfile.open(logFilePath);
    mcfile << "start fork a process" << std::endl;
  }
#endif

//  static bool startupPie = s_start_pie;
  static bool startupPie = SoUtils::pie_support();
  LOGE("startupPie :%d", startupPie);

  pid_t child;
  if (reinit) {
#if PRINT_LOG_CACHEFILE
    mcfile << "reinit is ture use vfork" << std::endl;
    mcfile.close();
#endif
    child = vfork();
  } else {
#if PRINT_LOG_CACHEFILE
    mcfile << "reinit is false use fork" << std::endl;
    mcfile.close();
#endif
    child = fork();
  }
  if (child == -1) {
    int myerrno = errno;
    munmap(base, IPCFutexPageQueue::ipc_size);
    close(fd);
    close(fd2);
    throw IPCException("failed to fork: %s", strerror(myerrno));
  } else if (child == 0) {
    // the child
//    closeAllButThis(fd, fd2);
    // implements close all but handles[1]
    // do exec
    doExec(fd, fd2, true, startupPie);
    LOGE("exec Failed completely.");
    // failed to exec
    _exit(1);
  } else {
    printLogOnFile("fork success on main process and start m_impl->futexPageQueue->spinWaitPeer()");
    close(fd);
    close(fd2);
    m_impl->child = child;
    try {
      m_impl->futexPageQueue->spinWaitPeer();
    } catch (IPCException &e) {
      LOGE("WeexJSConnection catch: %s", e.msg());
      // TODO throw exception
      return nullptr;
    }
  }
  return m_impl->serverSender.get();
}

void WeexJSConnection::end() {
  m_impl->serverSender.reset();
  m_impl->futexPageQueue.reset();
  if (m_impl->child) {
    int wstatus;
    pid_t child;
    kill(m_impl->child, 9);
    while (true) {
      child = waitpid(m_impl->child, &wstatus, 0);
      if (child != -1)
        break;
      if (errno != EINTR)
        break;
    }
  }
}

IPCSender* WeexJSConnection::sender() {
  return m_impl->serverSender.get();
}

void printLogOnFile(const char *log) {
#if PRINT_LOG_CACHEFILE
  std::ofstream mcfile;
  mcfile.open(logFilePath, std::ios::app);
  mcfile << log << std::endl;
  mcfile.close();
#endif
}

static void findIcuDataPath(std::string &icuDataPath) {
  FILE *f = fopen("/proc/self/maps", "r");
  if (!f) {
    return;
  }
  fseek(f,0L,SEEK_END);
  int size=ftell(f);

    LOGE("file size is %d",size);
    struct stat statbuf;
    stat("/proc/self/maps",&statbuf);
    int size1=statbuf.st_size;
    LOGE("file size1 is %d",size1);
  char buffer[256];
  char *line;
  while ((line = fgets(buffer, 256, f))) {
    if (icuDataPath.empty() && strstr(line, "icudt")) {
      icuDataPath.assign(strstr(line, "/"));
      icuDataPath = icuDataPath.substr(0, icuDataPath.length() - 1);
    }

    if (!icuDataPath.empty()) {
      break;
    }
  }
  fclose(f);
  return;
}

class EnvPBuilder {
public:
    EnvPBuilder();

    ~EnvPBuilder() = default;

    void addNew(const char *n);

    std::unique_ptr<const char *[]> build();

private:
    std::vector<const char *> m_vec;
};

EnvPBuilder::EnvPBuilder() {
  for (char **env = environ; *env; env++) {
    // fixme:add for ANDROID_ROOT envp
    // if cannot find some env, can use such as
    // PATH/ANDROID_BOOTLOGO/ANDROID_ASSETS/ANDROID_DATA/ASEC_MOUNTPOINT
    // LOOP_MOUNTPOINT/BOOTCLASSPATH and etc
    // but don't use LD_LIBRARY_PATH env may cause so cannot be found
    const char *android_root_env = "ANDROID_ROOT=";
    if (strstr(*env, android_root_env) != nullptr) {
      addNew(*env);
      break;
    }
  }
}

void EnvPBuilder::addNew(const char *n) {
  m_vec.emplace_back(n);
}

std::unique_ptr<const char *[]> EnvPBuilder::build() {
  std::unique_ptr<const char *[]> ptr(new const char *[m_vec.size() + 1]);
  for (size_t i = 0; i < m_vec.size(); ++i) {
    ptr.get()[i] = m_vec[i];
  }
  ptr.get()[m_vec.size()] = nullptr;
  return ptr;
}

void doExec(int fdClient, int fdServer, bool traceEnable, bool startupPie) {
  std::string executablePath;
  std::string icuDataPath;
  findIcuDataPath(icuDataPath);
//  if(g_jssSoPath != nullptr) {
//    executablePath = g_jssSoPath;
  if(SoUtils::jss_so_path() != nullptr) {
    executablePath = SoUtils::jss_so_path();
  }
  if (executablePath.empty()) {
    executablePath = SoUtils::FindLibJssSoPath();
  }
#if PRINT_LOG_CACHEFILE
  std::ofstream mcfile;
  mcfile.open(logFilePath, std::ios::app);
  mcfile << "jsengine WeexJSConnection::doExec executablePath:" << executablePath << std::endl;
  mcfile << "jsengine WeexJSConnection::doExec icuDataPath:" << icuDataPath << std::endl;
#endif
  std::string::size_type pos = std::string::npos;
  std::string libName = SoUtils::jss_so_name();
  pos = executablePath.find(libName);
  if (pos != std::string::npos) {
    executablePath.replace(pos, libName.length(), "");

  if (executablePath.empty()) {
    LOGE("executablePath is empty");

#if PRINT_LOG_CACHEFILE
    mcfile << "jsengine WeexJSConnection::doExec executablePath is empty and return" << std::endl;
    mcfile.close();
#endif

    return;
  } else {
    LOGE("executablePath is %s", executablePath.c_str());
  }}
  if (icuDataPath.empty()) {
    LOGE("icuDataPath is empty");
#if PRINT_LOG_CACHEFILE
    mcfile << "jsengine WeexJSConnection::doExec icuDataPath is empty and return" << std::endl;
    mcfile.close();
#endif
    return;
  }
  std::string ldLibraryPathEnv("LD_LIBRARY_PATH=");
  std::string icuDataPathEnv("ICU_DATA_PATH=");
  std::string crashFilePathEnv("CRASH_FILE_PATH=");
  ldLibraryPathEnv.append(executablePath);
  icuDataPathEnv.append(icuDataPath);
#if PRINT_LOG_CACHEFILE
  mcfile << "jsengine ldLibraryPathEnv:" << ldLibraryPathEnv << " icuDataPathEnv:" << icuDataPathEnv
         << std::endl;
#endif
//  if (!s_cacheDir) {
  if (!SoUtils::cache_dir()) {
    crashFilePathEnv.append("/data/data/com.taobao.taobao/cache");
  } else {
    crashFilePathEnv.append(SoUtils::cache_dir());
  }
  crashFilePathEnv.append("/jsserver_crash");
  char fdStr[16];
  char fdServerStr[16];
  snprintf(fdStr, 16, "%d", fdClient);
  snprintf(fdServerStr, 16, "%d", fdServer);
  EnvPBuilder envpBuilder;
  envpBuilder.addNew(ldLibraryPathEnv.c_str());
  envpBuilder.addNew(icuDataPathEnv.c_str());
  envpBuilder.addNew(crashFilePathEnv.c_str());
  auto envp = envpBuilder.build();
  {
    std::string executableName = executablePath + '/' + "libweexjsb64.so";
    chmod(executableName.c_str(), 0755);
    const char *argv[] = {executableName.c_str(), fdStr, fdServerStr, traceEnable ? "1" : "0", nullptr};
    if (-1 == execve(argv[0], const_cast<char *const *>(&argv[0]),
                     const_cast<char *const *>(envp.get()))) {
    }
  }

  std::string start_so = "";
  if (startupPie) {
    start_so = "libweexjsb.so";
  } else {
    start_so = "libweexjst.so";
  }

  {
    std::string executableName = executablePath + '/' + start_so;
    chmod(executableName.c_str(), 0755);
    int result = access(executableName.c_str(), 01);

#if PRINT_LOG_CACHEFILE
    mcfile << "jsengine WeexJSConnection::doExec file exsist result:"
           << result << " startupPie:" << startupPie << std::endl;
#endif
    if (result == -1) {
      executableName = std::string(SoUtils::cache_dir()) + '/' + start_so;
      int result_cache = access(executableName.c_str(), 00);
      if (result_cache == -1) {
        std::string sourceSo = executablePath + '/' + start_so;
        int ret = copyFile(sourceSo.c_str(), executableName.c_str());
#if PRINT_LOG_CACHEFILE
        mcfile << "jsengine WeexJSConnection::doExec copy so from:" << sourceSo
               << " to:" << executableName << ", success: " << ret << std::endl;
#endif
      }
      chmod(executableName.c_str(), 0755);
#if PRINT_LOG_CACHEFILE
      mcfile << "jsengine WeexJSConnection::doExec start path on sdcard, start execve so name:"
             << executableName << std::endl;
#endif
      const char *argv[] = {executableName.c_str(), fdStr, fdServerStr, traceEnable ? "1" : "0", nullptr};
      if (-1 == execve(argv[0], const_cast<char *const *>(&argv[0]),
                       const_cast<char *const *>(envp.get()))) {
#if PRINT_LOG_CACHEFILE
        mcfile << "execve failed11:" << strerror(errno) << std::endl;
#endif
      }
    } else {
      // std::string executableName = executablePath + '/' + "libweexjsb.so";
      chmod(executableName.c_str(), 0755);
#if PRINT_LOG_CACHEFILE
      mcfile << "jsengine WeexJSConnection::doExec start execve so name:" << executableName
             << std::endl;
#endif
      const char *argv[] = {executableName.c_str(), fdStr, fdServerStr, traceEnable ? "1" : "0", nullptr};
      if (-1 == execve(argv[0], const_cast<char *const *>(&argv[0]),
                       const_cast<char *const *>(envp.get()))) {
#if PRINT_LOG_CACHEFILE
        mcfile << "execve failed:" << strerror(errno) << std::endl;
#endif
      }
    }

  }
#if PRINT_LOG_CACHEFILE
  mcfile.close();
#endif
}

static void closeAllButThis(int exceptfd, int fd2) {
  DIR *dir = opendir("/proc/self/fd");
  if (!dir) {
    return;
  }
  int dirFd = dirfd(dir);
  struct dirent *cur;
  struct timespec start;
  clock_gettime(CLOCK_MONOTONIC, &start);
  while ((cur = readdir(dir))) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if ((now.tv_sec - start.tv_sec) > 6) {
      break;
    }
    if (!strcmp(cur->d_name, ".")
        || !strcmp(cur->d_name, "..")) {
      continue;
    }
    errno = 0;
    unsigned long curFd = strtoul(cur->d_name, nullptr, 10);
    if (errno)
      continue;
    if (curFd <= 2)
      continue;
    if ((curFd != dirFd) && (curFd != exceptfd) && (curFd != fd2)) {
      close(curFd);
    }
  }
  closedir(dir);
}

int copyFile(const char *SourceFile, const char *NewFile) {
  std::ifstream in;
  std::ofstream out;
  in.open(SourceFile, std::ios::binary);
  if (in.fail()) {
    in.close();
    out.close();
    return 0;
  }
  out.open(NewFile, std::ios::binary);
  if (out.fail()) {
    out.close();
    in.close();
    return 0;
  } else {
    out << in.rdbuf();
    out.close();
    in.close();
    return 1;
  }
}
