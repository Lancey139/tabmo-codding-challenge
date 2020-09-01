/*
 * Campagnes.cpp
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */
#include "CampagnesManager.h"

#include <drogon/drogon.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdio.h>
#include <random>
#include <math.h>
#include <iomanip>
#include <thread>

#include "Campagne.h"

using namespace std;

vector<Campagne> CampagnesManager::mListeCampagnes;
mutex CampagnesManager::mAccesCampagnes;
vector<mutex> CampagnesManager::mListeMutex;


bool CampagnesManager::ParseCampagnesFromJson(string pCheminVersJson)
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

	        		// En théorie a ce stade de l'execution 1 seul thread travaille
	        		// On prend tout de meme le lock par précaution
	        		mAccesCampagnes.lock();
	        		mListeCampagnes.push_back(lCampagne);
	        		mAccesCampagnes.unlock();
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


Json::Value CampagnesManager::SelectCampagne(BidRequest& pBidRequest)
{
	/*
	 * Afin que cette méthode reste ThreadSafe mais qu'elle soit également perfomante
	 * J'ai choisi d'intégrer un mutex a mes campagnes : on itère sur la liste des campagnes mais
	 * on ne lock que la campagne en cours d'itération
	 */
	Json::Value lReturn;
	vector<Json::Value> lCampagneSelectionnee;

	for(vector<Campagne>::iterator itr = mListeCampagnes.begin(); itr != mListeCampagnes.end(); itr++ )
	{
		itr->GetMutex().lock();

		bool lFiltreCompatible = true;
		/*
		 * Vérifiation de la taille
		 */
		lFiltreCompatible = pBidRequest.mDeviceH == itr->GetHeight() && pBidRequest.mDeviceW == itr->GetWidth();

		// En cas de non compatibilité, on vérifie si la campagne est responsive
		if(!lFiltreCompatible && itr->GetResponsive())
		{
			// Dans ce cas les ratio doivent correspondre (static cast des int)
			// pas de vérification dénominateur != 0 car toutes les vérifis ont été faites en amont
			lFiltreCompatible = (float)pBidRequest.mDeviceH / (float)pBidRequest.mDeviceW == (float)itr->GetHeight() / (float)itr->GetWidth();
		}

		/*
		 * Vérification du budget
		 * Le budget restant de la campagne doit être supérieur au prix
		 * et le bid-price doit être supérieur au bid-floor
		 */
		if(lFiltreCompatible)
			lFiltreCompatible = itr->GetBudget() >= itr->GetBidPrice() && itr->GetBidPrice() >= pBidRequest.mBidFloor;

		/*
		 * Vérification des filtres
		 */

		/*
		 * LANGUES
		 */
		if(lFiltreCompatible)
		{
			// On commence par passer la langue dans la requete en minuscule
			for_each(pBidRequest.mDeviceLang.begin(), pBidRequest.mDeviceLang.end(), [](char& c){c = tolower(c);});

			auto lLambdaLang = [&pBidRequest](string filterValue){
						return pBidRequest.mDeviceLang.compare(filterValue) == 0;
					};
			// Pas de vérification sur mDeviceLang ici car déjà vérifier à l'arriver de la requete
			if( itr->GetFilterInclude().count("language") > 0)
				lFiltreCompatible =  find_if(itr->GetFilterInclude().at("language").begin(), itr->GetFilterInclude().at("language").end(), lLambdaLang) !=
						itr->GetFilterInclude().at("language").end();

			if( lFiltreCompatible && itr->GetFilterExclude().count("language") > 0)
				lFiltreCompatible =  find_if(itr->GetFilterExclude().at("language").begin(), itr->GetFilterExclude().at("language").end(), lLambdaLang) ==
						itr->GetFilterExclude().at("language").end();
		}
		/*
		 * Nom de l'application
		 */
		if(lFiltreCompatible)
		{
			auto lLambdaApp = [&pBidRequest](string filterValue){
						return pBidRequest.mAppName.find(filterValue) != string::npos;
					};
			if( lFiltreCompatible && itr->GetFilterInclude().count("application") > 0)
			{
				lFiltreCompatible = !pBidRequest.mAppName.empty() &&
						find_if(itr->GetFilterInclude().at("application").begin(), itr->GetFilterInclude().at("application").end(), lLambdaApp) !=
						itr->GetFilterInclude().at("application").end();
			}

			if( lFiltreCompatible && itr->GetFilterExclude().count("application") > 0)
			{
				lFiltreCompatible =  !pBidRequest.mAppName.empty() &&
						find_if(itr->GetFilterExclude().at("application").begin(), itr->GetFilterExclude().at("application").end(), lLambdaApp) ==
						itr->GetFilterExclude().at("application").end();
			}
		}

		/*
		 * IFA
		 */
		if(lFiltreCompatible)
		{
			for_each(pBidRequest.mDeviceIfa.begin(), pBidRequest.mDeviceIfa.end(), [](char& c){c = tolower(c);});

			auto lLambdaIFA = [&pBidRequest](string filterValue){
						return pBidRequest.mDeviceIfa.compare(filterValue) == 0;
					};

			if( lFiltreCompatible && itr->GetFilterInclude().count("ifa") > 0)
			{
				lFiltreCompatible =  !pBidRequest.mDeviceIfa.empty() &&
						find_if(itr->GetFilterInclude().at("ifa").begin(), itr->GetFilterInclude().at("ifa").end(), lLambdaIFA) !=
						itr->GetFilterInclude().at("ifa").end();
			}

			if( lFiltreCompatible && itr->GetFilterExclude().count("ifa") > 0)
			{
				lFiltreCompatible =  !pBidRequest.mDeviceIfa.empty() &&
						find_if(itr->GetFilterExclude().at("ifa").begin(), itr->GetFilterExclude().at("ifa").end(), lLambdaIFA) ==
						itr->GetFilterExclude().at("ifa").end();
			}
		}
		/*
		 * Fin de la vérification de la campagne, ajout au vecteur de campagne compatible
		 */
		if(lFiltreCompatible)
		{
			LOG_DEBUG << "Campagne compatible trouvée " << itr->GetId();
			Json::Value lCampagne;
			lCampagne["auctionId"] = pBidRequest.mId;
			lCampagne["campaignId"] = itr->GetId();
			lCampagne["url"] = itr->GetUrl();
			// le prix est entre arrondi a 2 chiffres
			stringstream stream;
			stream << std::fixed << setprecision(2) << itr->GetBidPrice();
			lCampagne["price"] = stream.str();

			// TODO vérifier si JSON Value est movable sinon stocker autrement
			lCampagneSelectionnee.push_back(lCampagne);
		}

		itr->GetMutex().unlock();
	}

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
			return lReturn["campaignId"].asString().compare(pCampagne.GetId()) == 0;
		});
		if (lIterator != mListeCampagnes.end())
		{
			LOG_DEBUG << "Campagne selectionnée !! " << lIterator->ToString();
			if(!lIterator->DecrementeBudget())
			{
				LOG_ERROR << "Erreur critique, une campagne a été sélectionnée mais son budget est insuffisant La requete est ignorée";
				lReturn = Json::Value();
			}
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

int CampagnesManager::GetCampagnesCount()
{
	mAccesCampagnes.lock();
	int lTaille = mListeCampagnes.size();
	mAccesCampagnes.unlock();
	return lTaille;
}

map<string, vector<string>> CampagnesManager::GetFilterFromItr(Json::ValueIterator& pItr, string pFilterType)
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
