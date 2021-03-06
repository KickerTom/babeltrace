#ifndef BABELTRACE_PLUGIN_PLUGIN_INTERNAL_H
#define BABELTRACE_PLUGIN_PLUGIN_INTERNAL_H

/*
 * BabelTrace - Plug-in Internal
 *
 * Copyright 2015 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * Author: Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <babeltrace/babeltrace-internal.h>
#include <babeltrace/plugin/plugin.h>
#include <babeltrace/plugin/plugin-dev.h>
#include <babeltrace/object-internal.h>
#include <babeltrace/types.h>
#include <glib.h>

enum bt_plugin_type {
	BT_PLUGIN_TYPE_SO = 0,
	BT_PLUGIN_TYPE_PYTHON = 1,
};

struct bt_plugin {
	struct bt_object base;
	enum bt_plugin_type type;
	bt_bool frozen;

	/* Array of pointers to bt_component_class (owned by this) */
	GPtrArray *comp_classes;

	/* Info (owned by this) */
	struct {
		GString *path;
		GString *name;
		GString *author;
		GString *license;
		GString *description;
		struct {
			unsigned int major;
			unsigned int minor;
			unsigned int patch;
			GString *extra;
		} version;
		bt_bool path_set;
		bt_bool name_set;
		bt_bool author_set;
		bt_bool license_set;
		bt_bool description_set;
		bt_bool version_set;
	} info;

	/* Value depends on the specific plugin type */
	void *spec_data;
	void (*destroy_spec_data)(struct bt_plugin *);
};

struct bt_plugin_set {
	struct bt_object base;

	/* Array of struct bt_plugin * */
	GPtrArray *plugins;
};

static inline
const char *bt_plugin_status_string(enum bt_plugin_status status)
{
	switch (status) {
	case BT_PLUGIN_STATUS_OK:
		return "BT_PLUGIN_STATUS_OK";
	case BT_PLUGIN_STATUS_ERROR:
		return "BT_PLUGIN_STATUS_ERROR";
	case BT_PLUGIN_STATUS_NOMEM:
		return "BT_PLUGIN_STATUS_NOMEM";
	default:
		return "(unknown)";
	}
}

static inline
const char *bt_plugin_type_string(enum bt_plugin_type type)
{
	switch (type) {
	case BT_PLUGIN_TYPE_SO:
		return "BT_PLUGIN_TYPE_SO";
	case BT_PLUGIN_TYPE_PYTHON:
		return "BT_PLUGIN_TYPE_PYTHON";
	default:
		return "(unknown)";
	}
}

static inline
void bt_plugin_destroy(struct bt_object *obj)
{
	struct bt_plugin *plugin;

	assert(obj);
	plugin = container_of(obj, struct bt_plugin, base);
	BT_LOGD("Destroying plugin object: addr=%p, name=\"%s\"",
		plugin, plugin->info.name ? plugin->info.name->str : NULL);

	if (plugin->destroy_spec_data) {
		plugin->destroy_spec_data(plugin);
	}

	if (plugin->comp_classes) {
		BT_LOGD_STR("Putting component classes.");
		g_ptr_array_free(plugin->comp_classes, TRUE);
	}

	if (plugin->info.name) {
		g_string_free(plugin->info.name, TRUE);
	}

	if (plugin->info.path) {
		g_string_free(plugin->info.path, TRUE);
	}

	if (plugin->info.description) {
		g_string_free(plugin->info.description, TRUE);
	}

	if (plugin->info.author) {
		g_string_free(plugin->info.author, TRUE);
	}

	if (plugin->info.license) {
		g_string_free(plugin->info.license, TRUE);
	}

	if (plugin->info.version.extra) {
		g_string_free(plugin->info.version.extra, TRUE);
	}

	g_free(plugin);
}

static inline
struct bt_plugin *bt_plugin_create_empty(enum bt_plugin_type type)
{
	struct bt_plugin *plugin = NULL;

	BT_LOGD("Creating empty plugin object: type=%s",
		bt_plugin_type_string(type));

	plugin = g_new0(struct bt_plugin, 1);
	if (!plugin) {
		BT_LOGE_STR("Failed to allocate one plugin.");
		goto error;
	}

	bt_object_init(plugin, bt_plugin_destroy);
	plugin->type = type;

	/* Create empty array of component classes */
	plugin->comp_classes =
		g_ptr_array_new_with_free_func((GDestroyNotify) bt_put);
	if (!plugin->comp_classes) {
		BT_LOGE_STR("Failed to allocate a GPtrArray.");
		goto error;
	}

	/* Create empty info */
	plugin->info.name = g_string_new(NULL);
	if (!plugin->info.name) {
		BT_LOGE_STR("Failed to allocate a GString.");
		goto error;
	}

	plugin->info.path = g_string_new(NULL);
	if (!plugin->info.path) {
		BT_LOGE_STR("Failed to allocate a GString.");
		goto error;
	}

	plugin->info.description = g_string_new(NULL);
	if (!plugin->info.description) {
		BT_LOGE_STR("Failed to allocate a GString.");
		goto error;
	}

	plugin->info.author = g_string_new(NULL);
	if (!plugin->info.author) {
		BT_LOGE_STR("Failed to allocate a GString.");
		goto error;
	}

	plugin->info.license = g_string_new(NULL);
	if (!plugin->info.license) {
		BT_LOGE_STR("Failed to allocate a GString.");
		goto error;
	}

	plugin->info.version.extra = g_string_new(NULL);
	if (!plugin->info.version.extra) {
		BT_LOGE_STR("Failed to allocate a GString.");
		goto error;
	}

	BT_LOGD("Created empty plugin object: type=%s, addr=%p",
		bt_plugin_type_string(type), plugin);
	goto end;

error:
	BT_PUT(plugin);

end:
	return plugin;
}

static inline
void bt_plugin_set_path(struct bt_plugin *plugin, const char *path)
{
	assert(plugin);
	assert(path);
	g_string_assign(plugin->info.path, path);
	plugin->info.path_set = BT_TRUE;
	BT_LOGV("Set plugin's path: addr=%p, name=\"%s\", path=\"%s\"",
		plugin, bt_plugin_get_name(plugin), path);
}

static inline
void bt_plugin_set_name(struct bt_plugin *plugin, const char *name)
{
	assert(plugin);
	assert(name);
	g_string_assign(plugin->info.name, name);
	plugin->info.name_set = BT_TRUE;
	BT_LOGV("Set plugin's name: addr=%p, name=\"%s\"",
		plugin, name);
}

static inline
void bt_plugin_set_description(struct bt_plugin *plugin,
		const char *description)
{
	assert(plugin);
	assert(description);
	g_string_assign(plugin->info.description, description);
	plugin->info.description_set = BT_TRUE;
	BT_LOGV("Set plugin's description: addr=%p, name=\"%s\"",
		plugin, bt_plugin_get_name(plugin));
}

static inline
void bt_plugin_set_author(struct bt_plugin *plugin, const char *author)
{
	assert(plugin);
	assert(author);
	g_string_assign(plugin->info.author, author);
	plugin->info.author_set = BT_TRUE;
	BT_LOGV("Set plugin's author: addr=%p, name=\"%s\", author=\"%s\"",
		plugin, bt_plugin_get_name(plugin), author);
}

static inline
void bt_plugin_set_license(struct bt_plugin *plugin, const char *license)
{
	assert(plugin);
	assert(license);
	g_string_assign(plugin->info.license, license);
	plugin->info.license_set = BT_TRUE;
	BT_LOGV("Set plugin's path: addr=%p, name=\"%s\", license=\"%s\"",
		plugin, bt_plugin_get_name(plugin), license);
}

static inline
void bt_plugin_set_version(struct bt_plugin *plugin, unsigned int major,
		unsigned int minor, unsigned int patch, const char *extra)
{
	assert(plugin);
	plugin->info.version.major = major;
	plugin->info.version.minor = minor;
	plugin->info.version.patch = patch;

	if (extra) {
		g_string_assign(plugin->info.version.extra, extra);
	}

	plugin->info.version_set = BT_TRUE;
	BT_LOGV("Set plugin's version: addr=%p, name=\"%s\", "
		"major=%u, minor=%u, patch=%u, extra=\"%s\"",
		plugin, bt_plugin_get_name(plugin),
		major, minor, patch, extra);
}

static inline
void bt_plugin_freeze(struct bt_plugin *plugin)
{
	assert(plugin);

	if (plugin->frozen) {
		return;
	}

	BT_LOGD("Freezing plugin: addr=%p, name=\"%s\", path=\"%s\"",
		plugin, bt_plugin_get_name(plugin),
		bt_plugin_get_path(plugin));
	plugin->frozen = BT_TRUE;
}

static
void bt_plugin_set_destroy(struct bt_object *obj)
{
	struct bt_plugin_set *plugin_set =
		container_of(obj, struct bt_plugin_set, base);

	if (!plugin_set) {
		return;
	}

	BT_LOGD("Destroying plugin set: addr=%p", plugin_set);

	if (plugin_set->plugins) {
		BT_LOGD_STR("Putting plugins.");
		g_ptr_array_free(plugin_set->plugins, TRUE);
	}

	g_free(plugin_set);
}

static inline
struct bt_plugin_set *bt_plugin_set_create(void)
{
	struct bt_plugin_set *plugin_set = g_new0(struct bt_plugin_set, 1);

	if (!plugin_set) {
		goto end;
	}

	BT_LOGD_STR("Creating empty plugin set.");
	bt_object_init(plugin_set, bt_plugin_set_destroy);

	plugin_set->plugins = g_ptr_array_new_with_free_func(
		(GDestroyNotify) bt_put);
	if (!plugin_set->plugins) {
		BT_LOGE_STR("Failed to allocate a GPtrArray.");
		BT_PUT(plugin_set);
		goto end;
	}

	BT_LOGD("Created empty plugin set: addr=%p", plugin_set);

end:
	return plugin_set;
}

static inline
void bt_plugin_set_add_plugin(struct bt_plugin_set *plugin_set,
		struct bt_plugin *plugin)
{
	assert(plugin_set);
	assert(plugin);
	g_ptr_array_add(plugin_set->plugins, bt_get(plugin));
	BT_LOGV("Added plugin to plugin set: "
		"plugin-set-addr=%p, plugin-addr=%p, plugin-name=\"%s\", "
		"plugin-path=\"%s\"",
		plugin_set, plugin, bt_plugin_get_name(plugin),
		bt_plugin_get_path(plugin));
}

#endif /* BABELTRACE_PLUGIN_PLUGIN_INTERNAL_H */
