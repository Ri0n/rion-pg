#include "rdaemonapp.h"
//#include "xmlrpcdaemon.h"

int main(int argc, char *argv[])
{
	RDaemonApp *app = new RDaemonApp(argc, argv);
	if (!app->isDaemonStarted()) {
		return 1;
	}
	return app->exec();
}
