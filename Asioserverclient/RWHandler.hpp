#pragma once
#include <boost/asio.hpp>
#include <array>
#include <functional>
#include <algorithm>
#include <iostream>
#include "Message.hpp"
using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
const size_t MAX_IP_PACK_SIZE = 10000;
const size_t HEAD_LEN = 4;
class RWHandler :public std::enable_shared_from_this<RWHandler> {
public:
	RWHandler(io_service& ios) :m_sock(ios) {}
	~RWHandler() {}
	void handleRead() {
		auto self = shared_from_this();
		async_read(m_sock, buffer(m_readmsg.data(), HEAD_LEN), 
			[this, self](const boost::system::error_code& ec, size_t sz) {
			if (ec||!m_readmsg.decond_header()) {
				handleError(ec);
				return;
			}
			readBody();
		}
		);
	}
	void readBody() {
		auto self = shared_from_this();
		async_read(m_sock, buffer(m_readmsg.data(), m_readmsg.body_length()),
			[this, self] (const boost::system::error_code& ec,size_t sz){
			if (ec) {
				handleError(ec);
				return;
			}
			callback(m_readmsg.data(), m_readmsg.length());

			handleRead();
		}
		);
	}

	void handlewrite(char* data,size_t len) {
		boost::system::error_code ec;
		write(m_sock, buffer(data, len), ec);
		if (ec) {
			handleError(ec);
		}
	}
	void closeSocket() {
		boost::system::error_code ec;
		m_sock.shutdown(tcp::socket::shutdown_both, ec);
		m_sock.close(ec);
	}
	void setconnid(int id) {
		m_connid = id;
	}
	int getconnid() { return m_connid; }
	template<typename T>
	void setcallerror(T f) {
		m_callerror = f;
	}
	tcp::socket& getsocket() { return m_sock; }
	void callback(char* pdata, size_t l) {
		std::cout << pdata + HEAD_LEN << std::endl;
	}


private:
	void handleError(const boost::system::error_code& ec) {
		closeSocket();
		std::cout << ec.message() << std::endl;
		if (m_callerror) {
			m_callerror(m_connid);
		}
	}
private:
	tcp::socket m_sock;
	std::array<char, MAX_IP_PACK_SIZE> m_buff;
	int m_connid;
	std::function<void(int)> m_callerror;
	Message m_readmsg;

};