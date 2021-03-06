#ifndef CERT_TRANS_SERVER_HANDLER_H_
#define CERT_TRANS_SERVER_HANDLER_H_

#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>

#include "proto/ct.pb.h"
#include "util/libevent_wrapper.h"
#include "util/sync_task.h"
#include "util/task.h"

class Frontend;

namespace cert_trans {

class CertChain;
class CertChecker;
template <class T>
class ClusterStateController;
class LogLookup;
class LoggedEntry;
class PreCertChain;
class Proxy;
class ReadOnlyDatabase;
class ThreadPool;


class HttpHandler {
 public:
  // Does not take ownership of its parameters, which must outlive
  // this instance.
  HttpHandler(LogLookup* log_lookup, const ReadOnlyDatabase* db,
              const ClusterStateController<LoggedEntry>* controller,
              ThreadPool* pool, libevent::Base* event_base);
  virtual ~HttpHandler();

  void Add(libevent::HttpServer* server);

  void SetProxy(Proxy* proxy);

 protected:
  // Implemented by subclasses which want to add their own extra http handlers.
  virtual void AddHandlers(libevent::HttpServer* server) = 0;

  void AddEntryReply(evhttp_request* req, const util::Status& add_status,
                     const ct::SignedCertificateTimestamp& sct) const;

  void ProxyInterceptor(
      const libevent::HttpServer::HandlerCallback& local_handler,
      evhttp_request* request);

  void AddProxyWrappedHandler(
      libevent::HttpServer* server, const std::string& path,
      const libevent::HttpServer::HandlerCallback& local_handler);

  void GetEntries(evhttp_request* req) const;
  void GetProof(evhttp_request* req) const;
  void GetSTH(evhttp_request* req) const;
  void GetConsistency(evhttp_request* req) const;

  void BlockingGetEntries(evhttp_request* req, int64_t start, int64_t end,
                          bool include_scts) const;

  bool IsNodeStale() const;
  void UpdateNodeStaleness();

  LogLookup* const log_lookup_;
  const ReadOnlyDatabase* const db_;
  const ClusterStateController<LoggedEntry>* const controller_;
  Proxy* proxy_;
  ThreadPool* const pool_;
  libevent::Base* const event_base_;

  util::SyncTask task_;
  mutable std::mutex mutex_;
  bool node_is_stale_;

  DISALLOW_COPY_AND_ASSIGN(HttpHandler);
};


}  // namespace cert_trans

#endif  // CERT_TRANS_SERVER_HANDLER_H_
