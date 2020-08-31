/*
 * Campagnes.cpp
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */
#include <drogon/drogon.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <random>
#include <math.h>
#include <iomanip>

#include "Campagnes.h"
#include "Campagne.h"

using namespace std;

vector<Campagne> Campagnes::mListeCampagnes;
mutex Campagnes::mAccesCampagnes;
vector<mutex> Campagnes::mListeMutex;

Campagnes::Campagnes() {
	// TODO Auto-generated constructor stub

}

Campagnes::~Campagnes() {
	// TODO Auto-generated destructor stub
}


bool Campagnes::ParseCampagnesFromJson(string pCheminVersJson)
{
	Json::Reader lJsonReader;
	Json::Value lJsonValue;
	std::ifstream lIfStream(pCheminVersJson);

	bool lReturn = true;


	if(lJsonReader.parse(lIfStream, lJsonValue))
	{
		// Récupération des éléments de la campagne
	    if( lJsonValue.size() > 0 )
	    {
	        for( Json::ValueIterator itr = lJsonValue.begin() ; itr != lJsonValue.end() ; itr++ )
	        {
	        	/*
	        	 * Les vérifications de la précense ou non des éléments obligatoires dans le JSON sont traitées dans le constructeur
	        	 */
	        	try
	        	{
	        		// Avant de créer la campagne on pré-charge les vecteurs de filtre grace a la méthode privée associée
	        		// Cette facon de procéder implique de copier la map des valeurs 2 fois (une par le return + une lors de la
	        		// construction de l'objet.
	        		// Ce code n'étant executer qu'une seule fois au démarage, ce problème est dépriorisé
	        		map<string,vector<string>> lIncludeFilter;
	        		map<string,vector<string>> lExcludeFilter;

					if(!(*itr)["filters"]["include"].empty())
					{
						lIncludeFilter = GetFilterFromItr(itr, "include");
					}
					if(!(*itr)["filters"]["exclude"].empty())
					{
						lExcludeFilter = GetFilterFromItr(itr, "exclude");
					}

	        		Campagne lCampagne((*itr)["id"].asString(),
	        				stof((*itr)["budget"].asString()),
							stof((*itr)["bidPrice"].asString()),
							(*itr)["width"].asInt(),
							(*itr)["height"].asInt(),
							(*itr)["responsive"].asBool(),
							lIncludeFilter,
							lExcludeFilter,
							(*itr)["url"].asString());

	        		// En thérorie a ce stade de l'execution 1 seul thread travaille
	        		// On prend tout de meme le lock par précaution
	        		mAccesCampagnes.lock();
	        		mListeCampagnes.push_back(lCampagne);
	        		// TODO
	        		//mutex lMutex;
	        		//mListeMutex.push_back(lMutex);
	        		mAccesCampagnes.unlock();

	        		//LOG_DEBUG << "CAMPAGNE INFO " << lCampagne.ToString();
	        	}
	        	// Cas ou une mauvaise valeur a été entrée dans le JSON
	    		catch(Json::LogicError &JsonExcept)
	    		{
	    			LOG_WARN << "La campagne " << (*itr)["id"].asString() << "n'a pas pu être ajoutée : JSON invalide";
	    			LOG_WARN << JsonExcept.what();
	    			lReturn = false;
	    		}
	        	// Cas ou une mauvaise valeur a été entrée dans l'un des floats
	    		// ou une des valeurs obligatoire n'est pas renseignée dans la campagne
	    		catch(invalid_argument &invalid)
	    		{
	    			LOG_WARN << "La campagne " << (*itr)["id"].asString() << "n'a pas pu être ajoutée pour l'une des raison suivante";
	    			LOG_WARN << " - L'une des valeurs obligatoires n'est pas renseignée";
	    			LOG_WARN << " - Les valeurs budget ou bidprice sont invalides";
	    			LOG_WARN << invalid.what();
	    			lReturn = false;
	    		}
	        }
	    }
	    else
	    {
	    	LOG_WARN << "Aucune campagne trouvée";
		    lReturn = false;
	    }

	}
	else
	{
		LOG_WARN << " Impossible de parser le fichier contenant les campagnes";
		lReturn = false;
	}
	return lReturn;
}


Json::Value Campagnes::SelectCampagne(BidRequest& pBidRequest)
{
	/*
	 * Afin que cette méthode reste ThreadSafe mais qu'elle soit également perfomante
	 * J'ai choisi d'utiliser un tableau de mutex : on itère sur la liste des campagnes mais
	 * on ne lock que la campagne en cours d'itération
	 * Il n'est pas nécessaire de locker pour la première boucle for car on ne va pas dans cette méthode
	 * Réallouer des ressources. Ainsi on peut se permettre de ne locker que les items Campagne
	 * TODO
	 */
	Json::Value lReturn;
	vector<Json::Value> lCampagneSelectionnee;
	mAccesCampagnes.lock();
	for(vector<Campagne>::iterator itr = mListeCampagnes.begin(); itr != mListeCampagnes.end(); itr++ )
	{
		/*
		 * Vérification de la compatibilité de la campagne
		 */
		bool lFiltreCompatible = true;
		/*
		 * Vérifiation de la taille
		 */
		lFiltreCompatible = pBidRequest.mDeviceH == itr->mHeight && pBidRequest.mDeviceW == itr->mWidth;

		// En cas de non compatibilité, on vérifie si la campagne est responsive
		if(!lFiltreCompatible && itr->mResponsive)
		{
			// Dans ce cas les ratio doivent correspondre (static cast des int)
			// pas de vérification dénominateur != 0 car toutes les vérifis ont été faites en amont
			lFiltreCompatible = (float)pBidRequest.mDeviceH / (float)pBidRequest.mDeviceW == (float)itr->mHeight / (float)itr->mWidth;
		}

		/*
		 * Vérification du budget
		 * Le budget restant de la campagne doit être supérieur au prix
		 * et le bid-price doit être supérieur au bid-floor
		 */
		if(lFiltreCompatible)
			lFiltreCompatible = itr->mBudget >= itr->mBidPrice && itr->mBidPrice >= pBidRequest.mBidFloor;

		/*
		 * Vérification des filtres
		 */

		/*
		 * LANGUES
		 */
		// On commence par passer la langue dans la requete en minuscule
		if(lFiltreCompatible)
		{
			for_each(pBidRequest.mDeviceLang.begin(), pBidRequest.mDeviceLang.end(), [](char& c){c = tolower(c);});

			auto lLambdaLang = [&pBidRequest](string filterValue){
						return pBidRequest.mDeviceLang.compare(filterValue) == 0;
					};
			// Pas de vérification sur mDeviceLang ici car déjà vérifier à l'arriver de la requete
			if( itr->mFilterInclude.count("language") > 0)
				lFiltreCompatible =  find_if(itr->mFilterInclude.at("language").begin(), itr->mFilterInclude.at("language").end(), lLambdaLang) !=
						itr->mFilterInclude.at("language").end();

			if( lFiltreCompatible && itr->mFilterExlude.count("language") > 0)
				lFiltreCompatible =  find_if(itr->mFilterExlude.at("language").begin(), itr->mFilterExlude.at("language").end(), lLambdaLang) ==
						itr->mFilterExlude.at("language").end();
		}
		/*
		 * Nom de l'application
		 */
		if(lFiltreCompatible)
		{
			auto lLambdaApp = [&pBidRequest](string filterValue){
						return pBidRequest.mAppName.find(filterValue) != string::npos;
					};
			// Vérification de la précense du app name

			if( lFiltreCompatible && itr->mFilterInclude.count("application") > 0)
			{
				lFiltreCompatible = !pBidRequest.mAppName.empty() &&
						find_if(itr->mFilterInclude.at("application").begin(), itr->mFilterInclude.at("application").end(), lLambdaApp) !=
						itr->mFilterInclude.at("application").end();
			}

			if( lFiltreCompatible && itr->mFilterExlude.count("application") > 0)
			{
				lFiltreCompatible =  !pBidRequest.mAppName.empty() &&
						find_if(itr->mFilterExlude.at("application").begin(), itr->mFilterExlude.at("application").end(), lLambdaApp) ==
						itr->mFilterExlude.at("application").end();
			}
		}

		/*
		 * IFA
		 */
		// On commence par passer la langue dans la requete en minuscule
		if(lFiltreCompatible)
		{
			for_each(pBidRequest.mDeviceIfa.begin(), pBidRequest.mDeviceIfa.end(), [](char& c){c = tolower(c);});

			auto lLambdaIFA = [&pBidRequest](string filterValue){
						return pBidRequest.mDeviceIfa.compare(filterValue) == 0;
					};

			if( lFiltreCompatible && itr->mFilterInclude.count("ifa") > 0)
			{
				lFiltreCompatible =  !pBidRequest.mDeviceIfa.empty() &&
						find_if(itr->mFilterInclude.at("ifa").begin(), itr->mFilterInclude.at("ifa").end(), lLambdaIFA) !=
						itr->mFilterInclude.at("ifa").end();
			}

			if( lFiltreCompatible && itr->mFilterExlude.count("ifa") > 0)
			{
				lFiltreCompatible =  !pBidRequest.mDeviceIfa.empty() &&
						find_if(itr->mFilterExlude.at("ifa").begin(), itr->mFilterExlude.at("ifa").end(), lLambdaIFA) ==
						itr->mFilterExlude.at("ifa").end();
			}
		}
		/*
		 * Fin de la vérification de la campagne, ajout au vecteur de campagne compatible
		 */
		if(lFiltreCompatible)
		{
			LOG_WARN << "Campagne compatible trouvée " << itr->mId;
			Json::Value lCampagne;
			lCampagne["auctionId"] = pBidRequest.mId;
			lCampagne["campaignId"] = itr->mId;
			lCampagne["url"] = itr->mUrl;
			// le prix est entree arrondi a 2 chiffres
			stringstream stream;
			stream << std::fixed << setprecision(2) << itr->mBidPrice;
			lCampagne["price"] = stream.str();

			// TODO vérifier si JSON Value est movable sinon stocker autrement
			lCampagneSelectionnee.push_back(lCampagne);
		}

	}
	mAccesCampagnes.unlock();
	/*
	 * La liste des campagne compatible a été établie, on finalise le choix
	 */
	// Tirage de la campagne random si il y en a plusieurs
	if(lCampagneSelectionnee.size() > 1)
	{
		// TODO Les impacts sur les performances sont à mesurer
		// Creation d'un seeder
		random_device lSeeder;
		// Le mersenne twister engine est en charge grace au seeder de générer le nombre aléatoire
		// La séquence ne se reproduira qu'apres 2 puissance 19937 donc tres bonne disbution
		mt19937 lGenerateur(lSeeder());
		uniform_int_distribution<int> lDistribution(0, lCampagneSelectionnee.size()-1);
		int random = lDistribution(lGenerateur);

		lReturn = lCampagneSelectionnee[random];
	}
	else if (lCampagneSelectionnee.size() == 1)
	{
		lReturn = lCampagneSelectionnee[0];
	}

	// TODO Bonus
	/*
	 * On retire l'argent de la campagne qui a payé l'emplacement
	 */
	// Si une campagne a été trouvée
	if(lReturn.size() > 0)
	{
		mAccesCampagnes.lock();
		vector<Campagne>::iterator lIterator = find_if(mListeCampagnes.begin(), mListeCampagnes.end(), [&lReturn](Campagne& pCampagne){
			return lReturn["campaignId"].asString().compare(pCampagne.mId) == 0;
		});
		if (lIterator != mListeCampagnes.end())
		{
			lIterator->mBudget -= lIterator->mBidPrice;
			LOG_WARN << "Campagne selectionnée !! " << lIterator->ToString();
		}
		else
		{
			LOG_ERROR << "Erreur critique, une campagne a été sélectionnée mais son budget n'a pas pu être mis à jour. La requete est ignorée";
			lReturn = Json::Value();

		}
		mAccesCampagnes.unlock();
	}

	return lReturn;
}

int Campagnes::GetCampagnesCount()
{
	mAccesCampagnes.lock();
	int lTaille = mListeCampagnes.size();
	mAccesCampagnes.unlock();
	return lTaille;
}

map<string, vector<string>> Campagnes::GetFilterFromItr(Json::ValueIterator& pItr, string pFilterType)
{
	map<string, vector<string>> lMapFilters;

	// Préparation de la lambda à appliquer à chaque élément
	auto lLamdba = [&](string filterKey) {
		// Récupération de la liste des filtres
		auto lListe = (*pItr)["filters"][pFilterType][filterKey];
		// Pour chaque filtre, on itère sur les éléments qui le compose (ie les valeurs surlesquels filtrer)
		for_each(begin(lListe), end(lListe), [&](auto filterValue) {
			/*
			 * Afin d'optimiser les performances sur le code exécuter à la requete, ces champs
			 * sont stockés en minuscule
			 */
			string lStringValue = filterValue.asString();
			if(filterKey.compare("language") == 0 || filterKey.compare("ifa") == 0)
			{
				for_each(lStringValue.begin(), lStringValue.end(), [](char& c){c = tolower(c);});
			}

			lMapFilters[filterKey].push_back(lStringValue);
			});
		};

	// Récupération de la liste des filtres à prendre en compte
	auto lListeMembers = (*pItr)["filters"][pFilterType].getMemberNames();
	// Application de notre lamda sur chacun d'entre eux
	for_each(begin(lListeMembers), end(lListeMembers), lLamdba);

	return lMapFilters;
}
