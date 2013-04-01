#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <glib.h>
#include <gio/gio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <config.h>

#include <xig.h>

/* For some reason sys/un.h doesn't define this */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

/* Timeout in ms waiting for the status we expect */
#define STATUS_TIMEOUT 2000

/* Timeout in ms to wait for SIGTERM to kill compiz */
#define KILL_TIMEOUT 2000

static XigServer *xserver;
static GKeyFile *config;
static GPid compiz_pid = 0;
static GList *statuses = NULL;
static GList *script = NULL;
static GList *script_iter = NULL;
static guint status_timeout = 0;
static gboolean failed = FALSE;
static guint compiz_kill_timeout = 0;

static void check_status (const gchar *status);

static gboolean
compiz_kill_timeout_cb (gpointer data)
{
    if (getenv ("DEBUG"))
        g_print ("Sending SIGKILL to compiz\n");
    kill (compiz_pid, SIGKILL);
    compiz_kill_timeout = 0;
    return FALSE;
}

static void
stop_compiz ()
{
    if (compiz_pid && compiz_kill_timeout == 0)
    {
        if (getenv ("DEBUG"))
            g_print ("Sending SIGTERM to compiz process %d\n", compiz_pid);
        kill (compiz_pid, SIGINT);
        compiz_kill_timeout = g_timeout_add (KILL_TIMEOUT, compiz_kill_timeout_cb, NULL);
    } 
}

static void
restart_compiz ()
{
    if (compiz_pid)
    {
        if (getenv ("DEBUG"))
            g_print ("Sending SIGHUP to compiz process %d\n", compiz_pid);
        kill (compiz_pid, SIGHUP);
    }
}

static void
quit (int status)
{
    if (xserver)
        xig_server_stop (xserver);
    if (compiz_pid)
        kill (compiz_pid, SIGKILL);

    exit (status);
}

static void
fail (const gchar *event, const gchar *expected)
{
    GList *link;

    if (failed)
        return;
    failed = TRUE;

    g_printerr ("Test failed, got the following events:\n");
    for (link = statuses; link; link = link->next)
        g_printerr ("    %s\n", (gchar *)link->data);
    if (event)
        g_printerr ("    %s\n", event);
    if (expected)
        g_printerr ("    ^^^ expected \"%s\"\n", expected);
    else
        g_printerr ("^^^ expected nothing\n");

    /* Either wait for the compiz to quit, or stop now if it already is */
    if (compiz_pid)
        stop_compiz ();
    else
        quit (EXIT_FAILURE);
}

static gchar *
get_script_line ()
{
    if (!script_iter)
        return NULL;
    return script_iter->data;
}

static void
compiz_exit_cb (GPid pid, gint status, gpointer data)
{
    gchar *status_text;

    compiz_pid = 0;
  
    if (compiz_kill_timeout)
        g_source_remove (compiz_kill_timeout);
    compiz_kill_timeout = 0;

    /* Quit when compiz does */
    if (failed)
        quit (EXIT_FAILURE);
 
    if (WIFEXITED (status))
        status_text = g_strdup_printf ("COMPIZ EXIT STATUS=%d", WEXITSTATUS (status));
    else
        status_text = g_strdup_printf ("COMPIZ TERMINATE SIGNAL=%d", WTERMSIG (status));
    check_status (status_text);
    g_free (status_text);
}

static void
unmap_notify_cb (XigWindow *window)
{
    gchar *status_text;

    status_text = g_strdup_printf ("UNMAP-NOTIFY ID=%d", xig_window_get_id (window));
    check_status (status_text);
    g_free (status_text);
}

static void
map_notify_cb (XigWindow *window)
{
    gchar *status_text;

    status_text = g_strdup_printf ("MAP-NOTIFY ID=%d", xig_window_get_id (window));
    check_status (status_text);
    g_free (status_text);
}

static void
reparent_notify_cb (XigWindow *window)
{
    gchar *status_text;

    status_text = g_strdup_printf ("REPARENT-NOTIFY ID=%d", xig_window_get_id (window));
    check_status (status_text);
    g_free (status_text);
}

static void
configure_notify_cb (XigWindow *window)
{
    gchar *status_text;

    status_text = g_strdup_printf ("CONFIGURE-NOTIFY ID=%d", xig_window_get_id (window));
    check_status (status_text);
    g_free (status_text);
}

static void
run_commands ()
{
    /* Stop compiz if requested */
    while (TRUE)
    {
        gchar *command, *name = NULL, *c;
        GHashTable *params;

        command = get_script_line ();
        if (!command)
            break;

        /* Commands start with an asterisk */
        if (command[0] != '*')
            break;
        statuses = g_list_append (statuses, g_strdup (command));
        script_iter = script_iter->next;

        c = command + 1;
        while (*c && !isspace (*c))
            c++;
        name = g_strdup_printf ("%.*s", (int) (c - command - 1), command + 1);

        params = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
        while (TRUE)
        {
            gchar *start, *param_name, *param_value;
          
            while (isspace (*c))
                c++;
            start = c;
            while (*c && !isspace (*c) && *c != '=')
                c++;
            if (*c == '\0')
                break;

            param_name = g_strdup_printf ("%.*s", (int) (c - start), start);

            if (*c == '=')
            {
                c++;
                while (isspace (*c))
                    c++;
                if (*c == '\"')
                {
                    gboolean escaped = FALSE;
                    GString *value;

                    c++;
                    value = g_string_new ("");
                    while (*c)
                    {
                        if (*c == '\\')
                        {
                            if (escaped)
                            {
                                g_string_append_c (value, '\\');
                                escaped = FALSE;
                            }
                            else
                                escaped = TRUE;
                        }
                        else if (!escaped && *c == '\"')
                            break;
                        if (!escaped)
                            g_string_append_c (value, *c);
                        c++;
                    }
                    param_value = value->str;
                    g_string_free (value, FALSE);
                    if (*c == '\"')
                        c++;
                }
                else
                {
                    start = c;
                    while (*c && !isspace (*c))
                        c++;
                    param_value = g_strdup_printf ("%.*s", (int) (c - start), start);
                }
            }
            else
                param_value = g_strdup ("");

            g_hash_table_insert (params, param_name, param_value);
        }

        if (strcmp (name, "STOP-XSERVER") == 0)
            xig_server_stop (xserver);
        else if (strcmp (name, "STOP-COMPIZ") == 0)
            stop_compiz ();
        else if (strcmp (name, "RESTART-COMPIZ") == 0)
            restart_compiz ();
        else if (strcmp (name, "CREATE-WINDOW") == 0)
        {
            XigWindow *root, *window;
            gchar *v;
            guint32 id = 0;
            gint16 x = 0, y = 0;
            guint16 width = 0, height = 0, border_width = 0;

            v = g_hash_table_lookup (params, "ID");
            if (v)
                id = atoi (v);
            v = g_hash_table_lookup (params, "X");
            if (v)
                x = atoi (v);
            v = g_hash_table_lookup (params, "Y");
            if (v)
                y = atoi (v);
            v = g_hash_table_lookup (params, "WIDTH");
            if (v)
                width = atoi (v);
            v = g_hash_table_lookup (params, "HEIGHT");
            if (v)
                height = atoi (v);
            v = g_hash_table_lookup (params, "BORDER-WIDTH");
            if (v)
                border_width = atoi (v);

            root = xig_server_get_root (xserver, 0);
            window = xig_window_add_child (root, NULL, id,
                                           XIG_WINDOW_CLASS_InputOutput,
                                           x, y, width, height, border_width,
                                           xig_window_get_visual (root),
                                           0, NULL, 0, NULL, 0, 0, 0, 0, 0, 0, FALSE, FALSE);
            g_signal_connect (window, "unmap-notify", G_CALLBACK (unmap_notify_cb), NULL);
            g_signal_connect (window, "map-notify", G_CALLBACK (map_notify_cb), NULL);
            g_signal_connect (window, "reparent-notify", G_CALLBACK (reparent_notify_cb), NULL);
            g_signal_connect (window, "configure-notify", G_CALLBACK (configure_notify_cb), NULL);
        }
        else if (strcmp (name, "MAP-WINDOW") == 0)
        {
            gchar *v;
            guint32 id = 0;
            XigWindow *window;

            v = g_hash_table_lookup (params, "ID");
            if (v)
                id = atoi (v);

            window = xig_server_get_window (xserver, id);
            xig_window_map (window, NULL);
        }
        else
        {
            g_printerr ("Unknown command '%s'\n", name);
            quit (EXIT_FAILURE);
            return;
        }

        g_free (name);
        g_hash_table_unref (params);
    }

    gchar *l = get_script_line ();

    /* Stop at the end of the script */
    if (l == NULL)
    {
        if (compiz_pid)
            stop_compiz ();
        else
            quit (EXIT_SUCCESS);
    }
}

static gboolean
status_timeout_cb (gpointer data)
{
    fail ("(timeout)", get_script_line ());
    status_timeout = 0;
    return FALSE;
}

static void
check_status (const gchar *status)
{
    gchar *pattern;

    if (getenv ("DEBUG"))
        g_print ("%s\n", status);

    if (failed)
        return;

    statuses = g_list_append (statuses, g_strdup (status));

    /* Try and match against expected */
    pattern = get_script_line ();
    if (!pattern || !g_regex_match_simple (pattern, status, 0, 0))
    {
        fail (NULL, pattern);
        return;
    }
    script_iter = script_iter->next;

    /* Restart timeout */
    if (status_timeout)
        g_source_remove (status_timeout);
    status_timeout = g_timeout_add (STATUS_TIMEOUT, status_timeout_cb, NULL);

    run_commands ();
}

static void
signal_cb (int signum)
{
    if (compiz_pid != 0)
    {
        if (compiz_pid)
        {
            g_print ("Caught signal %d, killing Compiz\n", signum);
            kill (compiz_pid, SIGKILL);
        }
        else
            g_print ("Caught signal %d, quitting\n", signum);      
    }
    else
    {
        g_print ("Caught signal %d, quitting\n", signum);
        quit (EXIT_FAILURE);
    }
}

static void
load_script (const gchar *filename)
{
    int i;
    gchar *data, **lines;

    if (!g_file_get_contents (filename, &data, NULL, NULL))
    {
        g_printerr ("Unable to load script: %s\n", filename);
        quit (EXIT_FAILURE);
    }

    lines = g_strsplit (data, "\n", -1);
    g_free (data);

    /* Load lines with #? prefix as expected behaviour */
    for (i = 0; lines[i]; i++)
    {
        gchar *line = g_strstrip (lines[i]);
        if (g_str_has_prefix (line, "#?"))
            script = g_list_append (script, g_strdup (line+2));
    }
    script_iter = script;
    g_strfreev (lines);
}

static void
client_connected_cb (XigServer *server,
		     XigRemoteClient *client)
{
    check_status ("X CLIENT-CONNECTED");
}

static void
client_disconnected_cb (XigServer *server, XigRemoteClient *client)
{
    check_status ("X CLIENT-DISCONNECTED");
}

static gboolean
client_stdout_cb (GIOChannel   *source,
                  GIOCondition condition,
                  gpointer     data)
{
    gchar *str_return;
    gsize length;
    gsize terminator_pos;
    GError *error;

    if (condition & G_IO_IN)
    {
        while (g_io_channel_read_line (source,
                                       &str_return,
                                       &length,
                                       &terminator_pos,
                                       &error) == G_IO_STATUS_NORMAL)
        {
            if (getenv ("DEBUG"))
                g_print ("%s", str_return);
        }
    }

    return TRUE;
}

int
main (int argc, char **argv)
{
    GMainLoop *loop;
    XigScreen *screen;
    XigVisual *visual;
    gchar *script_name, *config_file, *config_path;
    GString *command_line;
    gchar **compiz_env, **compiz_argv;
    gint compiz_stdin, compiz_stdout, compiz_stderr;
    GIOChannel *compiz_stdout_channel = NULL;
    GError *error = NULL;

    signal (SIGINT, signal_cb);
    signal (SIGTERM, signal_cb);

    g_type_init ();

    loop = g_main_loop_new (NULL, FALSE);

    if (argc != 2)
    {
        g_printerr ("Usage %s SCRIPT-NAME\n", argv[0]);
        quit (EXIT_FAILURE);
    }
    script_name = argv[1];
    config_file = g_strdup_printf ("%s.conf", script_name);
    config_path = g_build_filename (COMPIZ_XIG_TEST_SOURCE_DIR "/scripts", config_file, NULL);
    g_free (config_file);

    config = g_key_file_new ();
    g_key_file_load_from_file (config, config_path, G_KEY_FILE_NONE, NULL);

    load_script (config_path);

    /* Disable config if requested */
    if (g_key_file_has_key (config, "test-runner-config", "have-config", NULL) &&
        !g_key_file_get_boolean (config, "test-runner-config", "have-config", NULL))
        config_path = NULL;

    g_print ("----------------------------------------\n");
    g_print ("Running script %s\n", script_name);

    /* Create an X server to test with */
    xserver = xig_server_new ("compiz-test", 99);
    xig_server_set_listen_tcp (xserver, FALSE);
    g_signal_connect (xserver, "client-connected", G_CALLBACK (client_connected_cb), NULL);
    g_signal_connect (xserver, "client-disconnected", G_CALLBACK (client_disconnected_cb), NULL);
    xig_server_add_pixmap_format (xserver, 1, 1, 32);
    xig_server_add_pixmap_format (xserver, 4, 8, 32);
    xig_server_add_pixmap_format (xserver, 8, 8, 32);
    xig_server_add_pixmap_format (xserver, 15, 16, 32);
    xig_server_add_pixmap_format (xserver, 16, 16, 32);
    xig_server_add_pixmap_format (xserver, 24, 32, 32);
    xig_server_add_pixmap_format (xserver, 32, 32, 32);
    screen = xig_server_add_screen (xserver, 0x00FFFFFF, 0x00000000, 1024, 768, 1024, 768);
    visual = xig_screen_add_visual (screen, 24, XIG_VISUAL_CLASS_TrueColor, 8, 1, 0x00FF0000, 0x0000FF00, 0x000000FF);
    xig_screen_add_root (screen, visual);

    run_commands ();

    status_timeout = g_timeout_add (STATUS_TIMEOUT, status_timeout_cb, NULL);

    if (!xig_server_start (xserver, &error))
    {
        g_printerr ("Failed to start Xig X server: %s", error->message);
        quit (EXIT_FAILURE);
    }

    compiz_env = g_strsplit ("DISPLAY=:99", " ", -1);
    command_line = g_string_new (compiz_BINARY_DIR "/src/compiz");
    g_print ("Start Compiz with command: %s\n", command_line->str);
    if (!g_shell_parse_argv (command_line->str, NULL, &compiz_argv, &error))
    {
        g_warning ("Error parsing command line: %s", error->message);
        quit (EXIT_FAILURE);
    }
    g_clear_error (&error);
    if (!g_spawn_async_with_pipes (NULL, /* working directory */
                                   compiz_argv,
                                   compiz_env,
                                   G_SPAWN_DO_NOT_REAP_CHILD,
                                   NULL, NULL, /* child setup */
                                   &compiz_pid,
                                   &compiz_stdin,
                                   &compiz_stdout,
                                   &compiz_stderr,
                                   &error))
    {
        g_warning ("Error launching Compiz: %s", error->message);
        quit (EXIT_FAILURE);
    }
    g_clear_error (&error);
    g_child_watch_add (compiz_pid, compiz_exit_cb, NULL);
    if (getenv ("DEBUG"))
        g_print ("Compiz running with PID %d\n", compiz_pid);

    compiz_stdout_channel = g_io_channel_unix_new (compiz_stdout);
    g_io_channel_set_flags (compiz_stdout_channel, G_IO_FLAG_NONBLOCK, NULL);
    g_io_add_watch (compiz_stdout_channel, G_IO_IN, client_stdout_cb, NULL);

    check_status ("COMPIZ START");

    g_main_loop_run (loop);

    g_object_unref (compiz_stdout_channel);

    return EXIT_FAILURE;
}
