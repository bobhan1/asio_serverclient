#pragma once
#include <boost/asio.hpp>
#include <algorithm>
#include <thread>
#include <string>
#include <chrono>
#include "RWHandler.hpp"
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace boost;
#define _CRT_SECURE_NO_WARNINGS
class Connector {
public:
	Connector(io_service& ios_, const std::string& ip, short port) :ios(ios_),m_sock(ios_),
		m_serveraddr(tcp::endpoint(address::from_string(ip), port)), is_connected(false), m_ck(nullptr) {
		createhandler();
	}
	~Connector() {}
	bool start() {
		m_eventhandler->getsocket().async_connect(m_serveraddr, [this](const boost::system::error_code& ec) {
			if (ec) {
				handleConnecterror(ec);
				return false;
			}
			std::cout << "connect ok" << std::endl;
			is_connected = true;
			m_eventhandler->handleread();
		});
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return is_connected;
	}
	bool isConnected()const { return is_connected; }
	void send(char* data, size_t l) {
		if (!is_connected) {
			return;
		}
		m_eventhandler->handlewrite(data,l);
	}
private:
	void createhandler() {
		m_eventhandler = std::make_shared<RWHandler>(ios);
		m_eventhandler->setcallerror([this](int id) {handleRWerror(id); });
	}

	void checkConnect() {
		if (m_ck) {
			return;
		}
		m_ck = std::make_shared<std::thread>([this] {
			while (1) {
				if (!isConnected())start();
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		});
	}
	void handleConnecterror(const boost::system::error_code& ec) {
		is_connected = false;
		std::cout << ec.message() << std::endl;
		m_eventhandler->closeSocket();
		checkConnect();
	}
	void handleRWerror(int id) {
		is_connected = false;
		checkConnect();
	}

private:
	io_service& ios;
	tcp::socket m_sock;
	tcp::endpoint m_serveraddr;
	std::shared_ptr<RWHandler> m_eventhandler;
	bool is_connected;
	std::shared_ptr<std::thread> m_ck;
};

void testforclient() {
	io_service io;
	boost::asio::io_service::work wk(io);
	std::thread t1([&io] {io.run(); });
	Connector c1(io, "127.0.0.1", 9900);
	c1.start();
	std::string str;
	if (!c1.isConnected()) {
		return;
	}

	const size_t l = 512;
	char line[l] = "";
	while (std::cin >> str) {
		char header[HEAD_LEN] = "";
		size_t total = str.size() +1 + HEAD_LEN;
		std::sprintf(header, "%4d", total);
		memcpy(line, header, HEAD_LEN);
		memcpy(line + HEAD_LEN, str.c_str(), str.size() + 1);
		c1.send(line, total);
	}
}