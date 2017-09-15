/****************************************************************************
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/

/// @file tc_audio_main.c
/// @brief Main Function for Filesystem TestCase Example

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <tinyara/audio/audio.h>
#include <tinyalsa/tinyalsa.h>
#include <tinyara/fs/ioctl.h>
#include <apps/shell/tash.h>
#include "tc_common.h"

/****************************************************************************
 * Definitions
 ****************************************************************************/
#define AUDIO_TEST_FILE "/mnt/pcm"

#define AUDIO_DEFAULT_CHANNELS 2

#define AUDIO_DEFAULT_RATE 48000

#define AUDIO_DEFAULT_FORMAT PCM_FORMAT_S16_LE

#define AUDIO_RECORD_DURATION 3 //3sec
/****************************************************************************
 * Global Variables
 ****************************************************************************/

extern sem_t tc_sem;
extern int working_tc;

struct pcm *g_pcm;

unsigned int g_byte_size;

/**
* @testcase         audio_pcm_open_p
* @brief            open and initlaize pcm structure
* @scenario         Open pcm and check its validation
* @apicovered       pcm_open
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_open_tc_p(void)
{
	g_pcm = pcm_open(0, 0, PCM_IN, NULL);
	printf("g_pcm = %u\n", g_pcm);
	printf("fd = %d\n", pcm_get_file_descriptor(g_pcm));
	TC_ASSERT_GT("pcm_open", pcm_get_file_descriptor(g_pcm), 0)
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_open_n
* @brief            open and initlaize pcm structure with invalid values
* @scenario         Open pcm with invalid device & card value and check its validation
* @apicovered       pcm_open
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_open_tc_n(void)
{
	struct pcm *pcm;
	pcm = pcm_open(999, 999, PCM_IN, NULL);
	TC_ASSERT_LEQ("pcm_open", pcm_get_file_descriptor(pcm), 0)
	pcm_close(pcm);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_close_p
* @brief            close opened pcm
* @scenario         close opened pcm
* @apicovered       pcm_close
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_close_tc_p(void)
{
	int ret;
	ret = pcm_close(g_pcm);
	TC_ASSERT_EQ("pcm_close", ret, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_close_n
* @brief            close opened pcm
* @scenario         close opened pcm with null pcm
* @apicovered       pcm_close
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_close_tc_n(void)
{
	int ret;
	ret = pcm_close(NULL);
	printf("ret = %d\n", ret);
	TC_ASSERT_NEQ("pcm_close", ret, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_open_by_name_p
* @brief            open with name and initlaize pcm structure
* @scenario         Open pcm and check its validation
* @apicovered       pcm_open_by_name
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_open_by_name_tc_p(void)
{
	g_pcm = pcm_open_by_name("hw:0,0", PCM_IN, NULL);
	TC_ASSERT_GT("pcm_open_by_name", pcm_get_file_descriptor(g_pcm), 0)
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_open_by_name_n
* @brief            open with name and initlaize pcm structure
* @scenario         Open pcm and with invalid name, check its validation
* @apicovered       pcm_open_by_name
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_open_by_name_tc_n(void)
{
	struct pcm *pcm;
	pcm = pcm_open_by_name(NULL, PCM_IN, NULL);
	TC_ASSERT_LEQ("pcm_open", pcm_get_file_descriptor(pcm), 0)
	pcm_close(pcm);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_config_p
* @brief            get configuration values of pcm
* @scenario         get configuration values with pcm structure
* @apicovered       pcm_get_config
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_config_tc_p(void)
{
	const struct pcm_config *config = pcm_get_config(g_pcm);
	TC_ASSERT_EQ("pcm_get_config", config->channels, AUDIO_DEFAULT_CHANNELS);
	TC_ASSERT_EQ("pcm_get_config", config->format, AUDIO_DEFAULT_FORMAT);
	TC_ASSERT_EQ("pcm_get_config", config->rate, AUDIO_DEFAULT_RATE);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_config_n
* @brief            get configuration values of pcm
* @scenario         get configuration values of NULL pcm data
* @apicovered       pcm_get_config
* @precondition     NA.
* @postcondition    NA
*/
static void utc_audio_pcm_get_config_tc_n(void)
{
	const struct pcm_config *config = pcm_get_config(NULL);
	TC_ASSERT_EQ("pcm_get_config", config, NULL);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_channels_p
* @brief            get channels values of pcm
* @scenario         get channels values after pcm_open
* @apicovered       pcm_get_channels
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_channels_tc_p(void)
{
	unsigned int ch;
	ch = pcm_get_channels(g_pcm);
	TC_ASSERT_EQ("pcm_get_channels", ch, AUDIO_DEFAULT_CHANNELS);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_channels_n
* @brief            get channels values of pcm
* @scenario         get channels values with NULL pcm
* @apicovered       pcm_get_channels
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_channels_tc_n(void)
{
	int ch;
	ch = pcm_get_channels(NULL);
	TC_ASSERT_EQ("pcm_get_channels", ch, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_rate_p
* @brief            get bit-rate values of pcm
* @scenario         get bit-rate values after pcm_open
* @apicovered       pcm_get_rate
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_rate_tc_p(void)
{
	int rate;
	rate = pcm_get_rate(g_pcm);
	TC_ASSERT_EQ("pcm_get_rate", rate, AUDIO_DEFAULT_RATE);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_rate_n
* @brief            get rate values of pcm
* @scenario         get rate values with NULL pcm
* @apicovered       pcm_get_rate
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_rate_tc_n(void)
{
	int rate;
	rate = pcm_get_rate(NULL);
	TC_ASSERT_EQ("pcm_get_channels", rate, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_format_p
* @brief            get format values of pcm
* @scenario         get format values after pcm_open
* @apicovered       pcm_get_format
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_format_tc_p(void)
{
	int format;
	format = pcm_get_format(g_pcm);
	TC_ASSERT_EQ("pcm_get_format", format, AUDIO_DEFAULT_FORMAT);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_format_n
* @brief            get format values of pcm
* @scenario         get format values with NULL pcm
* @apicovered       pcm_get_format
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_format_tc_n(void)
{
	int pcm_format;
	pcm_format = pcm_get_format(NULL);
	TC_ASSERT_EQ("pcm_get_format", pcm_format, -1);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_file_descriptor_p
* @brief            get file descriptor values of pcm
* @scenario         get file descriptor after pcm_open
* @apicovered       pcm_get_file_descriptor
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_file_descriptor_tc_p(void)
{
	int fd;
	fd = pcm_get_file_descriptor(g_pcm);
	TC_ASSERT_GT("pcm_get_file_descriptor", fd, 0)
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_file_descriptor_n
* @brief            get file descriptor values of pcm
* @scenario         get file descriptor with NULL pcm
* @apicovered       pcm_get_file_descriptor
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_file_descriptor_tc_n(void)
{
	int fd;
	fd = pcm_get_file_descriptor(NULL);
	TC_ASSERT_LEQ("pcm_get_file_descriptor", fd, -1)
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_error_p
* @brief            get last error of pcm
* @scenario         file open with invalid file path and call pcm_get_error
* @apicovered       pcm_get_error
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_error_tc_p(void)
{
	unsigned int card = 0;
	unsigned int device = 0;
	int flags = PCM_IN;

	struct pcm *pcm;
	pcm = pcm_open(card, device, flags, NULL); //invalid card device values
	const char *errors = pcm_get_error(pcm);
	TC_ASSERT("pcm_get_error", errors);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_error_n
* @brief            get last error of pcm
* @scenario         call pcm_get_error with NULL pcm
* @apicovered       pcm_get_error
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_error_tc_n(void)
{
	TC_ASSERT_EQ("pcm_get_error", pcm_get_error(NULL), NULL);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_buffer_size_p
* @brief            get buffer size of pcm
* @scenario         get buffer size after pcm_open
* @apicovered       pcm_get_buffer_size
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_buffer_size_tc_p(void)
{
	ssize_t size;
	size = pcm_get_buffer_size(g_pcm);
	TC_ASSERT_GT("pcm_get_buffer_size", size, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_buffer_size_n
* @brief            get buffer size of pcm
* @scenario         get buffer size with NULL pcm
* @apicovered       pcm_get_buffer_size
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_get_buffer_size_tc_n(void)
{
	ssize_t size;
	size = pcm_get_buffer_size(NULL);
	TC_ASSERT_EQ("pcm_get_buffer_size", size, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_subdevice_p
* @brief            get subdevice of pcm
* @scenario         get subdevice of pcm
* @apicovered       pcm_get_subdevice
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_get_subdevice_tc_p(void)
{
	unsigned int device;
	device = pcm_get_subdevice(g_pcm);
	TC_ASSERT_NEQ("pcm_get_subdevice", device, 1);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_get_subdevice_n
* @brief            get subdevice of pcm
* @scenario         get subdevice with NULL pcm
* @apicovered       pcm_get_subdevice
* @precondition     NA.
* @postcondition    NA
*/
static void utc_audio_pcm_get_subdevice_tc_n(void)
{
	unsigned int device;
	device = pcm_get_subdevice(g_pcm);
	TC_ASSERT_EQ("pcm_get_subdevice", device, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_set_config_p
* @brief            set config values of pcm
* @scenario         set config values with pcm_config
* @apicovered       pcm_set_config
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_set_config_tc_p(void)
{
	struct pcm_config config;
	config.channels = 1;
	config.rate = 4000;
	config.format = PCM_FORMAT_S8;
	pcm_set_config(g_pcm, &config);
	TC_ASSERT_EQ("pcm_set_config", pcm_get_channels(g_pcm), 1);
	TC_ASSERT_EQ("pcm_set_config", pcm_get_rate(g_pcm), 4000);
	TC_ASSERT_EQ("pcm_set_config", pcm_get_format(g_pcm), PCM_FORMAT_S8);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_set_config_n
* @brief            set config values of pcm
* @scenario         set config values with NULL pcm
* @apicovered       pcm_set_config
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_set_config_tc_n(void)
{
	TC_ASSERT_LT("pcm_set_config", pcm_set_config(NULL, NULL), 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_frame_to_bytes_p
* @brief            convert frames to bytes
* @scenario         get configuration value of pcm and calculate frame to byte
* @apicovered       pcm_frames_to_bytes
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_frames_to_bytes_p(void)
{
	ssize_t size;
	/* set basic configuration values for next test*/
	pcm_set_config(g_pcm, NULL);

	size = pcm_get_buffer_size(g_pcm);
	g_byte_size = pcm_frames_to_bytes(g_pcm, size);
	TC_ASSERT_GT("pcm_frames_to_bytes", g_byte_size, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_frame_to_bytes_n
* @brief            convert frames to bytes
* @scenario         get configuration value of NULL pcm and calculate frame to byte
* @apicovered       pcm_frames_to_bytes
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_frames_to_bytes_n(void)
{
	unsigned int bytes;
	bytes = pcm_frames_to_bytes(NULL, pcm_get_buffer_size(g_pcm));
	TC_ASSERT_EQ("pcm_frames_to_bytes", bytes, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_bytes_to_frames_p
* @brief            convert bytes to frame
* @scenario         get configuration value of pcm and calculate byte to frame
* @apicovered       pcm_bytes_to_frames
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_bytes_to_frames_p(void)
{

	unsigned int frame_size;
	frame_size = pcm_bytes_to_frames(g_pcm, g_byte_size);
	TC_ASSERT_GT("pcm_bytes_to_frame", frame_size, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_bytes_to_frames_n
* @brief            convert bytes to frame
* @scenario         get configuration value of NULL pcm and calculate byte to frame
* @apicovered       pcm_bytes_to_frames
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_bytes_to_frames_n(void)
{
	unsigned int frame;
	frame = pcm_frames_to_bytes(NULL, 0);
	TC_ASSERT_LEQ("pcm_frames_to_bytes", frame, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         utc_audio_pcm_format_to_bits_p
* @brief            get bits information based on pcm's format
* @scenario         get bits of format value of pcm
* @apicovered       pcm_format_to_bits
* @precondition     pcm should be opened before.
* @postcondition    NA
*/
static void utc_audio_pcm_format_to_bits_p(void)
{
	enum pcm_format format;
	format = pcm_get_format(g_pcm);
	TC_ASSERT_NEQ("pcm_format_to_bits", format, PCM_FORMAT_NONE);
	TC_ASSERT_NEQ("pcm_format_to_bits", pcm_format_to_bits(format), 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         utc_audio_pcm_format_to_bits_n
* @brief            get bits information based on pcm's format
* @scenario        get bits of format value with invalid format
* @apicovered       pcm_format_to_bits
* @precondition     NA
* @postcondition    NA
*/
static void utc_audio_pcm_format_to_bits_n(void)
{
	TC_ASSERT_EQ("pcm_format_to_bits", pcm_format_to_bits(PCM_FORMAT_NONE), 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_readi_p
* @brief            read captured data from chip
* @scenario         record voice and write data in specific file
* @apicovered       pcm_readi
* @precondition     pcm should be opened before
* @postcondition    NA
*/
static void utc_audio_pcm_readi_p(void)
{
	FILE *fp;
	char *buffer;
	char input_str[16];
	ssize_t size;
	uint32_t frames_read = 0;
	unsigned int total_frames_read = 0;
	unsigned int frame_count;
	int capturing;
	int ret;

	fp = fopen(AUDIO_TEST_FILE, "w+");
	TC_ASSERT("pcm_readi", fp);
	TC_ASSERT("pcm_readi", pcm_is_ready(g_pcm));

	size = pcm_get_buffer_size(g_pcm);
	frame_count = pcm_bytes_to_frames(g_pcm, g_byte_size);
	buffer = malloc(frame_count);
	TC_ASSERT_CLEANUP("pcm_readi", buffer, fclose(fp));
	printf("Record will be start for 3s, press any key to start\n");
	fflush(stdout);

	gets(input_str);
	capturing = 1;
	while (capturing) {
		frames_read = pcm_readi(g_pcm, buffer, frame_count);
		total_frames_read += frames_read;

		if (frames_read > 0) {
			ret = fwrite(buffer, frames_read, 1, fp);
			if (ret != frames_read) {
				TC_ASSERT_EQ("pcm_readi", ret, frames_read);
				free(buffer);
				fclose(fp);
			}
		} else {
			capturing = 0;
		}
	}
	free(buffer);
	fclose(fp);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_readi_n
* @brief            read captured data from chip
* @scenario         call readi with invalid values
* @apicovered       pcm_readi
* @precondition     pcm should be opened before
* @postcondition    NA
*/
static void utc_audio_pcm_readi_n(void)
{
	int ret;
	ssize_t size;
	ssize_t bytes;
	char *buffer;

	size = pcm_get_buffer_size(g_pcm);
	bytes = pcm_frames_to_bytes(g_pcm, size);
	buffer = (char *)malloc(sizeof(bytes));
	TC_ASSERT("pcm_readi", buffer);

	ret = pcm_readi(NULL, buffer, size);
	TC_ASSERT_LT("pcm_readi", ret, 0);

	ret = pcm_readi(g_pcm, NULL, size);
	TC_ASSERT_LT("pcm_readi", ret, 0);

	ret = pcm_readi(g_pcm, buffer, 0);
	TC_ASSERT_LT("pcm_readi", ret, 0);

	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_write_p
* @brief            play captured data from filesystem
* @scenario         play recored file previously
* @apicovered       pcm_writei
* @precondition     pcm_readi should be opened before
* @postcondition    NA
*/
static void utc_audio_pcm_writei_p(void)
{
	FILE *fp;
	char *buffer;
	ssize_t size;
	ssize_t bytes;
	int ret;

	fp = fopen(AUDIO_TEST_FILE, "r");
	TC_ASSERT("pcm_writei", fp);

	/* use default config here */
	g_pcm = pcm_open(0, 0, PCM_OUT, NULL);
	TC_ASSERT("pcm_writei", pcm_is_ready(g_pcm));

	size = pcm_get_buffer_size(g_pcm);
	bytes = pcm_frames_to_bytes(g_pcm, size);
	buffer = (char *)malloc(sizeof(bytes));
	TC_ASSERT_CLEANUP("pcm_writei", buffer, fclose(fp));

	printf("playback start!!\n");

	do {
		ret = fread(buffer, 1, size, fp);
		if (ret > 0) {
			ret = pcm_writei(g_pcm, buffer, pcm_bytes_to_frames(g_pcm, ret));

			if (ret < 0) {
				printf("error!! : %d\n", ret);
				break;
			}
		}
	} while (ret > 0);
	free(buffer);
	fclose(fp);
	pcm_close(g_pcm);
	TC_ASSERT_GEQ("pcm_writei", ret, 0);
	TC_SUCCESS_RESULT();
}

/**
* @testcase         audio_pcm_write_n
* @brief            play captured data from filesystem
* @scenario         play recored file previously
* @apicovered       pcm_writei
* @precondition     pcm_readi should be opened before
* @postcondition    NA
*/
static void utc_audio_pcm_writei_n(void)
{
	FILE *fp;
	char *buffer;
	ssize_t size;
	ssize_t bytes;
	int ret;

	fp = fopen(AUDIO_TEST_FILE, "r");
	TC_ASSERT("audio_pcm_writei", fp);

	/* use default config here */
	g_pcm = pcm_open(0, 0, PCM_OUT, NULL);
	TC_ASSERT("pcm_writei", pcm_is_ready(g_pcm));

	size = pcm_get_buffer_size(g_pcm);
	bytes = pcm_frames_to_bytes(g_pcm, size);
	buffer = malloc(bytes);
	TC_ASSERT_CLEANUP("pcm_writei", buffer, fclose(fp));

	ret = pcm_writei(NULL, buffer, size);
	TC_ASSERT_LT("pcm_writei", ret, 0);

	ret = pcm_writei(g_pcm, NULL, size);
	TC_ASSERT_LT("pcm_writei", ret, 0);

	ret = pcm_writei(g_pcm, buffer, 0);
	TC_ASSERT_LT("pcm_writei", ret, 0);

	TC_SUCCESS_RESULT();
}

static int audio_tc_launcher(int argc, char **args)
{
	total_pass = 0;
	total_fail = 0;
	utc_audio_pcm_open_tc_p();
	utc_audio_pcm_open_tc_n();
	utc_audio_pcm_close_tc_p();
	utc_audio_pcm_close_tc_n();
	utc_audio_pcm_open_by_name_tc_p();
	utc_audio_pcm_open_by_name_tc_n();
	utc_audio_pcm_get_config_tc_p();
	utc_audio_pcm_get_config_tc_n();
	utc_audio_pcm_get_channels_tc_p();
	utc_audio_pcm_get_channels_tc_n();
	utc_audio_pcm_get_rate_tc_p();
	utc_audio_pcm_get_rate_tc_n();
	utc_audio_pcm_get_format_tc_p();
	utc_audio_pcm_get_format_tc_n();
	utc_audio_pcm_get_file_descriptor_tc_p();
	utc_audio_pcm_get_file_descriptor_tc_n();
	utc_audio_pcm_get_error_tc_p();
	utc_audio_pcm_get_error_tc_n();
	utc_audio_pcm_get_buffer_size_tc_p();
	utc_audio_pcm_get_buffer_size_tc_n();
	utc_audio_pcm_get_subdevice_tc_p();
	utc_audio_pcm_get_subdevice_tc_n();
	utc_audio_pcm_set_config_tc_p();
	utc_audio_pcm_set_config_tc_n();
	utc_audio_pcm_frames_to_bytes_p();
	utc_audio_pcm_frames_to_bytes_n();
	utc_audio_pcm_bytes_to_frames_p();
	utc_audio_pcm_bytes_to_frames_n();
	utc_audio_pcm_format_to_bits_p();
	utc_audio_pcm_format_to_bits_n();
	utc_audio_pcm_readi_p();
	utc_audio_pcm_readi_n();
	utc_audio_pcm_writei_p();
	utc_audio_pcm_writei_n();

	unlink(AUDIO_TEST_FILE);

	printf("#########################################\n");
	printf("           FS TC Result               \n");
	printf("           PASS : %d FAIL : %d        \n", total_pass, total_fail);
	printf("#########################################\n");
	return total_pass;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int utc_audio_main(int argc, char *argv[])
#endif
{
	sem_wait(&tc_sem);
	working_tc++;

#ifdef CONFIG_TASH
	tash_cmd_install("audio_utc", audio_tc_launcher, TASH_EXECMD_SYNC);
#else
	audio_tc_launcher(argc, argv);
#endif

	working_tc--;
	sem_post(&tc_sem);

	return 0;
}
