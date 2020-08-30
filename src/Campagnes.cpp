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

#include "Campagnes.h"
#include "Campagne.h"


vector<Campagne> Campagnes::mListeCampagnes;
mutex Campagnes::mAccesCampagnes;

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
	        		map<string,string> lIncludeFilter;
	        		map<string,string> lExcludeFilter;

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
	        		mAccesCampagnes.unlock();

	        		LOG_DEBUG << "CAMPAGNE INFO " << lCampagne.ToString();
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


const Campagne& Campagnes::SelectCampagne(BidRequest& pBidRequest)
{
	// TODO
	/*
	 * Vérification campagne
	 *
	Une campagne sera considérée comme compatible avec un emplacement publicitaire si :

	    La taille de l'emplacement publicitaire est compatible avec la taille de la publicité
	    Le budget restant de la campagne est supérieur ou égal au montant de l'enchère (bidPrice) à envoyer
	    L'emplacement publicitaire est compatible avec les filtres de la campagne
	*/

	/*
	 * Filtres
	 */

	//La vérification sera faite avec le champ device.lang de la bid-request. La comparaison doit être effectuée de manière insensible à la casse.

	//La vérification sera faite avec le champ app.name de la bid-request. La comparaison doit être effectuée sur un critère de type "contient".

	//La vérification sera faite avec le champ device.ifa de la bid-request. La comparaison doit être effectuée de manière insensible à la casse.

	/*
	 * Si plusieurs campagnes sont compatible avec un emplacement publicitaire, un tirage aléatoire sera effectué parmis elle.
	 *
	 *	Bonus : Pondérer le tirage aléatoire par un poids correspondant au budget disponible de l
	 *	a campagne (de telle manière qu'une campagne ayant un budget élevé aient plus de chance d'être diffusée).
	 */
}

int Campagnes::GetCampagnesCount()
{
	mAccesCampagnes.lock();
	int lTaille = mListeCampagnes.size();
	mAccesCampagnes.unlock();
	return lTaille;
}

map<string, string> Campagnes::GetFilterFromItr(Json::ValueIterator& pItr, string pFilterType)
{
	map<string, string> lMapFilters;

	// Préparation de la lambda à appliquer à chaque élément
	auto lLamdba = [&](string filterKey) {
		// Récupération de la liste des filtres
		auto lListe = (*pItr)["filters"][pFilterType][filterKey];
		// Pour chaque filtre, on itère sur les éléments qui le compose (ie les valeurs surlesquels filtrer)
		for_each(begin(lListe), end(lListe), [&](auto filterValue) {
			// Si l'élément est déjà présent dans la map on ajoute la valeur du filtre avec le séparateur /
			// TODO : tester avec un vecteur de string à la place
			if(lMapFilters.count(filterKey))
			{
				lMapFilters[filterKey] += "/"+filterValue.asString();
			}
			// Sinon on ajoute la clé
			else
			{
				lMapFilters.insert({filterKey, filterValue.asString()});
			}
			});
			LOG_DEBUG << "Inserting " << filterKey << " **** " << lMapFilters[filterKey];
		};

	// Récupération de la liste des filtres à prendre en compte
	auto lListeMembers = (*pItr)["filters"][pFilterType].getMemberNames();
	// Application de notre lamda sur chacun d'entre eux
	for_each(begin(lListeMembers), end(lListeMembers), lLamdba);

	return lMapFilters;
}
