/*
 Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "cJSON.h"

#include "c_types.h"
#include "osapi.h"
#include "mem.h"
#include "ets_sys.h"
#include "osapi.h"

/* Used by some code below as an example datatype. */
struct record {
	const char *precision;
	double lat;
	double lon;
	const char *address;
	const char *city;
	const char *state;
	const char *zip;
	const char *country;
};

/* Create a bunch of objects as demonstration. */
//static int ICACHE_FLASH_ATTR
int ICACHE_FLASH_ATTR
print_preallocated(cJSON *root) {
	/* declarations */
	char *out = NULL;
	char *buf = NULL;
	char *buf_fail = NULL;
	size_t len = 0;
	size_t len_fail = 0;

	/* formatted print */
	out = cJSON_Print(root);

	/* create buffer to succeed */
	/* the extra 5 bytes are because of inaccuracies when reserving memory */
	len = os_strlen(out) + 5;
	buf = (char*) os_malloc(len);
	if (buf == NULL) {
		os_printf("Failed to allocate memory.\n");
		//exit(1);
	}

	/* create buffer to fail */
	len_fail = os_strlen(out);
	buf_fail = (char*) os_malloc(len_fail);
	if (buf_fail == NULL) {
		os_printf("Failed to allocate memory.\n");
		//exit(1);
	}

	/* Print to buffer */
	if (!cJSON_PrintPreallocated(root, buf, (int) len, 1)) {
		os_printf("cJSON_PrintPreallocated failed!\n");
		if (os_strcmp(out, buf) != 0) {
			os_printf("cJSON_PrintPreallocated not the same as cJSON_Print!\n");
			os_printf("cJSON_Print result:\n%s\n", out);
			os_printf("cJSON_PrintPreallocated result:\n%s\n", buf);
		}
		os_free(out);
		os_free(buf_fail);
		os_free(buf);
		return -1;
	}

	/* success */
	os_printf("%s\n", buf);

	/* force it to fail */
	if (cJSON_PrintPreallocated(root, buf_fail, (int) len_fail, 1)) {
		os_printf(
				"cJSON_PrintPreallocated failed to show error with insufficient memory!\n");
		os_printf("cJSON_Print result:\n%s\n", out);
		os_printf("cJSON_PrintPreallocated result:\n%s\n", buf_fail);
		os_free(out);
		os_free(buf_fail);
		os_free(buf);
		return -1;
	}

	os_free(out);
	os_free(buf_fail);
	os_free(buf);
	return 0;
}

/* Create a bunch of objects as demonstration. */
static ICACHE_FLASH_ATTR
void create_objects(void) {
	/* declare a few. */
	cJSON *root = NULL;
	cJSON *fmt = NULL;
	cJSON *img = NULL;
	cJSON *thm = NULL;
	cJSON *fld = NULL;
	int i = 0;


	/* Here we construct some JSON standards, from the JSON site. */

	/* Our "Video" datatype: */
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "device_name",
			cJSON_CreateString("wifi_module"));
	cJSON_AddItemToObject(root, "format", fmt = cJSON_CreateObject());
	cJSON_AddNumberToObject(fmt, "temp", 4);
	cJSON_AddNumberToObject(fmt, "b", 3);
	cJSON_AddNumberToObject(fmt, "c", 1);
	cJSON_AddNumberToObject(fmt, "d", 1);


	/* Print to text */
	if (print_preallocated(root) != 0) {
		cJSON_Delete(root);
		//exit(EXIT_FAILURE);
	}
	cJSON_Delete(root);
}

int ICACHE_FLASH_ATTR
cJSON_test(void) {
	//os_printf("\r\n=====================\r\n");
	///* print the version */
	//os_printf("cJSON Version: %s", cJSON_Version());
	//os_printf("\r\n=====================\r\n");

	/* Now some samplecode for building objects concisely: */
	create_objects();

//	os_printf("\r\nparse string!\r\n");
//	u8* data = "{\"string\":\"hello world\"}";
//	cJSON * root = cJSON_Parse(data);
//	if (NULL != root) {
//		if (cJSON_HasObjectItem(root, "string")) {
//			cJSON *string = cJSON_GetObjectItem(root, "string");
//			if (cJSON_IsString(string)) {
//				char *s = cJSON_Print(string);
//				os_printf("string: %s\r\n", s);
//				cJSON_free((void *) s);
//			}
//		}
//		cJSON_Delete(root);
//	} else {
//		os_printf("\r\nparse error!\r\n");
//	}

//	os_printf("\r\nparse number!\r\n");
//	u8* num = "{\"number\":12345}";
//	cJSON * root2 = cJSON_Parse(num);
//	if (NULL != root) {
//		if (cJSON_HasObjectItem(root, "number")) {
//			cJSON *number = cJSON_GetObjectItem(root, "number");
//			if (cJSON_IsNumber(number)) {
//				char *n = cJSON_Print(number);
//				os_printf("number: %s\r\n", n);
//				cJSON_free((void *) n);
//			}
//		}
//		cJSON_Delete(root);
//
//	} else {
//		os_printf("\r\nparse error!\r\n");
//	}

	os_printf("\r\n");

	return 0;
}
