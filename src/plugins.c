#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "shout_private.h"

static void register_plugins(shout_plugin_desc** desc_array, shout_plugin_desc *sd);
static void register_plugins(shout_plugin_desc** desc_array, shout_plugin_desc *sd)
{
	int i = 0;
	/* find first empty slot (change that to a linked list)*/
	while(i < MAXPLUGINS && desc_array[i])
		i++;

	if (i == MAXPLUGINS)
		return;

	desc_array[i] = sd;
}

void open_plugins(shout_plugin_desc** desc_array)
{
	struct shout_plugin_desc *desc = NULL;
	DIR *dp;
	void *handle;
	struct dirent *ep;
	char buf[256];

	dp = opendir ("/tmp/");
	if (!dp)
		return;
	while ((ep = readdir(dp))) {
		if (!strncmp(ep->d_name, "libshout_", 10))
			continue;

		snprintf(buf, 256, "/tmp/%s", ep->d_name);
		handle = dlopen(buf , RTLD_NOW | RTLD_LOCAL);
		if (!handle)
			continue;

		desc = dlsym(handle, "shout_plugin");
		if (!desc) {
			dlclose(handle);
			continue;
		}	
		printf("registering file %s\n", ep->d_name);
		register_plugins(desc_array, desc);
	}
	closedir(dp);
}

void close_plugins(shout_plugin_desc* desc_array[])
{
}
