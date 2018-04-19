#ifndef CEPH_WAN_AGENT_CLIENT_H
#define CEPH_WAN_AGENT_CLIENT_H

#include <arpa/inet.h>
#include <string>
#include <inttypes.h>
#include "msg/msg_types.h"
#include "msg/Connection.h"
#include "messages/MOSDOp.h"
#include "msg/Messenger.h"
#include "common/RWLock.h"
class MOSDOp;
class Connection;
class RWLock;

class WANAgentClient : public Dispatcher {
private:
  CephContext *cct;

public:
  /**
   * constructor
   */
  WANAgentClient(CephContext *cct_, Messenger *ms_wan_agent) : 
    Dispatcher(cct_),
    cct(cct_),
    wana_messenger(ms_wan_agent),
    lock("wanaclient"),
    wana_con(nullptr) {
    // setup wana instance
    // TODO: the configuration is from the configuration file
    // those needs to be managed by monitor in the future
    std::string near_wana_addr = cct_->_conf->get_val<std::string>("local_dc_wana");
    std::istringstream f(near_wana_addr);
    std::string tk;

    this->wana_inst.name._type = entity_name_t::TYPE_WANA;
    this->wana_inst.name._num = 0; // always 0 for near wana?
    this->wana_inst.addr.type = entity_addr_t::TYPE_DEFAULT;
    this->wana_inst.addr.nonce = getpid();
    struct sockaddr_in *sin = (struct sockaddr_in*)&this->wana_inst.addr.u;
    sin->sin_family = AF_INET;
    assert(std::getline(f,tk,':'));
    inet_pton(AF_INET,tk.c_str(),&sin->sin_addr);
    assert(std::getline(f,tk,':'));
    sin->sin_port = htons((uint16_t)atoi(tk.c_str()));
  }
  /**
   * destructor
   */
  virtual ~WANAgentClient();
  /**
   * initialize
   *
    * @RETURNVALUE 0 for success, other for error
   */
  int init();
  /**
   * start
   */
  int shutdown();
  /**
   * forward a message to remote
   */
  #define RETURN_ON_MASK (0xf)
  #define RETURN_IMMEDIATELY (0x0)
  #define RETURN_ON_FULLY_COMMITED (0xf)
  int forward(Message *m,uint32_t flags);
  /**
   * forward a message to remote asynchronously
   * TODO:
   */

  // dispatcher methods
  bool ms_can_fast_dispatch_any() const override { return true; }
  bool ms_can_fast_dispatch(const Message *m) const override { return false;}
  void ms_fast_dispatch(Message *m) override;
  void ms_fast_preprocess(Message *m) override {}
  bool ms_dispatch(Message *m) override;
  void ms_handle_connect(Connection *con) override {}
  void ms_handle_fast_connect(Connection *con) override {}
  void ms_handle_accept(Connection *con) override {}
  void ms_handle_fast_accept(Connection *con) override {}
  bool ms_handle_reset(Connection *con) override {return false;}
  void ms_handle_remote_reset(Connection *con) override {}
  bool ms_handle_refused(Connection *con) override {return false;}
  bool ms_get_authorizer(int dest_type, AuthAuthorizer **a, bool force_new) override { return false;}
  bool ms_verify_authorizer(Connection *con,
                            int peer_type,
                            int protocol,
                            ceph::bufferlist& authorizer,
                            ceph::bufferlist& authorizer_reply,
                            bool& isvalid,
                            CryptoKey& session_key) override { return false; }

  // start
  // 
private:
  Messenger *wana_messenger;
  RWLock lock;
  ConnectionRef wana_con;
  struct entity_inst_t wana_inst;
};

#endif//CEPH_WAN_AGENT_CLIENT_H
