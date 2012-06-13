/**
 * @file   test_lwdb.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sat Jun  9 20:02:38 2012
 * 
 * @brief  Lwqq Database test module
 * 
 * 
 */

#include "login.h"
#include "logger.h"
#include "info.h"
#include "smemory.h"
#include "lwdb.h"

int main(int argc, char *argv[])
{
    LwdbGlobalDB *gdb = NULL;
    LwdbUserDB *udb = NULL;
    LwdbGlobalUserEntry *e = NULL;

    lwdb_init();
    gdb = lwdb_globaldb_new();
    if (!gdb) {
        lwqq_log(LOG_ERROR, "error\n");
        goto done;
    }
    e = gdb->query_user_info(gdb, "3456345");
    if (!e) {
        gdb->add_new_user(gdb, "3456345");
        e = gdb->query_user_info(gdb, "3456345");
        if (!e) {
            goto done;
        }
    }

    udb = lwdb_userdb_new("3456345");
    if (!udb) {
        goto done;
    }
    
    lwqq_log(LOG_NOTICE, "%s,%s,%s,%s,%s\n",
             e->qqnumber, e->db_name, e->password, e->status, e->rempwd);
done:
    lwdb_globaldb_free_user_entry(e);
    lwdb_globaldb_free(gdb);
    lwdb_userdb_free(udb);
    lwdb_final();
    return 0;
}
