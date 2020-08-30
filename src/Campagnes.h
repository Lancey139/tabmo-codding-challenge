/*
 * Campagnes.h
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#pragma once
#include <vector>
#include <string>
#include <mutex>

#include "BidRequest.h"
#include "Campagne.h"

using namespace std;

/*
 * Classe Campagnes contenant toutes les méthodes visant à manipuler les campagnes
 */
class Campagnes {
	public:
		Campagnes();
		virtual ~Campagnes();

		/*
		 * Methode permettant de remplir le vecteur de campagne avec un JSON
		 * @param pCheminVersJson : Contient le chemin vers le fichier contenant les campagnes
		 * @return True si le parsing a eu lieu sans erreur, false sinon
		 */
		static bool ParseCampagnesFromJson(string pCheminVersJson);

		/*
		 * Methode permettant a partir d'une BidRequest de determiner la campagne la plus adaptee
		 * @param pBidRequest : La bid request obtenue à partir du JSON de la requete / Passée par référence pour éviter une copie
		 * @return : Référence constante vers la campagne retenue
		 */
		static const Campagne& SelectCampagne(BidRequest& pBidRequest);

		/*
		 * Methode permettant d'obtenir le nombre de campagne parsée
		 * @return : Nombre de campagne parsée
		 */
		static int GetCampagnesCount();

	private:
		static vector<Campagne> mListeCampagnes;
		static mutex mAccesCampagnes;

		static map<string, string> GetFilterFromItr(Json::ValueIterator& pItr, string pFilterType);
};

