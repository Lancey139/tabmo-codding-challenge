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
	LOG_WARN << "TmListeCampagnes " << mListeCampagnes.size();
	vector<Json::Value> lCampagneSelectionnee;
	mAccesCampagnes.lock();
	for(vector<Campagne>::iterator itr = mListeCampagnes.begin(); itr != mListeCampagnes.end(); itr++ )
	{
			/*
			 * Vérification de la compatibilité de la campagne
			 */
			if(pBidRequest.mDeviceH == itr->mHeight &&
					pBidRequest.mDeviceW == itr->mWidth &&
					itr->mBudget >= itr->mBidPrice)
			{
				/*
				 * Vérification des filtres
				 */
				bool lFiltreCompatible = true;
				/*
				 * LANGUES
				 */
				// On commence par passer la langue dans la requete en minuscule
				for_each(pBidRequest.mDeviceLang.begin(), pBidRequest.mDeviceLang.end(), [](char& c){c = tolower(c);});

				auto lLambdaLang = [&pBidRequest](string filterValue){
							return pBidRequest.mDeviceLang.compare(filterValue) == 0;
						};
				// Pas de vérification sur mDeviceLang ici car déjà vérifier à l'arriver de la requete
				if( itr->mFilterInclude.count("language"))
					lFiltreCompatible =  count_if(itr->mFilterInclude.at("language").begin(), itr->mFilterInclude.at("language").end(), lLambdaLang) != 0;

				if( lFiltreCompatible && itr->mFilterExlude.count("language"))
					lFiltreCompatible =  count_if(itr->mFilterExlude.at("language").begin(), itr->mFilterExlude.at("language").end(), lLambdaLang) == 0;

				/*
				 * Nom de l'application
				 */
				auto lLambdaApp = [&pBidRequest](string filterValue){
							return pBidRequest.mAppName.find(filterValue) != string::npos;
						};
				// Pas de vérification sur mDeviceLang ici car déjà vérifier à l'arriver de la requete
				if( lFiltreCompatible && itr->mFilterInclude.count("application"))
					lFiltreCompatible =  count_if(itr->mFilterInclude.at("application").begin(), itr->mFilterInclude.at("application").end(), lLambdaApp) != 0;

				if( lFiltreCompatible && itr->mFilterExlude.count("application"))
					lFiltreCompatible =  count_if(itr->mFilterExlude.at("application").begin(), itr->mFilterExlude.at("application").end(), lLambdaApp) == 0;

				/*
				 * IFA
				 */
				// On commence par passer la langue dans la requete en minuscule
				for_each(pBidRequest.mDeviceIfa.begin(), pBidRequest.mDeviceIfa.end(), [](char& c){c = tolower(c);});

				auto lLambdaIFA = [&pBidRequest](string filterValue){
							return pBidRequest.mDeviceIfa.compare(filterValue) == 0;
						};
				// Pas de vérification sur mDeviceLang ici car déjà vérifier à l'arriver de la requete
				if( lFiltreCompatible && itr->mFilterInclude.count("ifa"))
					lFiltreCompatible =  count_if(itr->mFilterInclude.at("ifa").begin(), itr->mFilterInclude.at("ifa").end(), lLambdaIFA) != 0;

				if( lFiltreCompatible && itr->mFilterExlude.count("ifa"))
					lFiltreCompatible =  count_if(itr->mFilterExlude.at("ifa").begin(), itr->mFilterExlude.at("ifa").end(), lLambdaIFA) == 0;

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
					lCampagne["price"] = itr->mBidPrice;

					lCampagneSelectionnee.push_back(lCampagne);
				}

			}
	}
	mAccesCampagnes.unlock();
	/*
	 * La liste des campagne compatible a été établie, on finalise le choix
	 */
	// TODO
	/*
	 * On retire l'argent de la campagne qui a payé l'emplacement
	 */
	// TODO

	Json::Value ret;
	return ret;
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
