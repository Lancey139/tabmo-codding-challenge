/*
 * RtbCtrl.h
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#pragma once
#include <drogon/HttpSimpleController.h>
using namespace drogon;


class RtbCtrl : public HttpSimpleController<RtbCtrl>
{
  public:
	/*
	 * Méthode déclenchée sur réception d'une request http sur l'url /rtb.
	 * Cette méthode est susceptible d'être appelée simultanément par plusieurs threads afin d'optimiser les performances
	 * Toutes les variables utilisées doivent être thread safe
	 */
	virtual void asyncHandleHttpRequest(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback) override;

	PATH_LIST_BEGIN
	PATH_ADD("/rtb", Get);
	PATH_LIST_END
};

