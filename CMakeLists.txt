cmake_minimum_required (VERSION 2.6)

project (tabmo-coding-challenge)

add_subdirectory(third-party/drogon)

add_executable(tabmo-coding-challenge 
	src/tabmo-coding-challenge.cpp
	src/RtbCtrl.cpp
	src/BidRequest.cpp
	src/Campagnes)
	
file(COPY res DESTINATION ./)

target_link_libraries(tabmo-coding-challenge PRIVATE drogon)

