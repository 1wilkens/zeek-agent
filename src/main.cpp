#include "configuration.h"
#include "logger.h"
#include "zeekagent.h"

#include <iostream>

#include <signal.h>

std::atomic_bool terminate{false};

void sigintHandler(int) { terminate = true; }

int main() {
  std::cout << "Zeek Agent v" << ZEEK_AGENT_VERSION << "\n";

  zeek::ZeekAgent::Ref zeek_agent;
  auto status = zeek::ZeekAgent::create(zeek_agent);
  if (!status.succeeded()) {
    std::cerr << "Initialization failed: " << status.message() << "\n";
    return 1;
  }

  struct sigaction signal_action {};
  signal_action.sa_handler = sigintHandler;
  if (sigaction(SIGINT, &signal_action, nullptr) != 0) {
    std::cerr << "Failed to initialize the SIGINT handler\n";
    return 1;
  }

  status = zeek::initializeConfiguration(zeek_agent->virtualDatabase());
  if (!status.succeeded()) {
    std::cerr << "Initialization failed: " << status.message() << "\n";
    return 1;
  }

  status = zeek::initializeLogger(zeek_agent->virtualDatabase());
  if (!status.succeeded()) {
    std::cerr << "Initialization failed: " << status.message() << "\n";
    return 1;
  }

  status = zeek_agent->exec(terminate);

  zeek::deinitializeConfiguration();
  zeek::deinitializeLogger();

  if (!status.succeeded()) {
    std::cerr << "An error has occurred: " << status.message() << "\n";
    return 1;
  }

  return 0;
}
