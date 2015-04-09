#include "config.h"

config::config()
{
	fStream.open("config.ini");
}

config::~config()
{
	fStream.close();
}
