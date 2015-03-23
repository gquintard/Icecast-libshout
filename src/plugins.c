#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "shout_private.h"

typedef struct plugin_list {
	shout_plugin_desc *plugin;
	struct plugin_list *next;
} plugin_list;

static struct plugin_list* register_plugins(struct plugin_list *list, shout_plugin_desc *desc);
static struct plugin_list* register_plugins(struct plugin_list *list, shout_plugin_desc *desc)
{
	struct plugin_list *entry = malloc(sizeof(struct plugin_list));

	if (!entry)
		return list;

	entry->next = list;
	entry->plugin = desc;
	return entry;
}

void *open_plugins()
{
	shout_plugin_desc *desc = NULL;
	struct plugin_list *list = NULL;
	DIR *dp;
	void *handle;
	struct dirent *ep;
	char buf[256];


	dp = opendir ("/tmp/");
	if (!dp)
		return NULL;
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
		printf("registering file %s (%s)\n", ep->d_name, desc->name);
		list = register_plugins(list, desc);
	}
	closedir(dp);
	return list;
}

void close_plugins(void *plugins)
{
	struct plugin_list *list = (struct plugin_list *)plugins;
	struct plugin_list *listtmp = NULL;

	while (list) {
		listtmp = list->next;
		free((void *)list);
		list = listtmp;
	}
}

int plugin_selector(void *plugins, shout_t *self, const char *mime)
{
	struct plugin_list *list = (struct plugin_list *)plugins;
	shout_plugin_desc *plugin = NULL;
	char **mimes = NULL;

	do {
		plugin = list->plugin; 
		for (mimes = plugin->mimes; *mimes; mimes++) {
			if (strcmp(mime, *mimes))
				continue;

			/* XXX: strdup?  probably not useful as desc won't change */
			self->plugin = plugin;
			self->mime = *mimes;
			self->format = SHOUT_FORMAT_PLUGIN;
			return self->error = SHOUTERR_SUCCESS;
		}

	} while ((list = list->next));
	return self->error = SHOUTERR_UNSUPPORTED;

}
