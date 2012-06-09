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
    LwdbGlobalDB *db = NULL;
    LwdbGlobalUserEntry *e = NULL;
    
    db = lwdb_globaldb_new("/tmp/test.db");
    if (!db) {
        lwqq_log(LOG_ERROR, "error\n");
        goto done;
    }
    db->add_new_user(db, "3456345");
    e = db->get_user_info(db, "3456345");
    if (!e) {
        goto done;
    }

    lwqq_log(LOG_NOTICE, "%s,%s,%s,%s,%s\n",
             e->number, e->db_name, e->password, e->status, e->rempwd);
done:
    lwdb_globaldb_free_user_entry(e);
    lwdb_globaldb_free(db);
    return 0;
}
