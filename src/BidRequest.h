/*
 * BidRequest.h
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#pragma once

#include <string>
#include <drogon/drogon.h>
using namespace std;

struct BidRequest
{
	// Attributs decrivant la requete
	string mId;
	float mBidFloor;

	// Attributs decrivant le device
	int mDeviceW;
	int mDeviceH;
	string mDeviceIfa;
	string mDeviceLang;

	// Attribut decriavant l application
	string mAppName;
};

