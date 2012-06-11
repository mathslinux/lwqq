/**
 * @file   info.h
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Sun May 27 19:49:51 2012
 *
 * @brief  Fetch QQ information. e.g. friends information, group information.
 * 
 * 
 */

#ifndef LWQQ_INFO_H
#define LWQQ_INFO_H

#include "type.h"

/** 
 * Get QQ friends information. These information include basic friend
 * information, friends group information, and so on
 * 
 * @param lc 
 * @param err 
 */
void lwqq_info_get_friends_info(LwqqClient *lc, LwqqErrorCode *err);
void lwqq_info_get_groups_info(LwqqClient *lc, LwqqErrorCode *err);

#endif  /* LWQQ_INFO_H */
