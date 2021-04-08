// sdv2_client_for_windows.cpp : Defines the entry point for the console application.
//

#ifndef _WIN32
#include <sys/time.h> /* for gettimeofday */
#include <getopt.h>
#else
#include "getopt.h"
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <netapp_api.h>
#include <errno.h>

#include "sdv2_api.h"

#define DBG_PRINTF(...)         ((void)0)
//#define DBG_PRINTF(...)         (printf(__VA_ARGS__))

typedef enum sd_err_origin {
	ERR_ORIGIN_UNKNOWN = 0,
	ERR_ORIGIN_NPS,
	ERR_ORIGIN_ZAPI,
	ERR_ORIGIN_SDAPI,
	ERR_ORIGIN_SDCLNT
} sd_err_origin;

const char* sd_err_origin_to_string[] = { "UNKNOWN", "NPS", "SDAPI", "SDCLIENT" };
char* vserver_ip = NULL;

static struct option sd_long_options[] =
{
	{ "noatime", no_argument, 0, 'A' },
	{ "nofattr", no_argument, 0, 'F' },
	{ "noi2p", no_argument, 0, 'I' },
	{ "nocheckpoint", no_argument, 0, 'C' },
	{ "debug", no_argument, 0, 'x' },
	{ "ip", required_argument, 0, 'f' },
	{ "volume", required_argument, 0, 'v' },
	{ "basesnap", required_argument, 0, 'b' },
	{ "diffsnap", required_argument, 0, 'd' },
	{ "fileproto", required_argument, 0, 'a' },
	{ "maxdiffs", required_argument, 0, 'm' },
	{ "rescookie", required_argument, 0, 'r' },
	{ "appname", required_argument, 0, 'n' },
	{ "apptype", required_argument, 0, 't' },
	{ "username", required_argument, 0, 'u' },
	{ "password", required_argument, 0, 'p' },
	{ "vserver", required_argument, 0, 's' },
	{ "vserverip", required_argument, 0, 'S' },
	{ 0, 0, 0, 0 }
};



static void sd_print_args(sd_start_args_t *sd_start_args);
static void sd_init_args(sd_start_args_t *sd_start_args);
static int32_t sd_validate_args(sd_start_args_t *sd_start_args);
static int sd_parse_input(int argc, char **argv, sd_start_args_t *sd_start_args);
static void print_usage();
int main(int argc, char **argv);


/*
* Print sd_start_args
*/
static void sd_print_args(sd_start_args_t *sd_start_args)
{
	DBG_PRINTF("IP = %s\n", sd_start_args->ip_addr);
	DBG_PRINTF("Username = %s\n", sd_start_args->username);
	DBG_PRINTF("Password = %s\n", sd_start_args->password);
	DBG_PRINTF("Vserver = %s\n", sd_start_args->vserver);
	DBG_PRINTF("Vserver IP = %s\n", vserver_ip);
	DBG_PRINTF("File access protocol = %s\n", sd_start_args->fileaccessproto);
	DBG_PRINTF("Volume = %s\n", sd_start_args->volname);
	DBG_PRINTF("Basesnap = %s\n", sd_start_args->basesnap);
	DBG_PRINTF("Diffsnap = %s\n", sd_start_args->diffsnap);
	DBG_PRINTF("Maxdiffs = %u\n", sd_start_args->maxdiffs);
	DBG_PRINTF("Atime = %d\n", sd_start_args->atime);
	DBG_PRINTF("Fattrs = %d\n", sd_start_args->fattrs);
	DBG_PRINTF("Checkpointing = %d\n", sd_start_args->checkpointing);
	DBG_PRINTF("I2P = %d\n", sd_start_args->i2p);
	if (sd_start_args->restartcookie) {
		DBG_PRINTF("Restart cookie = %s\n", sd_start_args->restartcookie);
	}
	DBG_PRINTF("Appname = %s\n", sd_start_args->appname);
	DBG_PRINTF("Apptype = %s\n", sd_start_args->apptype);
}

/*
* This will initialize all the sd_start_args
* NOTE: No allocations are done here!
*/
static void sd_init_args(sd_start_args_t *sd_start_args)
{
	sd_start_args->ip_addr = NULL;
	sd_start_args->username = NULL;
	sd_start_args->password = NULL;
	sd_start_args->volname = NULL;
	sd_start_args->diffsnap = NULL;
	sd_start_args->basesnap = NULL;
	sd_start_args->fileaccessproto = "nfs";
	sd_start_args->atime = TRUE;
	sd_start_args->fattrs = TRUE;
	sd_start_args->checkpointing = TRUE;
	sd_start_args->i2p = TRUE;
	sd_start_args->maxdiffs = 0;
	sd_start_args->restartcookie = NULL;
	sd_start_args->appname = "default_name";
	sd_start_args->apptype = "default_type";
	sd_start_args->vserver = NULL;
	vserver_ip = NULL;
}

/*
* This function will perform some
* basic validation on sd_start_args
*
* WARNING: This function does not protect against junk
* values, so be sure to initialize all the arguments
* you send!
*/
int32_t
static sd_validate_args(sd_start_args_t *sd_start_args)
{
	if (sd_start_args == NULL) {
		return SD_ERROR;
	}

	if (!sd_start_args->ip_addr || !sd_start_args->volname ||
		!sd_start_args->vserver || !vserver_ip ||
		!sd_start_args->diffsnap || !sd_start_args->fileaccessproto ||
		!sd_start_args->maxdiffs ||
		!sd_start_args->appname || !sd_start_args->apptype) {
		DBG_PRINTF("sd_validate_start_args: one or more required arguments missing\n");
		return SD_ERROR;
	}

	if (!sd_start_args->username || !sd_start_args->password) {
		DBG_PRINTF("sd_validate_start_args: username/password not specified\n");
		return SD_ERROR;
	}

	return SD_SUCCESS;
}

/*
* This function initializes sd_start_args to default values.
* It then goes through the arg list and fills sd_start_args
* accordingly. Finally, it does some basic validation
* via sd_validate_args.
*/
static int sd_parse_input(int argc, char **argv, sd_start_args_t *sd_start_args)
{
	int opt;
	/*      opterr = 0; //suppress getopt's output */

	//set defaults
	sd_init_args(sd_start_args);

	while ((opt = getopt_long(argc, argv, "ACFIa:b:d:f:m:n:p:r:s:S:t:u:v:", sd_long_options, NULL)) != -1)
	{
		switch (opt) {
		case 'A':
			sd_start_args->atime = FALSE;
			break;

		case 'F':
			sd_start_args->fattrs = FALSE;
			break;

		case 'I':
			sd_start_args->i2p = FALSE;
			break;

		case 'C':
			printf("%s: option -C (disable checkpointing) set\n", argv[0]);
			sd_start_args->checkpointing = FALSE;
			break;

		case 'a':
			sd_start_args->fileaccessproto = optarg;
			break;

		case 'b':
			sd_start_args->basesnap = optarg;
			break;

		case 'd':
			sd_start_args->diffsnap = optarg;
			break;

		case 'm':
			sd_start_args->maxdiffs = atoi(optarg);
			break;

		case 'n':
			sd_start_args->appname = optarg;
			break;

		case 'p':
			sd_start_args->password = optarg;
			break;

		case 'r':
			sd_start_args->restartcookie = optarg;
			break;

		case 't':
			sd_start_args->apptype = optarg;
			break;

		case 'u':
			sd_start_args->username = optarg;
			break;

		case 'v':
			sd_start_args->volname = optarg;
			break;

		case 'f':
			sd_start_args->ip_addr = optarg;
			break;

		case 's':
			sd_start_args->vserver = optarg;
			break;

		case 'S':
			vserver_ip = optarg;
			break;

		case '?':
			/* getopt_long prints an error message anyway */
			/* printf("%s: unknown option '-%c'\n", argv[0], optopt); */
			return SD_ERROR;
			break;

		default:
			printf("%s: getopt failure - returned character code 0%o\n", argv[0], opt);
			return SD_ERROR;
		}
	}

	if (optind < argc) {
		printf("%s: unknown argument(s) specified: ", argv[0]);
		while (optind < argc)
			printf("%s ", argv[optind++]);
		putchar('\n');
		return SD_ERROR;
	}

	return sd_validate_args(sd_start_args);
}

/*
* Print the correct usage of sd-client
*/
static void print_usage()
{
	fprintf(stderr, "USAGE: sdv2_client_for_windows.exe -f <CLUSTERIP> -s <VSERVERNAME> -S <VSERVERIP> -v <VOLNAME> -d <DIFF_SNAP> -m <MAX_DIFFS>\n");
	fprintf(stderr, "                   [-b <BASE_SNAP>] -u|--username USERNAME> -p|--password <PASSWORD>\n");
	fprintf(stderr, "                   [-a <FILE_XS_PROT>] [-A|--noatime] [-F|--nofattr] [-I|-noi2p]\n");
	fprintf(stderr, "                   [-C|--nocheckpointing]\n");
	fprintf(stderr, "                   [-r|--rescookie <RESTART_COOKIE>]\n");
	fprintf(stderr, "                   [-n|--appname <APPNAME>] [-t|--apptype <APPTYPE>]\n\n");
	fprintf(stderr, "       USERNAME, PASSWORD: Cluster credentials for the CLUSTERIP used above.\n");
}

/*
* The hierarchy of this main:
*      parse input args (sd_parse_input)
*      validate input args (sd_validate_start_args)
*      init nps session (sd_rpc_connect)
*      call sd internal apis (sd_*_int)
*      print output, if applicable (print_sd_output)
*      free output allocated by sd internal apis (sd_rpc_free_resp)
*      close nps session (sd_rpc_disconnect)
*/
int main(int argc, char **argv)
{
	uint32_t error = 0;
	int session_inited = 0;
	int status = SD_SUCCESS;
	sd_connect_handle_t sd_connect_hdl = NULL;
	sd_session_handle_t sd_session_hdl = NULL;
	sd_start_args_t sd_start_args;
	char *connect_ip = NULL;
	sd_err_origin err_origin = ERR_ORIGIN_UNKNOWN;
	int iter_next_count = 0;

#ifndef _WIN32
	struct timeval init_start, start, end;
#else
	LARGE_INTEGER init_start, start, end, freq;
#endif
	double total_time = 0, diff_time = 0;
	uint64_t total_diffs = 0;

	printf("%s: sd client v4.0 started\n", argv[0]);
	DBG_PRINTF("%s: parsing input...\n", argv[0]);
	status = sd_parse_input(argc, argv, &sd_start_args);
	if (status != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDCLNT;
		goto errdone;
	}

	/* Print input */
	sd_print_args(&sd_start_args);
	status = sd_validate_args(&sd_start_args);
	if (status != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDCLNT;
		goto errdone;
	}

	connect_ip = vserver_ip;
	DBG_PRINTF("connect_ip: %s\n", connect_ip);

	DBG_PRINTF("%s: Calling CONNECT API\n", argv[0]);

	// SD_CONNECT API
	DBG_PRINTF("%s: calling sd_rpc_connect\n", argv[0]);
	error = sd_rpc_connect(connect_ip, &sd_connect_hdl);
	if (error != 0) {
		DBG_PRINTF("%s: cannot create session, exiting\n", argv[0]);
		err_origin = ERR_ORIGIN_NPS;
		status = SD_ERROR;
		goto errdone;
	}
	else {
		session_inited = 1;
	}

	DBG_PRINTF("%s: Starting API calls\n", argv[0]);

	// SD_START API
	DBG_PRINTF("\n\nCalling sd_rpc_start API\n");
	sd_rpc_resp_t *sd_start_resp = NULL;
	error = sd_rpc_start(sd_connect_hdl, &sd_start_args,
		&sd_session_hdl, &sd_start_resp);
	DBG_PRINTF("%s: returned from sd_rpc_start\n", argv[0]);

	(void)print_sd_output(sd_start_resp);

	if (error != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDAPI;
		status = SD_ERROR;
		sd_rpc_free_resp(&sd_start_resp);
		goto errdone;
	}

	if (sd_start_resp->status != NFS4_OK) {
		sd_rpc_free_resp(&sd_start_resp);
		goto errdone;
	}

	sd_rpc_free_resp(&sd_start_resp);

	// SD_NEXT API
	DBG_PRINTF("\n\nCalling sd_rpc_next API\n");
	sd_rpc_resp_t *sd_next_resp = NULL;

#ifndef _WIN32
	gettimeofday(&init_start, NULL);
#else
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&init_start);
#endif

	while (1) {
		sd_next_resp = NULL;

#ifndef _WIN32
		gettimeofday(&start, NULL);
#else
		QueryPerformanceCounter((LARGE_INTEGER*)&start);
#endif

		error = sd_rpc_next(sd_connect_hdl, sd_session_hdl, &sd_next_resp);
		DBG_PRINTF("%s: returned from sd_next\n", argv[0]);

#ifndef _WIN32
		gettimeofday(&end, NULL);
		diff_time = (double)(end.tv_usec - start.tv_usec) / 1000000 + (double)(end.tv_sec - start.tv_sec);
#else
		QueryPerformanceCounter((LARGE_INTEGER*)&end);
		diff_time = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
#endif

		(void)print_sd_output(sd_next_resp);

		if (error != SD_SUCCESS) {
			err_origin = ERR_ORIGIN_SDAPI;
			status = SD_ERROR;
			sd_rpc_free_resp(&sd_next_resp);
			break;
		}

		if (sd_next_resp->status != NFS4_OK) {
			sd_rpc_free_resp(&sd_next_resp);
			break;
		}

		sd_rpc_ok_t *sd_rpc_ok = &sd_next_resp->sd_res_out.sd_ok;

		iter_next_count++;
		total_diffs += RPC_NEXT_NUM_CHANGES(sd_next_resp);
		printf("'iter-next#%d' time => %lf\n\n", iter_next_count, diff_time);

		if (RPC_NEXT_EOF(sd_next_resp)) {
			printf("Reached end of diff\n");
			sd_rpc_free_resp(&sd_next_resp);
			break;
		}

		sd_rpc_free_resp(&sd_next_resp);

	}

#ifndef _WIN32
	total_time = (double)(end.tv_usec - init_start.tv_usec) / 1000000 + (double)(end.tv_sec - init_start.tv_sec);
#else
	total_time = (double)(end.QuadPart - init_start.QuadPart) / freq.QuadPart;
#endif

	//TODO: Fix PRIu64 for windows
	//printf("total diffs: %"PRIu64"\n", total_diffs);
	printf("total diffs: %llu\n", total_diffs);
	printf("total time: %lf\n", total_time);

	if (total_time != 0) {
		printf("diffs per sec: %lu\n", (unsigned long)(total_diffs / total_time));
	}

	// SD_STATUS API
	DBG_PRINTF("\n\nCalling sd_rpc_status API\n");
	sd_rpc_resp_t *sd_status_resp = NULL;
	error = sd_rpc_status(sd_connect_hdl, sd_session_hdl, &sd_status_resp);
	DBG_PRINTF("%s: returned from sd_status\n", argv[0]);

	(void)print_sd_output(sd_status_resp);

	if (error != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDAPI;
		status = SD_ERROR;
	}

	sd_rpc_free_resp(&sd_status_resp);

	// SD_END API
	DBG_PRINTF("\n\nCalling sd_rpc_end API - This API will also clear the sd_session_hdl\n");
	sd_rpc_resp_t *sd_end_resp = NULL;
	error = sd_rpc_end(sd_connect_hdl, &sd_session_hdl, &sd_end_resp);
	DBG_PRINTF("%s: returned from sd_end\n", argv[0]);

	(void)print_sd_output(sd_end_resp);

	if (error != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDAPI;
		status = SD_ERROR;
	}

	sd_rpc_free_resp(&sd_end_resp);

errdone:
	if (error != SD_SUCCESS) {
		DBG_PRINTF("%s: %s returned an error\n", argv[0], sd_err_origin_to_string[err_origin]);
	}
	if (status != SD_SUCCESS) {
		printf("\n");
		printf("%s: Encountered an error!\n", argv[0]);
		print_usage();
	}
	else {
		printf("%s: Request completed successfully!\n", argv[0]);
	}

	if (session_inited) {
		(void)sd_rpc_disconnect(&sd_connect_hdl);
	}

	return status;
}

