#include <vector>
#include "WANAgentClient.h"
#include "global/global_init.h"

#define dout_context cct
#define dout_subsys ceph_subsys_wana

int WANAgentClient::init() {
  assert(this->wana_messenger);

  lock.get_write();
  // stop if initialized.
  if (wana_con) {
    this->lock.unlock();
    return -1; // already initialized
  }

  // make a connection to local WANAgent
  this->wana_con = this->wana_messenger->get_connection(this->wana_inst);
  
  this->lock.unlock();
  return 0;
}

int WANAgentClient::forward(Message *m, uint32_t flags) {

  this->lock.get_read();
  if (!this->wana_con){
    this->lock.unlock();
    return -1; // not initialized.
  }
  this->lock.unlock();

  // we only forward CEPH_MSG_OSD_OP
  if (m->get_type() != CEPH_MSG_OSD_OP) {
    m->put();
    return -2; // invalid message type
  }
  // we only forward updates.
  MOSDOp* fm = static_cast<MOSDOp*>(m);
  bool is_update = false;
  std::vector<OSDOp>::const_iterator citr = fm->ops.cbegin();
  while(citr != fm->ops.cend()) {
    if(citr->op.op & CEPH_OSD_OP_MODE_WR){
      is_update = true;
      break;
    }
  }
  if (!is_update) {
    m->put();
    return -3; // no updates to be forwarded.
  }
  // forward it.
  // TODO: if connection broken, write to local buffer
  // and wait again.
  if (this->wana_con->send_message(fm)) {
    return -4; // send_message error.
  }

  // wait on message according to FLAGS?
  //TODO: wait on message.

  return 0;
}

int WANAgentClient::shutdown() {
}

WANAgentClient::~WANAgentClient() {
  if (this->wana_con) {
    this->wana_con->put();
  }
}

void WANAgentClient::ms_fast_dispatch(Message *m) {
  this->ms_dispatch(m);
}

bool WANAgentClient::ms_dispatch(Message *m) {
  // TODO: response to the messages
  return true;
}
