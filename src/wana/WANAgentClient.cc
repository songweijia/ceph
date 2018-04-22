#include <vector>
#include "WANAgentClient.h"
#include "global/global_init.h"

#define dout_context cct
#define dout_subsys ceph_subsys_wana

int WANAgentClient::init() {
  dout(10) << __func__ << "begins." << dendl;
  assert(this->wana_messenger);

  lock.get_write();
  // stop if initialized.
  // do nothing... so fa
  this->lock.unlock();
  dout(10) << __func__ << "done." << dendl;
  return 0;
}

int WANAgentClient::forward(Message *m, uint32_t flags) {

  dout(10) << __func__ << "begin with: m->get_type()=" << m->get_type() << ",flags=" << flags <<dendl;
  this->lock.get_read();
  dout(10) << __func__ << " acquired read lock." << flags <<dendl;
  // TODO: if connection broken, write to local buffer
  // and wait again.
  if (this->wana_messenger->send_message(m,this->wana_inst)) {
    dout(1) << __func__ << "forward message failed. return." << dendl;
    this->lock.unlock();
    return -4; // send_message error.
  }
  // wait on message according to FLAGS?
  //TODO: wait on message.
  this->lock.unlock();
  dout(10) << __func__ << "done." << flags <<dendl;
  return 0;
}

int WANAgentClient::shutdown() {
}

WANAgentClient::~WANAgentClient() {
}

void WANAgentClient::ms_fast_dispatch(Message *m) {
  this->ms_dispatch(m);
}

bool WANAgentClient::ms_dispatch(Message *m) {
  // TODO: response to the messages
  return true;
}
