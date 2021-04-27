#include <cstdio>
#include "connector.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "RWHandler.hpp"
#define _CRT_SECURE_NO_WARNINGS

int main() {
	int x;
	scanf("%d", &x);
	if (x == 1) {
		testforserver();
	}
	else {
		testforclient();
	}
	return 0;
 }