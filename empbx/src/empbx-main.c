/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>
#define main_goto_err(_status, _label) err = _status; goto _label

empbx_global_t *global;

static void signal_handler(int sig) {
    if(global->fl_shutdown) { return; }
    switch(sig) {
        case SIGHUP:
            break;
        case SIGALRM:
            break;
        case SIGINT:
        case SIGTERM:
            global->fl_shutdown = true;
            re_cancel();
            break;
    }
}

static void usage(void) {
    (void)re_fprintf(stdout,"Usage: empbxd [options]\n"
        "options:\n"
        "\t-a <action>      start|stop|debug\n"
        "\t-h <path>        home path (default: /opt/empbx)\n"
        "\t-u username      start daemon with a specified user\n"
        "\t-g groupname     start daemon with a specified group\n"
        "\t?                this help\n"
    );
}


// -------------------------------------------------------------------------------------------------------------------------------
// public
// -------------------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    daemon_action_t action = 0;
    int opt = 0, err = 0;
    bool fl_del_pid = false;
    pid_t srv_pid = 0;

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    sys_coredump_set(true);

    if((err = libre_init()) != 0) {
        re_fprintf(stderr, "libre_init() (%m)\n", err);
        main_goto_err(-1, out);
    }

    if(empbx_init(&global) != EMPBX_STATUS_SUCCESS) {
        re_fprintf(stderr, "empbx_init()\n");
        main_goto_err(-1, out);
    }

    while((opt = getopt(argc, argv, "a:h:u:g:?")) != -1) {
        switch (opt) {
            case 'a':
                if(!str_casecmp(optarg, "start")) {
                    action = D_ACTION_START;
                } else if(!str_casecmp(optarg, "stop")) {
                    action = D_ACTION_STOP;
                } else if(!str_casecmp(optarg, "debug")) {
                    action = D_ACTION_START;
                    global->fl_debug_enabled = true;
                }
                break;
            case 'h':
                if(str_dup(&global->path_home, optarg) != LIBRE_SUCCESS) {
                    print_mem_fail();
                    main_goto_err(-1, out);
                }
                break;
            case 'u':
                if(str_dup(&global->xuser, optarg) != LIBRE_SUCCESS) {
                    print_mem_fail();
                    main_goto_err(-1, out);
                }
                break;
            case 'g':
                if(str_dup(&global->xgroup, optarg) != LIBRE_SUCCESS) {
                    print_mem_fail();
                    main_goto_err(-1, out);
                }
                break;
            case '?':
                action = D_ACTION_NONE;
                break;
        }
    }

    if(action == D_ACTION_NONE) {
        usage();
        main_goto_err(0, out);
    }

    if(zstr(global->path_home)) {
        if(re_sdprintf(&global->path_home, DEFAULT_HOME) != LIBRE_SUCCESS) {
            print_mem_fail();
            main_goto_err(-1, out);
        }
    }
    if(re_sdprintf(&global->path_config, "%s/configs", global->path_home) != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }
    if(re_sdprintf(&global->path_sounds, "%s/sounds", global->path_home) != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }
    if(re_sdprintf(&global->path_var, "%s/var", global->path_home) != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }
    if(re_sdprintf(&global->path_www, "%s/www", global->path_home) != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }
    if(re_sdprintf(&global->path_tmp, "/tmp") != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }
    if(re_sdprintf(&global->file_config, "%s/%s", global->path_config, APP_CONFIG_NAME) != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }
    if(re_sdprintf(&global->file_pid, "%s/%s", global->path_var, APP_PID_NAME) != LIBRE_SUCCESS) {
        print_mem_fail();
        main_goto_err(-1, out);
    }

    if(action == D_ACTION_STOP)  {
        srv_pid = empbx_pid_read(global->file_pid);
        if(!srv_pid) {
            re_fprintf(stderr, "ERROR: Server is not running\n");
            main_goto_err(-1, out);
        }
        if(kill(srv_pid, SIGTERM) != 0) {
            re_fprintf(stderr, "ERROR: Unable to send a signel to the server (pid=%d)\n", srv_pid);
            main_goto_err(-1, out);
        }
        goto out;
    }

    if(global->xuser) {
        if(empbx_switch_ug(global->xuser, global->xgroup) != EMPBX_STATUS_SUCCESS) {
            re_fprintf(stderr, "ERROR: Unable to switch to user/group = %s:%s\n", global->xuser, global->xgroup);
            main_goto_err(-1, out);
        }
    }

    if(empbx_dir_create_ifne(global->path_home) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }
    if(empbx_dir_create_ifne(global->path_config) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }
    if(empbx_dir_create_ifne(global->path_sounds) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }
    if(empbx_dir_create_ifne(global->path_var) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }
    if(empbx_dir_create_ifne(global->path_www) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }
    if(empbx_dir_create_ifne(global->path_tmp) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }

    srv_pid = empbx_pid_read(global->file_pid);
    if(srv_pid && srv_pid != getpid()) {
        re_fprintf(stderr,"ERROR: Server already running (pid=%d)\n", srv_pid);
        main_goto_err(-1, out);
    }

    if(!global->fl_debug_enabled) {
        if((err = sys_daemon()) != LIBRE_SUCCESS) {
            re_fprintf(stderr,"ERROR: Unable to daemonize! (%m)\n", err);
            main_goto_err(-1, out);
        }
    }

    empbx_log_init(!global->fl_debug_enabled);

    srv_pid = getpid();
    if(empbx_pid_write(global->file_pid, srv_pid) != EMPBX_STATUS_SUCCESS) {
        log_error("Unable to write PID (%s)", global->file_pid);
        main_goto_err(-1, out);
    }
    fl_del_pid = true;

    if(empbx_load_config(global->file_config) != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }

    if(empbx_service_mgmt_start() != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }
    if(empbx_service_sip_start() != EMPBX_STATUS_SUCCESS) {
        main_goto_err(-1, out);
    }

    global->fl_ready = true;
    global->fl_shutdown = false;

    log_notice("empbxd-%s [pid:%d / uid:%d / gid:%d / instance:%s] - started", APP_VERSION, srv_pid, getuid(), getgid(), global->instance_id);
    log_notice("(C)2026 akstel.org");

    if((err = re_main(signal_handler)) != LIBRE_SUCCESS) {
        log_error("re_main() fail (%i)", err);
        err = -1;
    }

    global->fl_ready = false;
    global->fl_shutdown = true;

    empbx_service_mgmt_stop();
    empbx_service_sip_stop();

    if(global->active_threds > 0) {
        log_warn("Waiting for termination (%i) threads...", global->active_threds);
        while(global->active_threds > 0) {
            sys_msleep(1000);
        }
    }

out:
    if(fl_del_pid) {
        empbx_pid_delete(global->file_pid);
    }

    if(global) {
        global = mem_deref(global);
    }

#ifdef EMPBX_DEBUG
    tmr_debug();
#endif

    libre_close();

#ifdef EMPBX_DEBUG
    mem_debug();
#endif

    if(action == D_ACTION_START) {
        if(err == 0) {
            log_notice("empbx-%s [pid:%d] - stopped", APP_VERSION, srv_pid);
        }
    }

    empbx_log_shutdown();
    return err;
}
