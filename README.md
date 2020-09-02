# tabmo-codding-challenge

L'objectif du coding challenge est de développer une application répondant aux besoins exprimés par le responsable fonctionnel. L'application prendra la forme d'un serveur HTTP simulant à échelle réduite le mécanisme d'achat de publicité programmatique.

Les spécifications fonctionnelles de ce challenge sont disponibles au [lien suivant](https://github.com/tabmo/coding-challenge/tree/master/backend/cpp).

# Requirements

Afin de compiler cette application, les paquets suivants doivent être installés (la ligne de commande sous debian est fournie)

## Requirements du projet

* **Git** : sudo apt install git
* **Gcc** : sudo apt install gcc
* **G++** : sudo apt install g++
* **Cmake** : sudo apt install cmake
* **Ninja-Build** :sudo apt install ninja-build

##  Requirements de la dépendance Drogon

* **JsonCpp** : sudo apt *install libjsoncpp-dev*. Sous debian cette librairie s'installe dans un sous dossier jsoncpp, la commande suivante a du être lancée pour éviter de modifier les sources des Drogon : *sudo ln -s /usr/include/jsoncpp/json/ /usr/include/json*
* **Uuid-dev** : sudo apt install uuid-dev
* **open-ssl** : sudo apt install openssl
* **libssl-dev** : sudo apt install libssl-dev
* **zlib1g-dev** : sudo apt install zlib1g-dev
* **Boost** : sudo apt install libboost-all-dev

# Compilation du projet

## Avec CMake

Une fois toutes les dépendances installées, executer le script :
>./build.sh

L'application est compilée en mode RELEASE.

# Lancement de l'application

## Manuel

Pour lancer l'application, tapez les commandes suivantes 
>cd build/defaut

>./tabmo-codding-challenge

L'application se lance. Les paramètres par défaut pricipaux sont les suivants:
* IP : 127.0.0.1
* Port : 8080
* Protocole : HTTP
* Nombre de thread : 3
* Niveau de log : INFO
* Chemin vers le dossier res : ./res

Afin de les modifier, se référer à la section configuration de l'application de ce README

## Avec Docker

TODO

# Configuration de l'application

La configuration de l'application se déroule dans le dossier **build/default/res**.
2 fichiers sont configurables :
* **campaigns.json** : Contient les campagnes fournies dans l'énoncé de l'exercice
* **config.drogon.json** : Contient toutes les configurations liées au framework Drogo

Le dossier bid-requests contient les 3 bid-requests fournies dans l'énoncé avec un script bash **stress-app.bash** permettant de les lancer 400 fois chacunes via curl. Ce script est notament utile pour tester la distribution des campagnes selectionnées aléatoirement (pondérées par leur budget).





