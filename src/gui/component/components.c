/** \file components.c
 *
 * Available Components and related utilities
 */
#include "components.h"

#define OPTIONAL 0
#define DEFAULT 1

LSComponent* ls_component_best_sum_new(json_t* config);
LSComponent* ls_component_pb_new(json_t* config);
LSComponent* ls_component_prev_segment_new(json_t* config);
LSComponent* ls_component_splits_new(json_t* config);
LSComponent* ls_component_detailed_timer_new(json_t* config);
LSComponent* ls_component_title_new(json_t* config);
LSComponent* ls_component_wr_new(json_t* config);

const LSComponentAvailable ls_components[] = {
    { "title", ls_component_title_new, DEFAULT },
    { "splits", ls_component_splits_new, DEFAULT },
    { "timer", ls_component_detailed_timer_new, DEFAULT },
    { "prev-segment", ls_component_prev_segment_new, DEFAULT },
    { "best-sum", ls_component_best_sum_new, DEFAULT },
    { "pb", ls_component_pb_new, DEFAULT },
    { "wr", ls_component_wr_new, DEFAULT },
    { NULL, NULL }
};

/**
 * Look up a component by name.
 *
 * @return Returns a reference to the component via the LSComponentAvailable
 * struct if found, NULL otherwise
 */
const LSComponentAvailable* get_component(const char* const name)
{
    const LSComponentAvailable* ref = &ls_components[0];

    if (!name) {
        return NULL;
    }

    while (ref->name) {
        if (!strcmp(ref->name, name)) {
            return ref;
        }
        ++ref;
    }
    return NULL;
}


/**
 * @brief Resolves the icon file path to a URI.
 * This handles converting full/relative file paths to the user's system
 * so that it can be converted to a usable file:// URI
 * while retaining already valid URI's for web urls, data-urls etc.
 *
 * If you use this, you must g_free the result if it is not NULL after usage.
 *
 * @param game The game struct for the current splits file.
 * @param source The path to the icon being loaded.
 * @return char* A new string to for the valid icon URI.
 */
static const char* resolve_icon_uri(const ls_game* game, const char* source)
{
    // max_len should be -1 for null terminated strings.
    if (!source || !g_utf8_validate(source, -1, NULL)) {
        return NULL;
    }

    // The supplied source is already correctly formatted.
    // Duplicate it so the followup free doesn't kill the source path.
    if (g_uri_peek_scheme(source)) {
        return g_strdup(source);
    }

    GFile* icon;
    if (g_path_is_absolute(source)) {
        icon = g_file_new_for_path(source);
    } else {
        GFile* splits_file = g_file_new_for_path(game->path);
        GFile* parent_dir = g_file_get_parent(splits_file);
        g_object_unref(splits_file);

        if (!parent_dir) {
            return NULL;
        }

        icon = g_file_resolve_relative_path(parent_dir, source);
        g_object_unref(parent_dir);
    }

    char* uri = g_file_get_uri(icon);
    g_object_unref(icon);
    return uri;
}
/**
 * @brief Appends a URI to a GString. Wraps the string in quotes and then escapes
 * special characters to w3 spec so that non-basic URIs don't break.
 * This also applies quotes so that you can pass something like
 * https://url.com
 * and get
 * "https://url.com"
 * As your response to pass directly to a css URL such as
 * background-image: url(YOUR-STRING);
 *
 * @param str The string to append the URI to.
 * @param value A raw URI path string.
 */
static void append_quoted_uri(GString* str, const char* value)
{
    g_string_append_c(str, '"');

    // https://www.w3.org/TR/cssom-1/#serialize-a-string
    for (const unsigned char* c = (const unsigned char*)value; *c != '\0'; ++c) {
        switch (*c) {
            /** U+0022 = " */
            case '"':
                g_string_append(str, "\\\"");
                break;
            /** U+005C = \ */
            case '\\':
                g_string_append(str, "\\\\");
                break;
            /** NULL character means the end of string so we don't need to handle that */
            default:
                if ((*c >= 1 && *c <= 0x1F) || *c == 0x7F) {
                    g_string_append_printf(str, "\\%x ", *c);
                } else {
                    g_string_append_c(str, *c);
                }
        }
    }

    g_string_append_c(str, '"');
}

// TODO docustring
GString* uri_from_path(const ls_game* game, const char* source_path)
{
	if (!game || !source_path) {
		return NULL;
	}

	const char* uri_raw = resolve_icon_uri(game, source_path);
	if (!uri_raw) {
		return NULL;
	}

	GString* uri_complete = g_string_new("url(");

	append_quoted_uri(uri_complete, uri_raw);
	g_string_append_c(uri_complete, ')');

	free((void*)uri_raw);

	return uri_complete;
}
