#pragma once
#include <algorithm>
class Message {
public:
	enum { header_length = 4 };
	enum { max_body_lenght = 512 };

	Message() :body_length_(0) {}
	const char* data()const { return data_; }
	char* data() { return data_; }
	size_t length()const { return header_length + body_length_; }
	const char* body()const { return data_ + header_length; }
	char* body() { return data_ + header_length; }
	size_t body_length()const { return body_length_; }
	void body_length(size_t new_length) {
		body_length_ = std::min<size_t>(new_length, max_body_lenght);
	}
	bool decond_header() {
		char header[header_length + 1] = "";
		std::strncat(header, data_, header_length);
		body_length_ = std::atoi(header) - header_length;
		if (body_length_ > max_body_lenght) {
			body_length_ = 0;
			return false;
		}
		return true;
	}
	void encode_header() {
		char header[header_length + 1] = "";
		std::sprintf(header, "%4d", body_length_);
		std::memcpy(data_, header, header_length);
	}

private:
	char data_[header_length + max_body_lenght];
	size_t body_length_;
};