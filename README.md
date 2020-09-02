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

## Requirements à vérifier
* sudo apt-get install postgresql-server-dev-all
* sudo apt-get install postgresql-all
* sudo apt install libmariadbclient-dev
* sudo apt-get install libsqlite3-dev

# Compilation du projet

## Avec Docker

## Avec CMake

Pour le moment, on a fait tout ce qu'il y a ci-dessous + intégration dans projet // à revoir !
>cd $WORK_PATH
git clone https://github.com/an-tao/drogon
cd drogon
git submodule update --init
mkdir build
cd build
cmake ..
make && sudo make install


