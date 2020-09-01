/*
 * Campagnes.h
 *
 *  Created on: 01 septembre 2020
 *      Author: Nicolas MEO
 */
#include "Campagne.h"

using namespace std;

Campagne::Campagne(string pId, float pBudget, float pBidPrice, int pWidth, int pHeight,
		bool pResponsive, map<string,vector<string>> pFilterInclude,
		map<string,vector<string>> pFilterExclude, string pUrl):
			mId(pId), mBudget(pBudget), mBidPrice(pBidPrice), mWidth(pWidth), mHeight(pHeight),
			mResponsive(pResponsive), mFilterInclude(pFilterInclude), mFilterExlude(pFilterExclude), mUrl(pUrl)
{
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
	// TODO : Vérification des nombres d'éléments dans les filters
	// TODO : Vérification si un filtre est dans include, il peut pas etre dans exclude
	// TODO : Controle des mins / maxs
}

Campagne::Campagne(const Campagne& pCampagne)
{
	pCampagne.mMutex.lock();

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

