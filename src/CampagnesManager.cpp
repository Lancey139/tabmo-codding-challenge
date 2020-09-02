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
	    			LOG_ERROR << "La campagne " << (*itr)["id"].asString() << " n'a pas pu être ajoutée : JSON invalide";
	    			LOG_ERROR << JsonExcept.what();
	    			lReturn = false;
	    		}
	        	// Cas ou une mauvaise valeur a été entrée dans l'un des floats
	    		// ou une des valeurs obligatoire n'est pas renseignée dans la campagne
	    		catch(invalid_argument &invalid)
	    		{
	    			LOG_ERROR << "La campagne " << (*itr)["id"].asString() << " n'a pas pu être ajoutée";
	    			LOG_ERROR << invalid.what();
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
	vector<Campagne*> lCampagnesSelectionnees;
	Json::Value lJsonReturn;
	for(vector<Campagne>::iterator itr = mListeCampagnes.begin(); itr != mListeCampagnes.end(); itr++ )
	{
		// Lock avec auto unlock quand on sort du scope
		lock_guard<mutex> lock(itr->GetMutex());

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
			// Unlock pour laisser le constructeur par copie opérer
			itr->GetMutex().unlock();
			lCampagnesSelectionnees.push_back(itr->GetPointeur());
		}
	}

	/*
	 * La liste des campagne compatible a été établie, on finalise le choix
	 */

	// Si une campagne a été trouvée
	if(lCampagnesSelectionnees.size() > 0)
	{
		// Tirage de la campagne random si il y en a plusieurs
		Campagne* lCampagne = RandomSelectCampagne(lCampagnesSelectionnees);
		LOG_DEBUG << "Campagne selectionnée !! \n" << lCampagne->ToString();
		// On retire l'argent de la campagne qui a payé l'emplacement et on créé le JSON
		// On lock la campagne avant de la modifer
		lock_guard<mutex> lockCampagne(lCampagne->GetMutex());
		if(!lCampagne->DecrementeBudget())
		{
			throw logic_error("Erreur critique, une campagne a été sélectionnée mais son budget est insuffisant La requete est ignorée");
		}
		lJsonReturn = CreerJsonFromCampagne(lCampagne, pBidRequest);
	}

	return lJsonReturn;
}

Campagne* CampagnesManager::RandomSelectCampagne(vector<Campagne*>& pCampagnesSelectionnees)
{
	/*
	 * Afin de réaliser l'objectif bonus, la stratégie suivante est appliquée :
	 * On calcule en pourcentage le budget des campagnes sélectionées. Il représentera
	 * les chances d'être choisi. On tire un nombre entre 0 et 100 puis on choisit la campagne
	 * en fonction de ce nombre
	*/
	if (pCampagnesSelectionnees.size() == 1)
	{
		return pCampagnesSelectionnees[0];
	}

	// TODO Les impacts sur les performances sont à mesurer

	// Creation d'un seeder
	random_device lSeeder;
	// Le mersenne twister engine est en charge grace au seeder de générer le nombre aléatoire
	// La séquence ne se reproduira qu'apres 2 puissance 19937 donc tres bonne disbution
	mt19937 lGenerateur(lSeeder());
	uniform_int_distribution<int> lDistribution(0, 100);
	float lRandom = (float)lDistribution(lGenerateur);

	/*
	 * Calcul de la pondération en % de chaque campagne
	 */
	vector<float> lVecteurPonderation;
	int lSommeBudget = 0;
	for_each(pCampagnesSelectionnees.begin(), pCampagnesSelectionnees.end(), [&lVecteurPonderation, &lSommeBudget](Campagne* pCampagne){
		// On lock la camapagne avant d'y acceder
		lock_guard<mutex> lock(pCampagne->GetMutex());
		lVecteurPonderation.push_back((float)pCampagne->GetBudget());
		lSommeBudget += pCampagne->GetBudget();
	});

	// On multiplie par 100 puis divise par budget total pour obtenir les %
	int lIndexSelectionne = 0;
	float lProbaBrecedente = 0.;
	for(int i=0; i < lVecteurPonderation.size(); i++)
	{
		lVecteurPonderation[i] = (lVecteurPonderation[i] / (float)lSommeBudget) * 100 ;
		// Chaque campagne a une de [ lProbaBrecedente ; lProbaBrecedente + lVecteurPonderation[i] ] de chance d'être tirée
		if(lRandom >= lProbaBrecedente && lRandom <= lProbaBrecedente + lVecteurPonderation[i])
		{
			// La campagne est choisie
			lIndexSelectionne = i;
			break;
		}
		lProbaBrecedente += lVecteurPonderation[i];
	}

	return pCampagnesSelectionnees[lIndexSelectionne];
}

int CampagnesManager::GetCampagnesCount()
{
	mAccesCampagnes.lock();
	int lTaille = mListeCampagnes.size();
	mAccesCampagnes.unlock();
	return lTaille;
}

Json::Value CampagnesManager::CreerJsonFromCampagne(Campagne* pCampagne, BidRequest& pBidRequest)
{
	Json::Value lCampagne;
	lCampagne["auctionId"] = pBidRequest.mId;
	lCampagne["campaignId"] = pCampagne->GetId();
	// le prix est entre arrondi a 2 chiffres
	stringstream stream;
	stream << std::fixed << setprecision(2) << pCampagne->GetBidPrice();
	lCampagne["price"] = stream.str();
	lCampagne["url"] = pCampagne->GetUrl();
	return lCampagne;
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
