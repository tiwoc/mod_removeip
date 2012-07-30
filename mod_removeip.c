/* Copyright 2006-2012 Andrew McNaughton <andrew@scoop.co.nz>,
                       Daniel Seither <post@tiwoc.de>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "httpd.h"
#include "http_config.h"

module AP_MODULE_DECLARE_DATA removeip_module;

typedef struct {
    int enable;
    int shorten;
} removeip_server_cfg;

static void *removeip_create_server_cfg(apr_pool_t *p, server_rec *s) {
    removeip_server_cfg *cfg = (removeip_server_cfg *)
                               apr_pcalloc(p, sizeof(removeip_server_cfg));
    if (!cfg)
        return NULL;

    cfg->enable = 0;
    cfg->shorten = 0;

    return (void *)cfg;
}

static const char *removeip_enable(cmd_parms *cmd, void *dummy, int flag) {
    server_rec *s = cmd->server;
    removeip_server_cfg *cfg = (removeip_server_cfg *)ap_get_module_config(
                               s->module_config, &removeip_module);

    cfg->enable = flag;
    return NULL;
}

static const char *removeip_shorten(cmd_parms *cmd, void *dummy, int flag) {
    server_rec *s = cmd->server;
    removeip_server_cfg *cfg = (removeip_server_cfg *)ap_get_module_config(
                               s->module_config, &removeip_module);

    cfg->shorten = flag;
    return NULL;
}

static void mask_inet_addr(removeip_server_cfg *cfg, char *ipaddr) {
    if (cfg->shorten) {
        /* set last bytes of remote IP address to .0.1 (keep /16 prefix) */
        ipaddr[2] = 0;
        ipaddr[3] = 1;
    } else {
        /* set whole address to 127.0.0.1 */
        ipaddr[0] = 127;
        ipaddr[1] = 0;
        ipaddr[2] = 0;
        ipaddr[3] = 1;
    }
}

static void mask_inet6_addr(removeip_server_cfg *cfg, char *ipaddr) {
    /* keep /32 prefix if desired, set remainder to ::1 */
    if (cfg->shorten)
        memset(ipaddr+4, 0, 15-4);
    else
        memset(ipaddr, 0, 15);
    ipaddr[15] = 1;
}

static int change_remote_ip(request_rec *r) {
    removeip_server_cfg *cfg = (removeip_server_cfg *)ap_get_module_config(
                               r->server->module_config, &removeip_module);

    if (!cfg->enable)
        return DECLINED;

	/* modify address struct */
    switch (r->connection->remote_addr->family) {
        case APR_INET:
            /* IPv4 */
            mask_inet_addr(cfg, r->connection->remote_addr->ipaddr_ptr);
            break;

        case APR_INET6:
            if (IN6_IS_ADDR_V4MAPPED((struct in6_addr *)
                    r->connection->remote_addr->ipaddr_ptr))

                /* IPv4 address mapped to an IPv6 address:
                   skip prefix 0:0:0:0:0:ffff, only consider IPv4 part */
                mask_inet_addr(cfg, r->connection->remote_addr->ipaddr_ptr+12);
            else
            	/* IPv6 */
                mask_inet6_addr(cfg, r->connection->remote_addr->ipaddr_ptr);
            break;
    }
    
    /* update address string */
    apr_sockaddr_ip_get(&r->connection->remote_ip, r->connection->remote_addr);

    return DECLINED;
}

static const command_rec removeip_cmds[] = {
    AP_INIT_FLAG(
                 "RemoveIpEnable",
                 removeip_enable,
                 NULL,
                 RSRC_CONF,
                 "Enable mod_removeip"
                 ),
    AP_INIT_FLAG(
                 "RemoveIpShortenOnly",
                 removeip_shorten,
                 NULL,
                 RSRC_CONF,
                 "Enable IP address shortening"
                 ),
    { NULL }
};

static void register_hooks(apr_pool_t *p) {
    ap_hook_post_read_request(change_remote_ip, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA removeip_module = {
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    removeip_create_server_cfg,
    NULL,
    removeip_cmds,
    register_hooks,
};
