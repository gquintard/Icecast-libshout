#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#include "shout_private.h"

typedef struct plugin_list {
	const shout_plugin_desc *plugin;
	void *dlhandle;
	const struct plugin_list *next;
} plugin_list;

static const struct plugin_list* register_plugins(const struct plugin_list *list, const shout_plugin_desc *desc, void *dlhandle);
static const struct plugin_list* register_plugins(const struct plugin_list *list, const shout_plugin_desc *desc, void *dlhandle)
{
	struct plugin_list *entry = malloc(sizeof(struct plugin_list));

	if (!entry)
		return list;

	entry->next = list;
	entry->plugin = desc;
	entry->dlhandle = dlhandle;
	return entry;
}

const void *open_plugins()
{
	const struct plugin_list *list = NULL;

#ifdef PLUGIN_DIR
#define PMAXLEN 256
	shout_plugin_desc *desc = NULL;
	DIR *dp;
	void *dlhandle;
	struct dirent *ep;
	char buf[PMAXLEN];
	int n;
	char *p;
	dp = opendir (PLUGIN_DIR);
	if (!dp)
		return NULL;
	while ((ep = readdir(dp))) {

		/* starts with libshout_ */
		if (strncmp(ep->d_name, "libshout_", 9))
			continue;
		/* ends with .so */
		p = strrchr(ep->d_name, '.');
		if (!p || strncmp(p, ".so\0", 4))
			continue;

		n = snprintf(buf, PMAXLEN, PLUGIN_DIR "/%s", ep->d_name);
		if (n >= PMAXLEN || n < 0)
			continue;

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
#undef PMAXLEN
#endif /* PLUGIN_DIR */

	/* baked-in plugins */
	list = register_plugins(list, &shout_plugin_mp3, NULL);
	list = register_plugins(list, &shout_plugin_webm, NULL);
	list = register_plugins(list, &shout_plugin_ogg, NULL);

	return list;
}

void close_plugins(const void *plugins)
{
	const struct plugin_list *list = (struct plugin_list *)plugins;
	const struct plugin_list *listtmp = NULL;

	while (list) {
		listtmp = list->next;
		if (list->dlhandle)
			dlclose(list->dlhandle);
		free((void *)list);
		list = listtmp;
	}
}

int plugin_selector(shout_t *self, const void *plugins, const char *mime)
{
	const struct plugin_list *list = (struct plugin_list *)plugins;
	const shout_plugin_desc *plugin = NULL;
	const char **mimes = NULL;

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
