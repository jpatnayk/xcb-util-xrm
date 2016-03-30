/*
 * vim:ts=4:sw=4:expandtab
 *
 * Copyright © 2016 Ingo Bürk
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 *
 */
#ifndef __XCB_XRM_H__
#define __XCB_XRM_H__

#include <stdbool.h>
#include <xcb/xcb.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup xcb_xrm_database_t XCB XRM Functions
 *
 * These functions are the xcb equivalent of the Xrm* function family in Xlib.
 * They allow the parsing and matching of X resources as well as some utility
 * functions.
 *
 * Here is an example of how this library can be used to retrieve a
 * user-configured resource:
 * @code
 * char *value;
 *
 * int screennr;
 * xcb_connection_t *conn = xcb_connect(NULL, &screennr);
 * if (conn == NULL || xcb_connection_has_error(conn))
 *     err(EXIT_FAILURE, "Could not connect to the X server.");
 *
 * xcb_screen_t *screen = xcb_aux_get_screen(conn, screennr);
 *
 * xcb_xrm_database_t *database = xcb_xrm_database_from_resource_manager(conn, screen);
 *
 * xcb_xrm_resource_t *resource;
 * if (xcb_xrm_resource_get(database, "Xft.dpi", "Xft.dpi", &resource) < 0) {
 *     // Resource not found in database
 *     value = NULL;
 * } else {
 *     value = strdup(xcb_xrm_resource_value(resource));
 * }
 *
 * xcb_xrm_resource_free(resource);
 * xcb_xrm_database_free(database);
 * xcb_disconnect(conn);
 * @endcode
 *
 * @{
 */

/**
 * @struct xcb_xrm_database_t
 * Reference to a database.
 *
 * The database can be loaded in different ways, e.g., from the
 * RESOURCE_MANAGER property by using @ref
 * xcb_xrm_database_from_resource_manager (). All queries for a resource go
 * against a specific database. A database must always be free'd by using @ref
 * xcb_xrm_database_free ().
 */
typedef struct xcb_xrm_database_t xcb_xrm_database_t;

/**
 * @struct xcb_xrm_resource_t
 * Describes a resource.
 *
 * This struct holds a resource after loading it from the database, e.g., by
 * calling @ref xcb_xrm_resource_get (). Its value can be retrieved using @ref
 * xcb_xrm_resource_value () or the utility functions for conversion. A
 * resource must always be free'd by calling @ref xcb_xrm_resource_free () on
 * it.
 */
typedef struct xcb_xrm_resource_t xcb_xrm_resource_t;

/**
 * Loads the RESOURCE_MANAGER property and creates a database with its
 * contents. If the database could not be created, this function will return
 * NULL.
 *
 * @param conn A working XCB connection.
 * @param screen The xcb_screen_t* screen to use.
 * @returns The database described by the RESOURCE_MANAGER property.
 *
 * @ingroup xcb_xrm_database_t
 */
xcb_xrm_database_t *xcb_xrm_database_from_resource_manager(xcb_connection_t *conn, xcb_screen_t *screen);

/**
 * Creates a database from the given string.
 * If the database could not be created, this function will return NULL.
 *
 * @param str The resource string.
 * @returns The database described by the resource string.
 *
 * @ingroup xcb_xrm_database_t
 */
xcb_xrm_database_t *xcb_xrm_database_from_string(const char *str);

/**
 * Creates a database from a given file.
 * If the file cannot be found or opened, NULL is returned.
 *
 * @param filename Valid filename.
 * @returns The database described by the file's contents.
 */
xcb_xrm_database_t *xcb_xrm_database_from_file(const char *filename);

/**
 * Returns a string representation of a database.
 * The string is owned by the caller and must be free'd.
 *
 * @param database The database to return in string format.
 * @returns A string representation of the specified database.
 */
char *xcb_xrm_database_to_string(xcb_xrm_database_t *database);

/**
 * Combines two databases.
 * The entries from the source database are stored in the target database. If
 * the same specifier already exists in the target database, the value will be
 * overridden if override is set; otherwise, the value is discarded.
 * The source database will implicitly be free'd and must not be used
 * afterwards.
 * If NULL is passed for target_db, a new and empty database will be created
 * and returned in the pointer.
 *
 * @param source_db Source database.
 * @param target_db Target database.
 * @param override If true, entries from the source database override entries
 * in the target database using the same resource specifier.
 */
void xcb_xrm_database_combine(xcb_xrm_database_t *source_db, xcb_xrm_database_t **target_db, bool override);

/**
 * Inserts a new resource into the database.
 * If the resource already exists, the current value will be replaced.
 * If NULL is passed for database, a new and empty database will be created and
 * returned in the pointer.
 *
 * Note that this is not the equivalent of @ref
 * xcb_xrm_database_put_resource_line when concatenating the resource name and
 * value with a colon. For example, if the value starts with a leading space,
 * this must (and will) be replaced with the special '\ ' sequence.
 *
 * @param database The database to modify.
 * @param resource The fully qualified or partial resource specifier.
 * @param value The value of the resource.
 */
void xcb_xrm_database_put_resource(xcb_xrm_database_t **database, const char *resource, const char *value);

/**
 * Inserts a new resource into the database.
 * If the resource already exists, the current value will be replaced.
 * If NULL is passed for database, a new and empty database will be created and
 * returned in the pointer.
 *
 * @param database The database to modify.
 * @param line The complete resource specification to insert.
 */
void xcb_xrm_database_put_resource_line(xcb_xrm_database_t **database, const char *line);

/**
 * Destroys the given database.
 *
 * @param database The database to destroy.
 *
 * @ingroup xcb_xrm_database_t
 */
void xcb_xrm_database_free(xcb_xrm_database_t *database);

/**
 * Fetches a resource from the database.
 *
 * @param database The database to use.
 * @param res_name The fully qualified resource name.
 * @param res_class The fully qualified resource class. Note that this argument
 * may be left empty / NULL, but if given, it must contain the same number of
 * components as the resource name.
 * @param resource A pointer to a xcb_xrm_resource_t* which will be modified to
 * contain the matched resource. Note that this resource must be free'd by the
 * caller.
 * @return 0 on success, a negative error code otherwise.
 */
int xcb_xrm_resource_get(xcb_xrm_database_t *database, const char *res_name, const char *res_class,
                         xcb_xrm_resource_t **resource);

/**
 * Returns the string value of the resource.
 * The string is owned by the caller and must be free'd.
 *
 * @param resource The resource to use.
 * @returns The string value of the given resource.
 */
char *xcb_xrm_resource_value(xcb_xrm_resource_t *resource);

/**
 * Returns the long value of the resource.
 * If the value cannot be converted to a long, LONG_MIN is returned.
 *
 * @param resource The resource to use.
 * @returns The long value of the given resource.
 */
long xcb_xrm_resource_value_long(xcb_xrm_resource_t *resource);

/**
 * Returns the bool value of the resource.
 * This function works by checking the following things in this order:
 *  - If the value can be converted to a long, the result will be the
 *    truthiness of the converted number.
 *  - If the value is one of "true", "on" or "yes" (case-insensitive), true is
 *    returned.
 *  - If the value is one of "false", "off" or "no" (case-insensitive), false
 *    is returned.
 *  - Otherwise, false is returned.
 *
 * @param resource The resource to use.
 * @returns The bool value of the given resource.
 */
bool xcb_xrm_resource_value_bool(xcb_xrm_resource_t *resource);

/**
 * Destroy the given resource.
 *
 * @param resource The resource to destroy.
 */
void xcb_xrm_resource_free(xcb_xrm_resource_t *resource);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __XCB_XRM_H__ */
