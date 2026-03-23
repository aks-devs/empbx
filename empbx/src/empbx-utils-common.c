/**
 **
 ** (C)2026 akstel.org
 **/
#include <empbx.h>
#include <pwd.h>
#include <grp.h>

empbx_status_t empbx_dir_create_ifne(char *dir) {
    empbx_status_t status = EMPBX_STATUS_SUCCESS;

    if(!empbx_dir_exists(dir)) {
        status = empbx_dir_create(dir, true);
    }
    if(status != EMPBX_STATUS_SUCCESS) {
        re_fprintf(stderr, "ERROR: Unable to create directory (%s)\n", dir);
    }
    return status;
}

empbx_status_t empbx_switch_ug(char *user, char *group) {
    struct passwd *pw = NULL;
    struct group *gr = NULL;
    uid_t uid = 0;
    gid_t gid = 0;

    if(user) {
        if((pw = getpwnam(user)) == NULL) {
            re_fprintf(stderr,"ERROR: Unknown user: %s\n", user);
            return EMPBX_STATUS_FALSE;
        }
        uid = pw->pw_uid;
    }
    if(group) {
        if((gr = getgrnam(group)) == NULL) {
            re_fprintf(stderr,"ERROR: Unknown group: %s\n", group);
            return EMPBX_STATUS_FALSE;
        }
        gid = gr->gr_gid;
    }

    if(uid && getuid() == uid && (!gid || gid == getgid())) {
        return EMPBX_STATUS_SUCCESS;
    }

    // set GID
#ifdef HAVE_SETGROUPS
    if(setgroups(0, NULL) < 0) {
        re_fprintf(stderr, "ERROR: setgroups() failed\n");
        return EMPBX_STATUS_FALSE;
    }
#endif
    if(gid) {
        if(setgid(gid) < 0) {
            re_fprintf(stderr, "ERROR: setgid() failed\n");
            return EMPBX_STATUS_FALSE;
        }
    } else {
        if (setgid(pw->pw_gid) < 0) {
            re_fprintf(stderr, "ERROR: setgid() failed\n");
            return EMPBX_STATUS_FALSE;
        }
#ifdef HAVE_INITGROUPS
        if (initgroups(pw->pw_name, pw->pw_gid) < 0) {
            re_fprintf(stderr, "ERROR: initgroups() failed\n");
            return EMPBX_STATUS_FALSE;
        }
#endif
    }

    // set UID
    if(setuid(uid) < 0) {

        re_fprintf(stderr, "ERROR: setuid() failed\n");
        return EMPBX_STATUS_FALSE;
    }

    return EMPBX_STATUS_SUCCESS;
}

bool empbx_str2bool(const char *str) {
    if(zstr(str)) {
        return false;
    }
    if(!str_casecmp(str, "0")) {
        return false;
    } else if (!str_casecmp(str, "1")) {
        return true;
    } else if (!str_casecmp(str, "false")) {
        return false;
    } else if (!str_casecmp(str, "true")) {
        return true;
    } else if (!str_casecmp(str, "disable")) {
        return false;
    } else if (!str_casecmp(str, "enable")) {
        return true;
    } else if (!str_casecmp(str, "off")) {
        return false;
    } else if (!str_casecmp(str, "on")) {
        return true;
    } else if (!str_casecmp(str, "no")) {
        return false;
    } else if (!str_casecmp(str, "yes")) {
        return true;
    }

    return false;
}
