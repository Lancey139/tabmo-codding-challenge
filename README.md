# tabmo-codding-challenge
L'objectif du coding challenge est de développer une application répondant aux besoins exprimés par le responsable fonctionnel. L'application prendra la forme d'un serveur HTTP simulant à échelle réduite le mécanisme d'achat de publicité programmatique.

# Requirements

sudo apt install git
sudo apt install gcc
sudo apt install g++
sudo apt install cmake
sudo apt install ninja-build

#  Requirements Drogon

sudo apt install libjsoncpp-dev

sudo ln -s /usr/include/jsoncpp/json/ /usr/include/json
sudo apt install uuid-dev
sudo apt install openssl
sudo apt install libssl-dev
sudo apt install zlib1g-dev
sudo apt install libboost-all-dev

sudo apt-get install postgresql-server-dev-all
sudo apt-get install postgresql-all
sudo apt install libmariadbclient-dev
sudo apt-get install libsqlite3-dev
sudo apt-get install brotli

Pour le moment, on a fait tout ce qu'il y a ci-dessous + intégration dans projet // à revoir !
cd $WORK_PATH
git clone https://github.com/an-tao/drogon
cd drogon
git submodule update --init
mkdir build
cd build
cmake ..
make && sudo make install


