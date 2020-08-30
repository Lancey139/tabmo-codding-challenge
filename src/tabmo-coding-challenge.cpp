#include <drogon/HttpAppFramework.h>
#include "Campagnes.h"
using namespace std;

int main(int argc, char **argv)
{
	if(!Campagnes::ParseCampagnesFromJson("./res/campaigns.json"))
	{
		LOG_ERROR << "Au moins l'une des campagnes est invalide";
		if(Campagnes::GetCampagnesCount() == 0)
		{
			LOG_ERROR << "Aucune campagne valide n'a été trouvée, arrêt de l'application";
			exit(1);
		}
	}

	drogon::app().loadConfigFile("./res/config.drogon.json").run();
	return 0;
}

