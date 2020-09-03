/*
 * Campagnes.h
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#pragma once
#include <vector>
#include <string>
#include <list>
#include <mutex>

#include "BidRequest.h"
#include "Campagne.h"

using namespace std;

/*
 * Classe Campagnes contenant toutes les méthodes visant à manipuler les campagnes
 */
class CampagnesManager {
	public:
		CampagnesManager();
		virtual ~CampagnesManager();

		/*
		 * Methode permettant de remplir le vecteur de campagne avec un JSON
		 * @param pCheminVersJson : Contient le chemin vers le fichier contenant les campagnes
		 * @return True si le parsing a eu lieu sans erreur, false sinon
		 */
		static bool ParseCampagnesFromJson(string pCheminVersJson);

		/*
		 * Methode permettant a partir d'une BidRequest de determiner la campagne la plus adaptee
		 * @param pBidRequest : La bid request obtenue à partir du JSON de la requete / Passée par référence pour éviter une copie
		 * @return : Json::Value représentant la campagne retenue
		 */
		static Json::Value  SelectCampagne(BidRequest& pBidRequest);

		/*
		 * Methode permettant d'obtenir le nombre de campagne parsée
		 * @return : Nombre de campagne parsée
		 */
		static int GetCampagnesCount();

	private:
		/*
		 * Methode permettant de realiser la selection finale de la campagne.
		 * Si plusieurs campagnes sont compatibles, l'une d'entre elle est tirée au sort.
		 * Le tirage au sort est pondéré par le budget restant de la campagne
		 * @param pCampagnesSelectionnees : Vecteur des campagnes compatibles
		 * @ return Campagne* : pointeur vers la campagne sélectionnée
		 */
		static Campagne* RandomSelectCampagne(vector<Campagne*>& pCampagnesSelectionnees);

		/*
		 * Methode permettant de créer le Json de retour a partir d'une campagne
		 * @param pCampagne : campagne à converttir en Json
		 * @param pBidRequest : requete en cours
		 * @ return Json::Value : la campagne sélectionnée
		 */
		static Json::Value CreerJsonFromCampagne(Campagne* pCampagne, BidRequest& pBidRequest);

		// L'attribut statique mListeCampagnes permet de stocker l'ensemble des campagnes parsées dans le JSON
		static vector<Campagne> mListeCampagnes;
		// L'attribut mAccesCampagnes permet de sécuriser l'accès à mListeCampagne en cas de modification du vecteur
		static mutex mAccesCampagnes;
		// L'attribut mListeMustex permet de sécuriser l'accès à une campagne en particulier : il y en a autant de campagnes
		static vector<mutex> mListeMutex;


		static map<string, vector<string>> GetFilterFromItr(Json::ValueIterator& pItr, string pFilterType);
};

