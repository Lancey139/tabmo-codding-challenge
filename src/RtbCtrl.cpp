/*
 * RtbCtrl.cpp
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */
#include <drogon/drogon.h>

#include "RtbCtrl.h"
#include "BidRequest.h"
#include "Campagnes.h"

using namespace std;

void RtbCtrl::asyncHandleHttpRequest(const HttpRequestPtr &req,
									  std::function<void(const HttpResponsePtr &)> &&callback)
{

	shared_ptr<Json::Value> lJsonData = req->getJsonObject();
	if(lJsonData)
	{
		/*
		 * Récupération des données
		 */
		// Création de la BidRequest
		BidRequest lBidRequest;

		// Récupération des éléments de type string (Pas d'exeption à catch)
		lBidRequest.mId = (*lJsonData)["id"].asString();
		lBidRequest.mDeviceLang = (*lJsonData)["device"]["lang"].asString();
		lBidRequest.mAppName = (*lJsonData)["app"]["name"].asString();
		lBidRequest.mDeviceIfa = (*lJsonData)["device"]["ifa"].asString();
		try
		{
			lBidRequest.mDeviceW = (*lJsonData)["device"]["w"].asInt();
			lBidRequest.mDeviceH = (*lJsonData)["device"]["h"].asInt();
			lBidRequest.mBidFloor = stof((*lJsonData)["bid-floor"].asString());
		}
		catch(Json::LogicError &JsonExcept)
		{
			LOG_WARN << JsonExcept.what();
		}
		catch(exception &e)
		{
			LOG_WARN << e.what();
		}

		/*
		 * Controle des données critiques:
		 * - Id ne peut pas être null ou vide
		 * - mDeviceLang ne peut pas être null ou vide
		 * - BidFloor DeviceW et H doivent être différent de 0 -> Json inscrit 0 si le noeud est inexistant
		 */
		if(!lBidRequest.mId.empty() && !lBidRequest.mDeviceLang.empty() && lBidRequest.mBidFloor != 0 && lBidRequest.mDeviceW != 0 && lBidRequest.mDeviceH !=0)
		{
			LOG_DEBUG << "Données valides reçues" << lBidRequest.mId;
			// Sélection de la campagne
			Campagnes::SelectCampagne(lBidRequest);
		}
		else
		{
			LOG_WARN << "L'une des données obligatoires est invalide, la requete est ignorée" << lBidRequest.mId << lBidRequest.mBidFloor << lBidRequest.mDeviceW << lBidRequest.mDeviceH;
		}
	}
	else
	{
		LOG_WARN << "Le payload de la requete n'est pas de type JSON, la requete est ignorée";
	}

	Json::Value ret;
	ret["message"] = "Hello, World!";
	auto resp = HttpResponse::newHttpJsonResponse(ret);
	callback(resp);
}
