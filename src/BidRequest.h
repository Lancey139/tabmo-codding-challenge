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

class BidRequest2
{
	public:
		// Constructeur
		BidRequest2(string pId,
				double pBidFloor,
				int pDeviceW,
				int pDeviceH,
				string pDeviceIfa,
				string pDeviceLang,
				string pAppName);

		BidRequest2();

		virtual ~BidRequest2();

		/*
		 * Accesseurs et setters vers nos données
		 * Accesseurs return par référence pour éviter une copie et optimiser les performances
		 */
		 const string& GetId() const  { return mId; }
		 const double& GetBidFloor() const  { return mBidFloor; }
		 const int& GetDeviceW() const  { return mDeviceW; }
		 const int& GetDeviceH() const  { return mDeviceH; }
		 const string& GetDeviceIfa() const  { return mDeviceIfa; }
		 const string& GetDeviceLang() const  { return mDeviceLang; }
		 const string& GetAppName() const  { return mAppName; }

	private:
		/*
		 * Attributs contenant les valeurs parsées dont nous avons besoin dans ce programme
		 * depuis le JSON obtenu via la requete
		 */

		// Attributs decrivant la requete
		string mId;
		double mBidFloor;

		// Attributs decrivant le device
		int mDeviceW;
		int mDeviceH;
		string mDeviceIfa;
		string mDeviceLang;

		// Attribut decriavant l application
		string mAppName;
};

