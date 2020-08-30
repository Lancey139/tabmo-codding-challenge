/*
 * RtbCtrl.cpp
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#include "RtbCtrl.h"
#include <drogon/drogon.h>

using namespace std;

void RtbCtrl::asyncHandleHttpRequest(const HttpRequestPtr &req,
									  std::function<void(const HttpResponsePtr &)> &&callback)
{
	LOG_DEBUG << "Reception d'une nouvelle requete" << req->getBody();
	shared_ptr<Json::Value> lJsonData = req->getJsonObject();
	if(lJsonData)
	{
		string lId = (*lJsonData)["id"].asString();
		string lLang = (*lJsonData)["device"]["lang"].asString();
		string lExistepas = (*lJsonData)["device"]["test"].asString();
		// Check if null or empty
		lExistepas.empty();
		LOG_DEBUG <<  lId << " "<< lLang << " lExistepas" << lExistepas;
	}
	else
	{
		LOG_DEBUG << "Le payload de la requete n'est pas de type JSON, la requete est ignorée";
	}

	Json::Value ret;
	ret["message"] = "Hello, World!";
	auto resp = HttpResponse::newHttpJsonResponse(ret);
	callback(resp);
}
