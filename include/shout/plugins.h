/*  shout-plugins.h
 *
 *  API for libshout plugins
 *
 *  Copyright (C) 2002-2003 the Icecast team <team@icecast.org>,
 *  Copyright (C) 2012-2015 Philipp "ph3-der-loewe" Schafft <lion@lion.leolix.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __LIBSHOUT_PLUGINS_H__
#define __LIBSHOUT_PLUGINS_H__

#define PLUGIN_API_VERSION 1
typedef struct {
	const int api_version;
	const char *name;   /* human readable plugin name */
} shout_plugin_generic;

typedef struct {
	shout_plugin_generic desc;
	const char **mimes; /* null terminated string array*/

	int (* open_check)(shout_t *self);
	int (* open)(shout_t *self);
	int (* send)(shout_t *self, const unsigned char *data, size_t len);
	void (* close)(shout_t *self);
} shout_plugin_format;

#endif /* __LIBSHOUT_PLUGINS_H__ */
