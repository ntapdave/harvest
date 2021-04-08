#ifndef __SDV3_API_H__
#define __SDV3_API_H__

#include <assert.h>
#include "nfs4_prot.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
#ifdef _SD_API_DLL
            #undef _SD_API_DLL
#endif
#ifdef SD_API
            #define _SD_API_DLL __declspec(dllexport)
#else
            #define _SD_API_DLL __declspec(dllimport)
#endif
#else
            #define _SD_API_DLL
#endif

#define RPC_STATUS(sd_rpc_resp)         (sd_rpc_resp->status)
#define RPC_OK(sd_rpc_resp)		(sd_rpc_resp->sd_res_out.sd_ok)
#define RPC_API(sd_rpc_resp)		(RPC_OK(sd_rpc_resp).sd_api)
#define RPC_NEXT_RESP_V1(sd_rpc_resp)	(RPC_OK(sd_rpc_resp).sd_out_ok.next_res_V1)
#define RPC_NEXT_RESP_V2(sd_rpc_resp)   (RPC_OK(sd_rpc_resp).sd_out_ok.next_res_V2)
#define RPC_STATUS_RESP(sd_rpc_resp)	(RPC_OK(sd_rpc_resp).sd_out_ok.status_res)
#define RPC_DISCONNECT_RESP(sd_rpc_resp)	(RPC_OK(sd_rpc_resp).sd_out_ok.disconnect_res)

// MACROS to access version specific structures.
#define SCR_INODE_NUM(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_inode_num: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_inode_num: \
                0))

#define SCR_CHANGE_TYPE(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_change_type: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_change_type: \
                0))

#define SCR_FTYPE(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_ftype: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_ftype: \
                0))

#define SCR_CRTIME(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_crtime: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_crtime: \
                0))

#define SCR_MTIME(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_mtime: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_mtime: \
                0))

#define SCR_ATIME(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_atime: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_atime: \
                0))

#define SCR_CTIME(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_ctime: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_ctime: \
                0))

#define SCR_OWNER(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_owner: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_owner: \
                0))

#define SCR_GROUP(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_group: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_group: \
                0))

#define SCR_FATTR(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_fattr: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_fattr: \
                0))

#define SCR_DOS_BITS(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_dos_bits: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_dos_bits: \
                0))

#define SCR_SIZE(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_size: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_size: \
                0))

#define SCR_LINKS(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_links: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_links: \
                0))

#define SCR_IS_IOFENCED(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_is_iofenced: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_is_iofenced: \
                0))

#define SCR_NAME_VAL(sd_rpc_resp, index) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->crp.crp_val[index].scr_name.scr_name_val: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->crp.crp_val[index].scr_name.scr_name_val: \
                NULL))

#define RPC_NEXT_NUM_CHANGES(sd_rpc_resp) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->num_changes: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->num_changes: \
                0))

#define RPC_NEXT_EOF(sd_rpc_resp) \
        ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V1) ? \
            RPC_NEXT_RESP_V1(sd_rpc_resp)->eof: \
            ((RPC_API(sd_rpc_resp) == SD_NEXT_API_V2) ? \
                RPC_NEXT_RESP_V2(sd_rpc_resp)->eof: \
                0))

#define SD_SUCCESS 0
#define SD_ERROR -1
#define DISCONNECT_MAX_RETRIES 10

typedef void* sd_connect_handle_t;
typedef void* sd_rpc_cookie_t;

typedef enum sd_rpc_api {
        SD_NEXT_API_V1=1,
        SD_NEXT_API_V2,
        SD_STATUS_API,
        SD_DISCONNECT_API,
} sd_rpc_api_t;

typedef struct sd_rpc_ok {
        sd_rpc_cookie_t sd_rpc_cookie;
        sd_rpc_api_t    sd_api;
        union {
                SD_NEXT4resok_V1 *next_res_V1;
                SD_NEXT4resok_V2 *next_res_V2;
                SD_STATUS4resok *status_res;
                SD_DISCONNECT4resok *disconnect_res;
        }sd_out_ok;
} sd_rpc_ok_t;

typedef struct sd_rpc_resp {
        nfsstat4 status;
        union {
                sd_rpc_ok_t sd_ok;
                const char  *errstr;
        }sd_res_out;
}sd_rpc_resp_t;

typedef struct sd_connect_args {
        char *ip_addr;
        char *session_handle;
        char *session_id;
} sd_connect_args_t;



/*******************************************************************/
_SD_API_DLL int32_t sd_rpc_connect_v3(const sd_connect_args_t *sd_connect_args,
                                      sd_connect_handle_t *sd_connect);

_SD_API_DLL int32_t sd_rpc_next_v3(const sd_connect_handle_t sd_connect,
                                   sd_rpc_resp_t **sd_next_res);

_SD_API_DLL int32_t sd_rpc_status_v3(const sd_connect_handle_t sd_connect,
                                     sd_rpc_resp_t **sd_status_res);

_SD_API_DLL int32_t sd_rpc_disconnect_v3(sd_connect_handle_t sd_connect,
				         sd_rpc_resp_t **sd_disconnect_resp);

_SD_API_DLL int print_sd_output(const sd_rpc_resp_t *sd_resp);

_SD_API_DLL void sd_rpc_free_resp(sd_rpc_resp_t **sd_rpc_resp);


#ifdef __cplusplus
}
#endif
#endif

