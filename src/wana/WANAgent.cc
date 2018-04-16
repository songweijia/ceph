#include <signal.h>
#include <iostream>
#include "WANAgent.h"
#include "global/global_context.h"
#include "global/signal_handler.h"
#include "common/debug.h"

#define dout_context cct

void WANAgent::ms_fast_dispatch(Message *m) {
  this->ms_dispatch(m);
}

bool WANAgent::ms_dispatch(Message *m) {
  //TODO: handle messages
  std::cout << "WANAgent received message:" << *m << std::endl;
  return true;
}

void WANAgent::handle_signal(int signum) {
  assert(signum == SIGINT || signum == SIGTERM);
  derr << "*** got signal " << sig_str(signum) << " ***" << dendl;
  exit(0);
}

WANAgent::~WANAgent() {
  //TODO: destructor
}
