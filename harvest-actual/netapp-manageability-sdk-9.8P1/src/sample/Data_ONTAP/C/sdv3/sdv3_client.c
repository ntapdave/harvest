#ifndef _WIN32
#include <sys/time.h> /* for gettimeofday */
#include <getopt.h>
#include <unistd.h>
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

#include "sdv3_api.h"

#define DBG_PRINTF(...)         ((void)0)
//#define DBG_PRINTF(...)         (printf(__VA_ARGS__))

typedef enum sd_err_origin {
	ERR_ORIGIN_UNKNOWN=0,
	ERR_ORIGIN_SDAPI,
	ERR_ORIGIN_SDCLNT
} sd_err_origin;
const char* sd_err_origin_to_string[] = {"UNKNOWN", "SDAPI", "SDCLIENT"};

static struct option sd_long_options[] =
{
	{"ip", required_argument, 0, 'f'},
	{"session_handle", required_argument, 0, 'h'},
	{"session_id", required_argument, 0, 'i'},
	{0, 0, 0, 0}
};

static void sd_print_args(sd_connect_args_t *sd_connect_args);
static void sd_init_connect_args(sd_connect_args_t *sd_connect_args);
static int32_t sd_validate_connect_args(sd_connect_args_t *sd_connect_args);
static int sd_parse_input(int argc, char **argv, sd_connect_args_t *sd_connect_args);
static void print_usage();
int main(int argc, char **argv);

/*
 * Print sd_connect_args
 */
static void sd_print_args(sd_connect_args_t *sd_connect_args)
{
	DBG_PRINTF("IP = %s\n", sd_connect_args->ip_addr);
	DBG_PRINTF("Session_handle = %s\n", sd_connect_args->session_handle);
	DBG_PRINTF("Session_id = %s\n", sd_connect_args->session_id);
}

/*
 * This will initialize all the sd_connect_args
 * NOTE: No allocations are done here!
 */
static void sd_init_connect_args(sd_connect_args_t *sd_connect_args)
{
	sd_connect_args->ip_addr = NULL;
	sd_connect_args->session_handle = NULL;
	sd_connect_args->session_id = NULL;
}

/*
 * This function will perform some
 * basic validation on sd_connect_args
 *
 * WARNING: This function does not protect against junk
 * values, so be sure to initialize all the arguments
 * you send!
 */
int32_t
static sd_validate_connect_args(sd_connect_args_t *sd_connect_args)
{
	if (sd_connect_args == NULL) {
		return SD_ERROR;
	}

	if (!sd_connect_args->ip_addr ||
		!sd_connect_args->session_handle || !sd_connect_args->session_id) {
		DBG_PRINTF("sd_validate_connect_args: one or more required arguments missing\n");
		return SD_ERROR;
	}

	return SD_SUCCESS;
}

/*
 * This function initializes sd_connect_args to default values.
 * It then goes through the arg list and fills sd_connect_args
 * accordingly. Finally, it does some basic validation
 * via sd_validate_args.
 */
static int sd_parse_input(int argc, char **argv, sd_connect_args_t *sd_connect_args)
{
	int opt;
/*	opterr = 0; //suppress getopt's output */

	//set defaults
	sd_init_connect_args(sd_connect_args);

	while ((opt = getopt_long(argc, argv, "f:h:i:", sd_long_options, NULL)) != -1)
	{
		switch (opt) {
			case 'f':
				sd_connect_args->ip_addr = optarg;
				break;

			case 'h':
				sd_connect_args->session_handle = optarg;
				break;

			case 'i':
				sd_connect_args->session_id = optarg;
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
			printf ("%s ", argv[optind++]);
		putchar ('\n');
		return SD_ERROR;
	}

	return sd_validate_connect_args(sd_connect_args);
}

/*
 * Print the correct usage of sd-client
 */
static void print_usage()
{
	fprintf(stderr, "USAGE: sdv3-client -f <VSERVERIP> -h <SESSION_HANDLE> -i <SESSION_ID>\n");
	fprintf(stderr, "VSERVERIP:		     Vserver Data IP address.\n");
        fprintf(stderr, "SESSION_HANDLE, SESSION_ID: Please refer in snapdiff\n");
	fprintf(stderr, "                            V3 API documentation on how to invoke REST API \n");
	fprintf(stderr, "                            \"POST /storage/volumes/{volume.uuid}/snapdiff/sessions\"\n");
        fprintf(stderr, "                            and get SESSION_HANDLE and SESSION_ID.\n");
}

/*
 * The hierarchy of this main:
 *	parse input args (sd_parse_input)
 *	validate input args (sd_validate_connect_args)
 *	init session (sd_rpc_connect_v3)
 *	call sd internal apis
 *              i) API to fetch diffs (sd_rpc_next_v3)
 *              ii) API to check the status of the diff operation (sd_rpc_status_v3)
 *	print output, if applicable (print_sd_output)
 *	free output allocated by sd internal apis (sd_rpc_free_resp)
 *	close session (sd_rpc_disconnect_v3)
 */
int main(int argc, char **argv)
{
	uint32_t error = 0;
	int status = SD_SUCCESS;
	char *connect_ip = NULL;
	sd_connect_handle_t sd_connect_hdl = NULL;
	sd_connect_args_t sd_connect_args;
	sd_err_origin err_origin = ERR_ORIGIN_UNKNOWN;
	int iter_next_count = 0;

#ifndef _WIN32
	struct timeval init_start, start, end;
#else
	LARGE_INTEGER init_start, start, end, freq;
#endif
	double total_time = 0, diff_time = 0;
	uint64_t total_diffs = 0;

	printf("%s: sd client v5.0 started\n", argv[0]);
	DBG_PRINTF("%s: parsing input...\n", argv[0]);
	status = sd_parse_input(argc, argv, &sd_connect_args);
	if (status != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDCLNT;
		goto errdone;
	}

	/* Print input */
	sd_print_args(&sd_connect_args);
	status = sd_validate_connect_args(&sd_connect_args);
	if (status != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDCLNT;
		goto errdone;
	}

	connect_ip = sd_connect_args.ip_addr;
	DBG_PRINTF("connect_ip: %s\n", connect_ip);

	DBG_PRINTF("%s: Starting API calls\n", argv[0]);

	DBG_PRINTF("%s: Calling CONNECT API\n", argv[0]);
	// SD_CONNECT_V3 API
        error = sd_rpc_connect_v3(&sd_connect_args, &sd_connect_hdl);
	DBG_PRINTF("%s: returned from sd_rpc_connect_v3\n", argv[0]);
	if (error != 0) {
		err_origin = ERR_ORIGIN_SDAPI;
		status = SD_ERROR;
		goto errdone;
	}

	// SD_NEXT_V3 API
	DBG_PRINTF("\n\nCalling sd_rpc_next_v3 API\n");
	sd_rpc_resp_t *sd_next_resp = NULL;

#ifndef _WIN32
	    gettimeofday(&init_start, NULL);
#else
	    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	    QueryPerformanceCounter((LARGE_INTEGER*)&init_start);
#endif

	while(1) {
		sd_next_resp = NULL;

#ifndef _WIN32
		gettimeofday(&start, NULL);
#else
		QueryPerformanceCounter((LARGE_INTEGER*)&start);
#endif
		error = sd_rpc_next_v3(sd_connect_hdl, &sd_next_resp);

#ifndef _WIN32
		gettimeofday(&end, NULL);
		diff_time = (double)(end.tv_usec - start.tv_usec)/1000000 + (double)(end.tv_sec - start.tv_sec);
#else
		QueryPerformanceCounter((LARGE_INTEGER*)&end);
		diff_time = (double)(end.QuadPart - start.QuadPart)/freq.QuadPart;
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
		total_diffs +=RPC_NEXT_NUM_CHANGES(sd_next_resp);
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
	printf("total diffs: %"PRIu64"\n", total_diffs);
#else
        total_time = (double)(end.QuadPart - init_start.QuadPart) / freq.QuadPart;
	printf("total diffs: %llu\n", total_diffs);
#endif

	printf("total time: %lf\n", total_time);

	if (total_time != 0) {
		printf("diffs per sec: %lu\n", (unsigned long)(total_diffs/total_time));
	}

	// SD_STATUS_V3 API
	DBG_PRINTF("\n\nCalling sd_rpc_status_v3 API\n");
	sd_rpc_resp_t *sd_status_resp = NULL;
	error = sd_rpc_status_v3(sd_connect_hdl, &sd_status_resp);
	DBG_PRINTF("%s: returned from sd_status\n", argv[0]);

	(void)print_sd_output(sd_status_resp);

	if (error != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDAPI;
		status = SD_ERROR;
	}

	sd_rpc_free_resp(&sd_status_resp);

	// SD_DISCONNECT_V3 API
        DBG_PRINTF("\n\nCalling sd_rpc_disconnect_v3 API - This API will also disconnect.\n");
	/* In case of any failures in sd_disconnect_v3, we are going to retry internally
	 * for DISCONNECT_MAX_RETRIES times. In the case when the error is unrecoverable, then
	 * the memory of the connect handle is freed.
         */
	sd_rpc_resp_t *sd_disconnect_resp = NULL;
	error = sd_rpc_disconnect_v3(sd_connect_hdl, &sd_disconnect_resp);
	DBG_PRINTF("%s: returned from sd_rpc_disconnect_v3\n", argv[0]);

	(void)print_sd_output(sd_disconnect_resp);

	if (error != SD_SUCCESS) {
		err_origin = ERR_ORIGIN_SDAPI;
		status = SD_ERROR;
	}

	sd_rpc_free_resp(&sd_disconnect_resp);
errdone:
	if (error != SD_SUCCESS) {
		DBG_PRINTF("%s: %s returned an error\n", argv[0], sd_err_origin_to_string[err_origin]);
	}
	if (status != SD_SUCCESS) {
		printf("\n");
		printf("%s: Encountered an error!\n", argv[0]);
		print_usage();
	} else {
		printf("%s: Request completed successfully!\n", argv[0]);
	}

	return status;
}
