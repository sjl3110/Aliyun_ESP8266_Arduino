/*
 * user_main.c
 */

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "driver/uart.h"
#include "mqtt/mqtt.h"
#include "mqtt/debug.h"

#include "user_config.h"
#include "aliyun_mqtt.h"
#include "user_wifi.h"
#include "cJSON.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
MQTT_Client mqttClient;

// topic
#define BASE_TOPIC "/sys/" PRODUCT_KEY "/" DEVICE_NAME   //topic头
#define GET_TOPIC BASE_TOPIC "/user/get"      //云端下传消息
//#define UPDATE_TOPIC BASE_TOPIC "/user/update"
#define UPDATE_TOPIC BASE_TOPIC "/thing/event/property/post"   //设备上传消息
static os_timer_t os_timer;
#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"


/*****************************************************************************/
// 以下是esp8266例程里的mqtt示例，只改动了小部分。
void wifiConnectCb(uint8_t status)
{
	if (status == STATION_GOT_IP)
	{
		MQTT_Connect(&mqttClient);
	}
	else
	{
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttConnectedCb(uint32_t *args)
{
	MQTT_Client *client = (MQTT_Client *)args;
	INFO("MQTT: Connected\r\n");

	MQTT_Subscribe(client, GET_TOPIC, 0);

	/*
 * MQTT_Publish函数参数说明
 * @param  client: 	    MQTT_Client reference
 * @param  topic: 		string topic will publish to
 * @param  data: 		buffer data send point to
 * @param  data_length: length of data
 * @param  qos:		    qos
 * @param  retain:      retain
 */

	MQTT_Publish(client, UPDATE_TOPIC, "hello", 6, 0, 0);
}

void mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client *client = (MQTT_Client *)args;
	INFO("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
	MQTT_Client *client = (MQTT_Client *)args;
	INFO("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char *topic, uint32_t topic_len,
				const char *data, uint32_t data_len)
{
	char *topicBuf = (char *)os_zalloc(topic_len + 1), *dataBuf =
														   (char *)os_zalloc(data_len + 1);

	MQTT_Client *client = (MQTT_Client *)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
	os_free(topicBuf);
	os_free(dataBuf);
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
	enum flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map)
	{
	case FLASH_SIZE_4M_MAP_256_256:
		rf_cal_sec = 128 - 5;
		break;

	case FLASH_SIZE_8M_MAP_512_512:
		rf_cal_sec = 256 - 5;
		break;

	case FLASH_SIZE_16M_MAP_512_512:
	case FLASH_SIZE_16M_MAP_1024_1024:
		rf_cal_sec = 512 - 5;
		break;

	case FLASH_SIZE_32M_MAP_512_512:
	case FLASH_SIZE_32M_MAP_1024_1024:
		rf_cal_sec = 1024 - 5;
		break;

	default:
		rf_cal_sec = 0;
		break;
	}

	return rf_cal_sec;
}

/*****************************************************************************/
static ICACHE_FLASH_ATTR
void loop(uint32_t *args)
{
	char data[100];
	os_memset(data,'\0',100);
	int data_len =100;
	int topic_len = sizeof(GET_TOPIC);
	MQTT_Client *client = (MQTT_Client *)args;

	uint8 uart_buf[128] = { 0 };
	uint16 len = 0;
	len = rx_buff_deq(uart_buf, 128);
	// tx_buff_enq(uart_buf, len);
	if (len > 0) {
	uint8 *pStr = "GET DATA: ";
		uart1_sendStr_no_wait(pStr);
		uart1_sendStr_no_wait(uart_buf);
	}
	char temp[5];
	char pressure[7];
	char altitude[5];
	char humidity[5];
	os_memset(temp,'\0',5);
	os_memset(pressure,'\0',7);
	os_memset(altitude,'\0',5);
	int i;
	for (i=0;i<5;i++)
	{
		temp[i] = uart_buf[i];
	}
	uart1_sendStr_no_wait(temp);

	for (i=5;i<12;i++)
	{
		pressure[i-5] = uart_buf[i];
	}
	uart1_sendStr_no_wait(pressure);
	for (i=12;i<17;i++)
	{
		humidity[i-12] = uart_buf[i];
	}
	uart1_sendStr_no_wait(humidity);



		/* declare a few. */
		cJSON *root = NULL;
		cJSON *fmt = NULL;

		root = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "id",cJSON_CreateString("123"));
		cJSON_AddItemToObject(root, "version",cJSON_CreateString("1.0"));
		cJSON_AddItemToObject(root, "method",cJSON_CreateString("thing.event.property.post"));
		cJSON_AddItemToObject(root, "params", fmt = cJSON_CreateObject());
		cJSON_AddNumberToObject(fmt, "humidity", atoi(humidity));
		cJSON_AddNumberToObject(fmt, "temp", atoi(temp));
		cJSON_AddNumberToObject(fmt, "pressure", atoi(pressure));
		cJSON_AddNumberToObject(fmt, "altitude", atoi(altitude));
		{
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
			/* create buffer to fail */
			len_fail = os_strlen(out);
			buf_fail = (char*) os_malloc(len_fail);
			/* Print to buffer */
			cJSON_PrintPreallocated(root, buf, (int) len, 1);
			/* success */
			os_printf("%s\n", buf);
			MQTT_Publish(client, UPDATE_TOPIC,buf,os_strlen(buf), 1, 0);

			os_free(out);
			os_free(buf_fail);
			os_free(buf);
		}
		cJSON_Delete(root);


//	mqttDataCb(args,GET_TOPIC,topic_len,data,data_len);
	os_timer_disarm(&os_timer);
	os_timer_arm(&os_timer, 2000, true );
}

void user_init(void)
{
	//uart_init(BIT_RATE_74880, BIT_RATE_74880);
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	// 测试 hmacmd5 生成mqtt passwrod
	test_hmac_md5();

	aliyun_mqtt_init();

	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);
	MQTT_InitConnection(&mqttClient, g_aliyun_mqtt.host, g_aliyun_mqtt.port, 0);

	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);
	MQTT_InitClient(&mqttClient, g_aliyun_mqtt.client_id, g_aliyun_mqtt.username,
					g_aliyun_mqtt.password, g_aliyun_mqtt.keepalive, 1);

	// 遗愿消息
	// 阿里云mqtt不需要设置遗愿消息
	//MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);

	// 设置mqtt的回调函数
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	// 连接wifi
	wifi_connect(wifiConnectCb);

	uart1_sendStr_no_wait("\r\nSystem started ...\r\n");
	uart1_sendStr_no_wait("\r\nStarting callback loop function...\r\n");

	os_timer_disarm(&os_timer);
	os_timer_setfn(&os_timer, (ETSTimerFunc *) ( loop ), &mqttClient );
	os_timer_arm(&os_timer, 1000, true );
}
