/*------------------------------------------------------------------------------
                Copyright Butterfly Energy Systems 2014-2015.
           Distributed under the Boost Software License, Version 1.0.
              (See accompanying file LICENSE_1_0.txt or copy at
                    http://www.boost.org/LICENSE_1_0.txt)
------------------------------------------------------------------------------*/

#ifndef CPPWAMP_INTERNAL_TCPOPENER_HPP
#define CPPWAMP_INTERNAL_TCPOPENER_HPP

#include <cassert>
#include <memory>
#include <string>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "../asiodefs.hpp"
#include "../tcphost.hpp"

namespace wamp
{

namespace internal
{

//------------------------------------------------------------------------------
class TcpOpener
{
public:
    using Info      = TcpHost;
    using Socket    = boost::asio::ip::tcp::socket;
    using SocketPtr = std::unique_ptr<Socket>;

    TcpOpener(AsioService& iosvc, Info info)
        : iosvc_(iosvc),
          info_(std::move(info))
    {}

    AsioService& iosvc() {return iosvc_;}

    template <typename TCallback>
    void establish(TCallback&& callback)
    {
        assert(!resolver_ && "Connect already in progress");

        resolver_.reset(new tcp::resolver(iosvc_));

        tcp::resolver::query query(info_.hostName(), info_.serviceName());

        // AsioConnector will keep this object alive until completion.
        resolver_->async_resolve(query,
            [this, callback](AsioErrorCode ec, tcp::resolver::iterator iterator)
            {
                if (ec)
                {
                    resolver_.reset();
                    callback(ec, nullptr);
                }
                else
                    connect(iterator, std::move(callback));
            });
    }

    void cancel()
    {
        if (resolver_)
            resolver_->cancel();
        if (socket_)
            socket_->close();
    }

private:
    using tcp = boost::asio::ip::tcp;

    template <typename TCallback>
    void connect(tcp::resolver::iterator iterator, TCallback&& callback)
    {
        assert(!socket_);
        socket_.reset(new Socket(iosvc_));
        socket_->open(boost::asio::ip::tcp::v4());
        internal::applyRawsockOptions(info_, *socket_);

        // AsioConnector will keep this object alive until completion.
        boost::asio::async_connect(*socket_, iterator,
            [this, callback](AsioErrorCode ec, tcp::resolver::iterator)
            {
                resolver_.reset();
                if (ec)
                    socket_.reset();
                callback(ec, std::move(socket_));
                socket_.reset();
            });
    }

    AsioService& iosvc_;
    Info info_;
    std::unique_ptr<tcp::resolver> resolver_;
    SocketPtr socket_;
};

} // namespace internal

} // namespace wamp

#endif // CPPWAMP_INTERNAL_TCPOPENER_HPP
