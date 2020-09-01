/*
 * Campagnes.h
 *
 *  Created on: 01 septembre 2020
 *      Author: Nicolas MEO
 */
#include "Campagne.h"
#include <algorithm>

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
	 * Vérification du nombres d'éléments et des mins / max
	 */

	// TODO : Vérification des nombres d'éléments dans les filters
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

Campagne::Campagne(Campagne&& pCampagne)
{
	pCampagne.mMutex.lock();

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
}

Campagne& Campagne::operator = (const Campagne& pCampagne)
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

	return *this;
}

Campagne& Campagne::operator = (Campagne&& pCampagne)
{
	pCampagne.mMutex.lock();

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

