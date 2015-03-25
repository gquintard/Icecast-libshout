#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "shout_private.h"

typedef struct plugin_list {
	shout_plugin_desc *plugin;
	void *dlhandle;
	struct plugin_list *next;
} plugin_list;

static struct plugin_list* register_plugins(struct plugin_list *list, shout_plugin_desc *desc, void *dlhandle);
static struct plugin_list* register_plugins(struct plugin_list *list, shout_plugin_desc *desc, void *dlhandle)
{
	struct plugin_list *entry = malloc(sizeof(struct plugin_list));

	if (!entry)
		return list;

	entry->next = list;
	entry->plugin = desc;
	entry->dlhandle = dlhandle;
	return entry;
}

void *open_plugins()
{
	struct plugin_list *list = NULL;

#ifdef PLUGIN_DIR
	shout_plugin_desc *desc = NULL;
	DIR *dp;
	void *dlhandle;
	struct dirent *ep;
	char buf[256];
	dp = opendir (PLUGIN_DIR);
	if (!dp)
		return NULL;
	while ((ep = readdir(dp))) {

		if (strncmp(ep->d_name, "libshout_", 9))
			continue;

		snprintf(buf, 256, PLUGIN_DIR "/%s", ep->d_name);
		dlhandle = dlopen(buf , RTLD_NOW | RTLD_LOCAL);
		if (!dlhandle)
			continue;

		desc = dlsym(dlhandle, "shout_plugin");
		if (!desc || desc->api_version != PLUGIN_API_VERSION) {
			dlclose(dlhandle);
			continue;
		}	
		list = register_plugins(list, desc, dlhandle);
	}
	closedir(dp);
#endif /* PLUGIN_DIR */

	/* baked-in plugins */
	list = register_plugins(list, &shout_plugin_mp3, NULL);
	list = register_plugins(list, &shout_plugin_webm, NULL);
	list = register_plugins(list, &shout_plugin_ogg, NULL);

	return list;
}

void close_plugins(void *plugins)
{
	struct plugin_list *list = (struct plugin_list *)plugins;
	struct plugin_list *listtmp = NULL;

	while (list) {
		listtmp = list;
		if (list->dlhandle)
			dlclose(list->dlhandle);
		free(list);
		list = listtmp->next;
	}
}

int plugin_selector(shout_t *self, void *plugins, const char *mime)
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
