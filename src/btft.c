/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by ftapplicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <rawsensordata.h>

#include <stdio.h>
#include <glib.h>
#include <dlog.h>
#include <app_common.h>
#include <bt.h>
#include <string.h>

static char transfer_messages[][100]={
	[SAP_FT_TRANSFER_SUCCESS]="Transfer Completed",
	[SAP_FT_TRANSFER_FAIL_CHANNEL_IO]="Channel IO Error",
	[SAP_FT_TRANSFER_FAIL_FILE_IO]="File IO Error",
	[SAP_FT_TRANSFER_FAIL_CMD_DROPPED]="Transfer dropped",
	[SAP_FT_TRANSFER_FAIL_PEER_UNRESPONSIVE]="Peer Un Responsive",
	[SAP_FT_TRANSFER_FAIL_PEER_CONN_LOST]="Device Connection Lost",
	[SAP_FT_TRANSFER_FAIL_PEER_CANCELLED]="Peer Cancelled",
	[SAP_FT_TRANSFER_FAIL_SPACE_NOT_AVAILABLE]="No Space",
	[6]="Unknown Error",
	[7]="Unknown Error",
	[8]="Unknown Error",
	[10]="Unknown Error",
};

static int connected = 0;
struct priv priv_data;

int deleteFile(char* filePath);
const char* get_next_filePath(const char*);
void update_last_upload_time();

static gboolean _find_peer_agent(gpointer user_data);

int send_file(char* file_path);

static void _on_send_completed(sap_file_transaction_h file_transaction,
			       sap_ft_transfer_e result,
			       const char *file_path,
			       void *user_data)
{
	if (priv_data.file_socket) {
		sap_file_transfer_destroy(priv_data.file_socket);
	}

	priv_data.file_socket = NULL;
	priv_data.result = result;

	if (result == SAP_FT_TRANSFER_SUCCESS) {
#ifdef DEBUG_ON
		dlog_print(DLOG_INFO, LOG_TAG, "Transfer Completed of: %s", file_path);
#endif
		deleteFile(file_path);
		file_on_progress = 0;
		const char* nextfilePath;
		if(nextfilePath = get_next_filePath(app_get_data_path()))
			send_file(nextfilePath);
		else update_last_upload_time();
	} 
#ifdef DEBUG_ON
	else {
		if (result < sizeof(transfer_messages)/sizeof(transfer_messages[0]))
			dlog_print(DLOG_ERROR, LOG_TAG, "%s", transfer_messages[(int)(result)]);
		else 
			dlog_print(DLOG_ERROR, LOG_TAG, "Unknown Error");
	}
#endif
	file_on_progress = 0;
}

static void _on_sending_file_in_progress(sap_file_transaction_h file_transaction,
					 unsigned short int percentage_progress,
					 void *user_data)
{
	dlog_print(DLOG_INFO, TAG, "on_file_progress: Trans:%d, Progress:%d\n",
	     file_transaction, percentage_progress);

//	set_progress_bar_value((float)percentage_progress / 100);
}

static void __set_file_transfer_cb()
{
	sap_file_transfer_set_progress_cb(priv_data.file_socket, _on_sending_file_in_progress, NULL);

	sap_file_transfer_set_done_cb(priv_data.file_socket, _on_send_completed, NULL);

	dlog_print(DLOG_DEBUG, TAG, "SET callbacks for socket :%u", priv_data.file_socket);
}

int send_file(char* file_path)
{
	dlog_print(DLOG_INFO, TAG, "sending file");
	priv_data.result = SAP_FT_TRANSFER_FAIL_SPACE_NOT_AVAILABLE;

//	sprintf(file_path, "%sfile.txt", app_get_shared_resource_path());

	sap_file_transfer_send(priv_data.peer_agent, file_path, &priv_data.file_socket);

	__set_file_transfer_cb();

	if (priv_data.file_socket == NULL && priv_data.result == SAP_FT_TRANSFER_SUCCESS)
		return 0;

	return 1;
}

void on_peer_agent_updated(sap_peer_agent_h peer_agent,
			   sap_peer_agent_status_e peer_status,
			   sap_peer_agent_found_result_e result,
			   void *user_data)
{
	switch (result) {

	case SAP_PEER_AGENT_FOUND_RESULT_DEVICE_NOT_CONNECTED:

		dlog_print(DLOG_DEBUG, TAG, "device is not connected");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_FOUND:
		dlog_print(DLOG_DEBUG, TAG, "SAP_PEER_AGENT_FOUND_RESULT_FOUND, peer_status:%d", peer_status);
		if (peer_status == SAP_PEER_AGENT_STATUS_AVAILABLE) {
			dlog_print(DLOG_INFO, TAG, "STATUS_AVAILABLE");
			priv_data.peer_agent = peer_agent;

		} else {
			priv_data.peer_agent = peer_agent;
			sap_peer_agent_destroy(priv_data.peer_agent);
			priv_data.peer_agent = NULL;
		}

		break;

	case SAP_PEER_AGENT_FOUND_RESULT_SERVICE_NOT_FOUND:

		dlog_print(DLOG_ERROR, TAG, "service not found");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_TIMEDOUT:

		dlog_print(DLOG_ERROR, TAG, "peer agent find timed out");
		break;

	case SAP_PEER_AGENT_FOUND_RESULT_INTERNAL_ERROR:

		dlog_print(DLOG_ERROR, TAG, "peer agent find search failed");
		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown status (%d)", peer_status);
		break;

	}
}

static gboolean _find_peer_agent(gpointer user_data)
{
	struct priv *priv = NULL;
	sap_result_e result = SAP_RESULT_FAILURE;

	priv = (struct priv *)user_data;

	if (priv->agent == 0) {
		dlog_print(DLOG_ERROR, TAG, "service agent is not registered");
		return FALSE;
	}
	result = sap_agent_find_peer_agent(priv->agent, on_peer_agent_updated, NULL);

	if (result == SAP_RESULT_SUCCESS) {
		dlog_print(DLOG_DEBUG, TAG, "find peer call succeeded");
	} else {
		dlog_print(DLOG_ERROR, TAG, "findsap_peer_agent_s is failed (%d)", result);
	}

	return FALSE;
}

gboolean find_peers()
{
	dlog_print(DLOG_INFO, TAG, "find peer calling");
	if (connected) {
		_find_peer_agent(&priv_data);
		return TRUE;
	}
	return FALSE;
}

void cancel_file()
{
	sap_file_transfer_cancel(priv_data.file_socket);
}

static void on_agent_initialized(sap_agent_h agent,
				 sap_agent_initialized_result_e result,
				 void *user_data)
{
	switch (result) {
	case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:

		dlog_print(DLOG_DEBUG, TAG, "agent is initialized");

		priv_data.agent = agent;

		break;

	case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
		dlog_print(DLOG_ERROR, TAG, "duplicate registration");

		break;

	case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
		dlog_print(DLOG_ERROR, TAG, "invalid arguments");

		break;

	case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_ERROR, TAG, "internal sap error");

		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown status (%d)", result);

		break;

	}
}


static void _on_device_status_changed(sap_device_status_e status,
				      sap_transport_type_e transport_type,
				      void *user_data)
{
	dlog_print(DLOG_INFO, TAG, "%s, status :%d", __func__, status);

	connected = status;

	switch (transport_type) {
	case SAP_TRANSPORT_TYPE_BT:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): bt", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_BLE:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): ble", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_TCP:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): tcp/ip", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_USB:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): usb", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_MOBILE:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): mobile", transport_type);
		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown transport_type (%d)", transport_type);
		break;
	}

	switch (status) {
	case SAP_DEVICE_STATUS_DETACHED:
		dlog_print(DLOG_DEBUG, TAG, "device is not connected.");
		sap_peer_agent_destroy(priv_data.peer_agent);
		priv_data.peer_agent = NULL;

		file_on_progress = 0;

		break;

	case SAP_DEVICE_STATUS_ATTACHED:
		dlog_print(DLOG_DEBUG, TAG, "Attached calling find peer now");

		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown status (%d)", status);
		break;
	}
}

gboolean agent_initialize()
{
	int result = 0;
	int c=0;

	do {
		result = sap_agent_initialize(priv_data.agent,
										MY_APPLICATION_ASPID,
										SAP_AGENT_ROLE_CONSUMER,
										on_agent_initialized,
										NULL);
		c++;
		dlog_print(DLOG_WARN, TAG, "SAP ---->>> getRegisteredServiceAgent() >>> %d", result);
	} while (result != SAP_RESULT_SUCCESS && c<50);

	return TRUE;
}

gboolean initialize_sap(void)
{
	sap_agent_h agent = NULL;

	sap_agent_create(&agent);

	priv_data.agent = agent;

	agent_initialize();

	sap_set_device_status_changed_cb(_on_device_status_changed, NULL);

	return TRUE;
}
