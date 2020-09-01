/*
 * Campagnes.h
 *
 *  Created on: 01 septembre 2020
 *      Author: Nicolas MEO
 */
#include <algorithm>
#include <drogon/drogon.h>

#include "Campagne.h"

using namespace std;

Campagne::Campagne(string pId, float pBudget, float pBidPrice, int pWidth, int pHeight,
		bool pResponsive, map<string,vector<string>> pFilterInclude,
		map<string,vector<string>> pFilterExclude, string pUrl):
			mId(pId), mBudget(pBudget), mBidPrice(pBidPrice), mWidth(pWidth), mHeight(pHeight),
			mResponsive(pResponsive), mFilterInclude(pFilterInclude), mFilterExlude(pFilterExclude), mUrl(pUrl)
{
	/*
	 * Le choix a été fait de réaliser l"ensemble des controles d'initialisation de la campagne dans le constructeur
	 * La raison est d'éviter de créer des setters : en effet, on ne souhaite pas que ces éléments soient modifiés dans la suite
	 * du programme.
	 */

	// On vérifie que les valeurs requises sont différentes de la valeur donnée par défaut par le parseur JSON en cas
	// de non précense d'une des balises obligatoires
	if(pId.empty() ||
		pBudget == 0 ||
		pBidPrice == 0 ||
		pWidth == 0 ||
		pHeight == 0 ||
		pUrl.empty()
		)
	{
		throw invalid_argument("L'une des données obligatoires de la campagne est invalide, la requete est ignorée");
	}

	/*
	 * Vérification de l'unicité des filtres
	 * ie si un filtre est dans include il n'est pas dans exclude
	 */
	map<string,vector<string>>::iterator lFind = find_if(mFilterInclude.begin(), mFilterInclude.end(), [&](pair<string,vector<string>> pPairInclude){
		// On parcourt le filter Exclude, si l'une des clés est égale à celle du pair include on return true
		return find_if(mFilterExlude.begin(), mFilterExlude.end(), [&pPairInclude](pair<string,vector<string>> pPairExclude){
			return pPairInclude.first.compare(pPairExclude.first) == 0;
		}) != mFilterExlude.end();
	});

	if(lFind != mFilterInclude.end())
	{
		throw invalid_argument("L'un des filtres est présent à la fois dans le include et le exclude");
	}


	/*
	 * Vérification du nombres d'éléments dans les filtres
	 */
	auto lVerificationFiltre = [](pair<string,vector<string>> pPair) {
		bool lIsValidFilter = false;
		if(pPair.first.compare("language") == 0)
		{
			// La langue peut contenir 1 à 10 éléments
			lIsValidFilter = pPair.second.size() >= 1 && pPair.second.size() <= 10;
		}
		else if(pPair.first.compare("application") == 0)
		{
			// L'application peut contenir 1 à 100 éléments
			lIsValidFilter = pPair.second.size() >= 1 && pPair.second.size() <= 100;
		}
		else if(pPair.first.compare("ifa") == 0)
		{
			// L'application peut contenir 1 à 100 000 éléments
			lIsValidFilter = pPair.second.size() >= 1 && pPair.second.size() <= 100000;
		}
		else
		{
			lIsValidFilter = false;
			LOG_ERROR << "Campagne invalide : Le filtre " << pPair.first << " n'est pas pris en compte par l'application.";
		}
		return lIsValidFilter;
	};
	int lNombreFiltreInterneValide = count_if(mFilterInclude.begin(), mFilterInclude.end(), lVerificationFiltre);
	int lNombreFiltreExterneValide = count_if(mFilterExlude.begin(), mFilterExlude.end(), lVerificationFiltre);
	if(lNombreFiltreInterneValide != mFilterInclude.size() || lNombreFiltreExterneValide != mFilterExlude.size())
	{
		throw invalid_argument("L'un des filtres include ou exclude est invalide : trop d'éléments ou balise inconnue");
	}

	/*
	 * Controle des min/max
	 */
	if(mBudget < 1. || mBudget > 1000.)
	{
		throw invalid_argument("Le budget de la campagne n'est pas compris entre 1 et 1000");
	}

	if(mBidPrice < 0.01 || mBidPrice > 10.)
	{
		throw invalid_argument("Le BidPrice de la campagne n'est pas compris entre 0.01 et 10");
	}

}

Campagne::Campagne(const Campagne& pCampagne)
{
	// Utilisation d'un std::lock pour lock les 2 mutex en meme temps
	// sans risquer un deadlock
	lock(pCampagne.mMutex, mMutex);

	mId = pCampagne.mId;
	mBudget = pCampagne.mBudget;
	mBidPrice = pCampagne.mBidPrice;
	mWidth = pCampagne.mWidth;
	mHeight = pCampagne.mHeight;
	mResponsive = pCampagne.mResponsive;
	mFilterInclude = pCampagne.mFilterInclude;
	mFilterExlude = pCampagne.mFilterExlude;
	mUrl = pCampagne.mUrl;

	pCampagne.mMutex.unlock();
	mMutex.unlock();
}

Campagne::Campagne(Campagne&& pCampagne)
{
	lock(pCampagne.mMutex, mMutex);

	mId = move(pCampagne.mId);
	mBudget = move(pCampagne.mBudget);
	mBidPrice = move(pCampagne.mBidPrice);
	mWidth = move(pCampagne.mWidth);
	mHeight = move(pCampagne.mHeight);
	mResponsive = move(pCampagne.mResponsive);
	mFilterInclude = move(pCampagne.mFilterInclude);
	mFilterExlude = move(pCampagne.mFilterExlude);
	mUrl = move(pCampagne.mUrl);

	pCampagne.mMutex.unlock();
	mMutex.unlock();
}

Campagne& Campagne::operator = (const Campagne& pCampagne)
{
	lock(pCampagne.mMutex, mMutex);

	mId = pCampagne.mId;
	mBudget = pCampagne.mBudget;
	mBidPrice = pCampagne.mBidPrice;
	mWidth = pCampagne.mWidth;
	mHeight = pCampagne.mHeight;
	mResponsive = pCampagne.mResponsive;
	mFilterInclude = pCampagne.mFilterInclude;
	mFilterExlude = pCampagne.mFilterExlude;
	mUrl = pCampagne.mUrl;

	pCampagne.mMutex.unlock();
	mMutex.unlock();

	return *this;
}

Campagne& Campagne::operator = (Campagne&& pCampagne)
{
	lock(pCampagne.mMutex, mMutex);

	mId = move(pCampagne.mId);
	mBudget = move(pCampagne.mBudget);
	mBidPrice = move(pCampagne.mBidPrice);
	mWidth = move(pCampagne.mWidth);
	mHeight = move(pCampagne.mHeight);
	mResponsive = move(pCampagne.mResponsive);
	mFilterInclude = move(pCampagne.mFilterInclude);
	mFilterExlude = move(pCampagne.mFilterExlude);
	mUrl = move(pCampagne.mUrl);

	pCampagne.mMutex.unlock();
	mMutex.unlock();

	return *this;
}

string Campagne::ToString()
{
	string lReturn;
	lReturn += "Information sur la campagne " + mId + "\n";
	lReturn += " ** mBudget " + to_string(mBudget) + "\n";
	lReturn += " ** mBidPrice " + to_string(mBidPrice) + "\n";
	lReturn += " ** mWidth " + to_string(mWidth) + "\n";
	lReturn += " ** mHeight " + to_string(mHeight) + "\n";
	lReturn += " ** mResponsive " + to_string(mResponsive) + "\n";
	lReturn += " ** mUrl " + mUrl + "\n";

	return lReturn;
}

bool Campagne::DecrementeBudget()
{
	bool lRet = true;
	mBudget >= mBidPrice ? mBudget -=  mBidPrice : lRet = false;
	return lRet;
}

