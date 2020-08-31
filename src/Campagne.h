/*
 * Campagnes.h
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#pragma once
#include <string>
#include <mutex>

/*
 * Structure de données permettant de stocker les informations relatives à une campagne
 * Tous les éléments sauf le Budget sont constants afin qu'on ne puisse pas les modifier
 */
struct Campagne
{
	const string mId;
	float  mBudget;
	const float mBidPrice;
	const int mWidth;
	const int mHeight;
	const bool mResponsive;
	const map<string,vector<string>> mFilterInclude;
	const map<string,vector<string>> mFilterExlude;
	const string mUrl;

	Campagne(string pId, float pBudget, float pBidPrice, int pWidth, int pHeight,
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

	string ToString()
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
};
