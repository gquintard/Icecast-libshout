#include "shout_private.h"

static void register_plugins(shout_plugin_desc* desc_array[], shout_plugin_desc *sd);
static void register_plugins(shout_plugin_desc* desc_array[], shout_plugin_desc *sd)
{
	int i = 0;
	/* find first empty slot (change that to a linked list)*/
	while(i < MAXPLUGINS && desc_array[i])
		i++;

	if (i == MAXPLUGINS)
		return;

	desc_array[i] = sd;
}

char *shout_mp3_mimes[] = {"audio/mpeg", NULL};
shout_plugin_desc shout_mp3_desc = {
	.name  = "mp3",
	.mimes = shout_mp3_mimes,
	.open  = shout_open_mp3
};

void open_plugins(shout_plugin_desc* desc_array[])
{
	register_plugins(desc_array, &shout_mp3_desc);
}

void close_plugins(shout_plugin_desc* desc_array[])
{
}
