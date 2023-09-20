#ifndef ZHENGQI_CPP_NETWORKING_EXAMPLES_SOCKS4A_TUNNEL_H
#define ZHENGQI_CPP_NETWORKING_EXAMPLES_SOCKS4A_TUNNEL_H

#include "networking/Callbacks.h"
#include "networking/TcpClient.h"
#include "utility/Logging.h"
#include "utility/noncopyable.h"
#include <functional>
#include <memory>
class Tunnel : public std::enable_shared_from_this<Tunnel>,
               zhengqi::utility::noncopyable {
public:
  Tunnel(zhengqi::networking::EventLoop *loop,
         const zhengqi::networking::InetAddress &serverAddr,
         const zhengqi::networking::TcpConnectionPtr &serverConn)
      : client_(loop, serverAddr, serverConn->name()), serverConn_(serverConn) {
    LOG_INFO << "Tunnel " << serverConn->peerAddress().toIpPort() << " <-> "
             << serverAddr.toIpPort();
  }

  ~Tunnel() { LOG_INFO << "~Tunnel"; }

  void setup() {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    client_.setConnectionCallback(
        std::bind(&Tunnel::onClientConnection, shared_from_this(), _1));
    client_.setMessageCallback(
        std::bind(&Tunnel::onClientMessage, shared_from_this(), _1, _2, _3));
    serverConn_->setHighWaterMarkCallback(
        std::bind(&Tunnel::onHighWaterMarkWeak,
                  std::weak_ptr<Tunnel>(shared_from_this()), kServer, _1, _2),
        1024 * 1024);
  }

  void connect() { client_.connect(); }
  void disconnect() { client_.disconnect(); }

private:
  void teardown() {
    client_.setConnectionCallback(
        zhengqi::networking::defaultConnectionCallback);
    client_.setMessageCallback(zhengqi::networking::defaultMessageCallback);
    if (serverConn_) {
      serverConn_->setContext(boost::any());
      serverConn_->shutdown();
    }
    clientConn_.reset();
  }

  void onClientConnection(const zhengqi::networking::TcpConnectionPtr &conn) {
    using std::placeholders::_1;
    using std::placeholders::_2;

    LOG_DEBUG << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
      conn->setTcpNoDelay(true);
      conn->setHighWaterMarkCallback(
          std::bind(&Tunnel::onHighWaterMarkWeak,
                    std::weak_ptr<Tunnel>(shared_from_this()), kClient, _1, _2),
          1024 * 1024);

      assert(serverConn_ != nullptr);
      serverConn_->setContext(conn);
      serverConn_->startRead();
      clientConn_ = conn;
      if (serverConn_->inputBuffer()->readableBytes() > 0) {
        conn->send(serverConn_->inputBuffer());
      }
    } else {
      teardown();
    }
  }

  void onClientMessage(const zhengqi::networking::TcpConnectionPtr &conn,
                       zhengqi::networking::Buffer *buf,
                       zhengqi::utility::Timestamp) {
    LOG_DEBUG << conn->name() << " " << buf->readableBytes();
    if (serverConn_) {
      serverConn_->send(buf);
    } else {
      buf->retrieveAll();
      abort();
    }
  }

  enum ServerClient { kServer, kClient };

  void onHighWaterMark(ServerClient which,
                       const zhengqi::networking::TcpConnectionPtr &conn,
                       size_t bytesToSent) {
    using std::placeholders::_1;
    LOG_INFO << (which == kServer ? "server" : "client") << "onHighWaterMark"
             << conn->name() << " bytes " << bytesToSent;

    if (which == kServer) {
      if (serverConn_->outputBuffer()->readableBytes() > 0) {
        clientConn_->stopRead();
        serverConn_->setWriteCompleteCallback(
            std::bind(&Tunnel::onWriteCompleteWeak,
                      std::weak_ptr<Tunnel>(shared_from_this()), kServer, _1));
      }
    } else {
      if (clientConn_->outputBuffer()->readableBytes() > 0) {
        serverConn_->stopRead();
        clientConn_->setWriteCompleteCallback(
            std::bind(&Tunnel::onWriteCompleteWeak,
                      std::weak_ptr<Tunnel>(shared_from_this()), kClient, _1));
      }
    }
  }

  static void
  onHighWaterMarkWeak(const std::weak_ptr<Tunnel> &wkTunnel, ServerClient which,
                      const zhengqi::networking::TcpConnectionPtr &conn,
                      size_t bytesToSent) {
    std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
    if (tunnel) {
      tunnel->onHighWaterMark(which, conn, bytesToSent);
    }
  }

  void onWriteComplete(ServerClient which,
                       const zhengqi::networking::TcpConnectionPtr &conn) {
    LOG_INFO << (which == kServer ? "server" : "client") << " onWriteComplete "
             << conn->name();
    if (which == kServer) {
      clientConn_->startRead();
      serverConn_->setWriteCompleteCallback(
          zhengqi::networking::WriteCompleteCallback());
    } else {
      serverConn_->startRead();
      clientConn_->setWriteCompleteCallback(
          zhengqi::networking::WriteCompleteCallback());
    }
  }

  static void
  onWriteCompleteWeak(const std::weak_ptr<Tunnel> &wkTunnel, ServerClient which,
                      const zhengqi::networking::TcpConnectionPtr &conn) {
    std::shared_ptr<Tunnel> tunnel = wkTunnel.lock();
    if (tunnel) {
      tunnel->onWriteComplete(which, conn);
    }
  }

private:
  zhengqi::networking::TcpClient client_;
  zhengqi::networking::TcpConnectionPtr serverConn_;
  zhengqi::networking::TcpConnectionPtr clientConn_;
};

typedef std::shared_ptr<Tunnel> TunnelPtr;

#endif // !DEBUG
