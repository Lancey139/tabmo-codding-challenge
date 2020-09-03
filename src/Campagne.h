/*
 * Campagnes.h
 *
 *  Created on: 30 août 2020
 *      Author: Nicolas MEO
 */

#pragma once
#include <string>
#include <mutex>
#include <map>
#include <vector>

using namespace std;

/*
 * classe permettant de stocker les informations relatives à une campagne
 * Tous les éléments sauf le Budget sont constants afin qu'on ne puisse pas les modifier
 * Cette objet contenant un mutex, les méthodes ont été implémentées pour le rendre movable
 */
class Campagne
{
public:
	/*
	 * Constructeur
	 */
	Campagne(string pId, float pBudget, float pBidPrice, int pWidth, int pHeight,
			bool pResponsive, map<string,vector<string>> pFilterInclude,
			map<string,vector<string>> pFilterExclude, string pUrl);

	/*
	 * Constructeur par copie
	 * @param pCampagne : Campagne à copier
	 */
	Campagne(const Campagne& pCampagne);

	/*
	 * Constructeur par move
	 */
	Campagne(Campagne&& pCampagne);

	/*
	 * Assignation par Copie
	 */
	Campagne& operator = (const Campagne& pCampagne);

	/*
	 * Assignation par move
	 */
	Campagne& operator = (Campagne&& pCampagne);

	/*
	 * Méthode permettant d'afficher la plupart des attributs
	 * @return : string
	 */
	string ToString();

	/*
	 * Getter
	 */
	 const string& GetId() const  { return mId; }
	 const float& GetBudget() const  { return mBudget; }
	 const float& GetBidPrice() const  { return mBidPrice; }
	 const int& GetWidth() const  { return mWidth; }
	 const int& GetHeight() const  { return mHeight; }
	 const bool& GetResponsive() const  { return mResponsive; }
	 const map<string,vector<string>>& GetFilterInclude() const  { return mFilterInclude; }
	 const map<string,vector<string>>& GetFilterExclude() const  { return mFilterExlude; }
	 const string& GetUrl() const  { return mUrl; }
	 mutex& GetMutex() { return mMutex; }
	 Campagne* GetPointeur() {return this;}

	 /*
	  * Méthode permettant de décrémenter le budget d'une campagne qui vient d'acheter une bid-request
	  * @return : true si le budget a pu etre prélevé, false sinon
	  */
	 bool DecrementeBudget();

private:
	 // Attributs permettant de stocker les valeurs issues du JSON
	string mId;
	float  mBudget;
	float mBidPrice;
	int mWidth;
	int mHeight;
	bool mResponsive;
	string mUrl;
	// Ces maps permettant de strocker les valeurs des filtres
	map<string,vector<string>> mFilterInclude;
	map<string,vector<string>> mFilterExlude;
	// Attribut permettant lors de la sélection d'une campagne de rester thread safe
	// sans bloquer tout le vecteur de campagne
	mutable mutex mMutex;

};
