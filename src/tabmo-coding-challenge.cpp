#include <drogon/HttpAppFramework.h>

#include "CampagnesManager.h"
using namespace std;

int main(int argc, char **argv)
{
	LOG_INFO << "Demarrage de l'application";
	if(!CampagnesManager::ParseCampagnesFromJson("./res/campaigns.json"))
	{
		LOG_ERROR << "Au moins l'une des campagnes est invalide";
		if(CampagnesManager::GetCampagnesCount() == 0)
		{
			LOG_ERROR << "Aucune campagne valide n'a été trouvée, arrêt de l'application";
			exit(1);
		}
		else
		{
			LOG_INFO << "Nombre de campagnes valides détectées " << CampagnesManager::GetCampagnesCount();
		}
	}

	drogon::app().loadConfigFile("./res/config.drogon.json").run();
	return 0;
}

