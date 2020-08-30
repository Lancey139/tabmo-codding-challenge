#include <iostream>
#include <drogon/HttpAppFramework.h>
using namespace std;


int main(int argc, char **argv) {

	drogon::app().loadConfigFile("./res/config.drogon.json").run();

	return 0;
}

