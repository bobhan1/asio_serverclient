#pragma once
#include "Message.hpp"
#include "RWHandler.hpp"
#include <boost/asio.hpp>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <numeric>

const int MAXCONNCTION = 65536;
const int MAXRECVSIZE = 65536;
class Server {
public:
	Server(io_service& ios_,short port):ios(ios_),m_ac(ios,tcp::endpoint(tcp::v4(),port)),
		 pool(MAXCONNCTION){
		pool.resize(MAXCONNCTION);
		std::iota(pool.begin(), pool.end(), 1);
	}
	~Server() {}
	void accept() {
		std::cout << "Start listening..." << std::endl;
		std::shared_ptr<RWHandler> handler = createhandler();
		m_ac.async_accept(handler->getsocket(), [this, handler](const boost::system::error_code& ec) {
			if (ec) {
				std::cout << ec.value() << " " << ec.message() << std::endl;
				handleacperror(handler, ec);
				return;
			}
			m_handlers.insert(std::make_pair(handler->getconnid(), handler));
			std::cout << "current connect count :" << m_handlers.size() << std::endl;
			handler->handleRead();
			accept();
		});
	}

private:
	void handleacperror(std::shared_ptr<RWHandler> eventhandler, const boost::system::error_code& ec) {
		std::cout << "Error , error reason: " << ec.value() << ec.message() << std::endl;
		eventhandler->closeSocket();
		stopaccept();
	}
	void stopaccept() {
		boost::system::error_code ec;
		m_ac.cancel(ec);
		m_ac.close(ec);
		ios.stop();
	}
	std::shared_ptr<RWHandler> createhandler() {
		int id = pool.front();
		pool.pop_front();
		std::shared_ptr<RWHandler> p = std::make_shared<RWHandler>(ios);
		p->setconnid(id);
		p->setcallerror([this](int id) {
			recycleid(id);
		});
	}
	void recycleid(int id){
		auto it = find(pool.begin(),pool.end(),id);
		if (it != pool.end())pool.erase(it);
		std::cout << "current connect count :" << m_handlers.size() << std::endl;
		pool.push_back(id);
	}

private:
	io_service& ios;
	tcp::acceptor m_ac;
	std::unordered_map<int, std::shared_ptr<RWHandler>> m_handlers;
	std::list<int> pool;
};

void testforserver() {
	io_service io;
	Server s(io, 9900);
	s.accept();
	io.run();
}