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
  if (wana_con) {
    this->lock.unlock();
    return -1; // already initialized
  }

  // make a connection to local WANAgent
  this->wana_con = this->wana_messenger->get_connection(this->wana_inst);
  
  this->lock.unlock();
  dout(10) << __func__ << "done." << dendl;
  return 0;
}

int WANAgentClient::forward(Message *m, uint32_t flags) {

  dout(10) << __func__ << "begin with: m->get_type()=" << m->get_type() << ",flags=" << flags <<dendl;
  this->lock.get_read();
  dout(10) << __func__ << " acquired read lock." << flags <<dendl;
  if (!this->wana_con){
    this->lock.unlock();
    return -1; // not initialized.
  }
  this->lock.unlock();

  // we only forward CEPH_MSG_OSD_OP
  if (m->get_type() != CEPH_MSG_OSD_OP) {
    derr << __func__ << " message type(" << m->get_type() << 
      ") other than CEPH_MSG_OSD_OP cannot be forwarded." << dendl;
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
    dout(10) << __func__ << "no updates, skip it." << dendl;
    return -3; // no updates to be forwarded.
  }

  // copy the message and forward it.
  // TODO: check if it possible avoiding copy.
  bufferlist payload,middle,data;
  m->get_payload().copy(0,m->get_payload().length(),payload);
  m->get_middle().copy(0,m->get_middle().length(),middle);
  m->get_data().copy(0,m->get_data().length(),data);
  MOSDOp *copied_fm = static_cast<MOSDOp*>(
    decode_message(this->cct, m->get_connection()->msgr->crcflags, 
      m->get_header(),
      m->get_footer(),
      payload,
      middle,
      data,
      this->wana_con.get()));
  

  // TODO: if connection broken, write to local buffer
  // and wait again.
  if (this->wana_con->send_message(copied_fm)) {
    dout(1) << __func__ << "forward message failed. return." << dendl;
    return -4; // send_message error.
  }
  // wait on message according to FLAGS?
  //TODO: wait on message.
  dout(10) << __func__ << "done." << flags <<dendl;

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
