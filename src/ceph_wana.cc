#include <sys/types.h>
#include <iostream>
#include <string>
#include <arpa/inet.h>

#include "wana/WANAgent.h"
#include "common/config.h"
#include "msg/Messenger.h"
#include "global/global_init.h"
#include "global/signal_handler.h"
#include "include/assert.h"

WANAgent *wana = nullptr;

void handle_wana_signal(int signum) {
  if(wana)
    wana->handle_signal(signum);
}

static void usage()
{
  std::cout << "usage: ceph-wana" << std::endl;
}

int main(int argc, const char **argv) {
  int r;
  std::string msgr_type = g_conf->get_val<std::string>("ms_type");
  Messenger *ms_near = Messenger::create(g_ceph_context, msgr_type,
    entity_name_t::WANA(), "wana", getpid(), Messenger::HAS_HEAVY_TRAFFIC | Messenger::HAS_MANY_CONNECTIONS );
  assert(ms_near);
  ms_near->set_cluster_protocol(CEPH_WANA_PROTOCOL);
  cout << "starting wana at " << ms_near->get_myaddr() << "." << std::endl;
  ms_near->set_policy(entity_name_t::TYPE_WANA,Messenger::Policy::stateless_server(0));

  // TODO: make this nicer by using configuration tools.
  std::string near_wana_addr = g_conf->get_val<std::string>("wana near");
  std::istringstream f(near_wana_addr);
  std::string tk;
  struct entity_addr_t bind_addr;
  struct sockaddr_in *sin = (struct sockaddr_in*)&bind_addr.u;
  bind_addr.type = entity_addr_t::TYPE_DEFAULT;
  bind_addr.nonce = getpid();
  sin->sin_family = AF_INET;
  assert(std::getline(f,tk,':'));
  inet_pton(AF_INET,tk.c_str(),&sin->sin_addr);
  assert(std::getline(f,tk,':'));
  sin->sin_port = htons((uint16_t)atoi(tk.c_str()));  
  r = ms_near->bind(bind_addr);

  // TODO: add those features.  
  WANAgent *wana = new WANAgent(g_ceph_context,ms_near,nullptr,nullptr,nullptr);
  ms_near->start();
  ms_near->wait();

  init_async_signal_handler();
  register_async_signal_handler(SIGHUP, sighup_handler);
  register_async_signal_handler_oneshot(SIGINT, handle_wana_signal);
  register_async_signal_handler_oneshot(SIGTERM, handle_wana_signal);

  if (g_conf->inject_early_sigterm)
    kill(getpid(), SIGTERM);

  ms_near->wait();

  unregister_async_signal_handler(SIGHUP, sighup_handler);
  unregister_async_signal_handler(SIGINT, handle_wana_signal);
  unregister_async_signal_handler(SIGTERM, handle_wana_signal);
  shutdown_async_signal_handler();

  delete wana;
  return 0;
}
