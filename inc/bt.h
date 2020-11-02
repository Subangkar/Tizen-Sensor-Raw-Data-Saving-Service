/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FT_MAIN_H__
#define __FT_MAIN_H__

//#include <app.h>
//#include <Elementary.h>
//#include <system_settings.h>
//#include <efl_extension.h>
#include <dlog.h>

#include <sap.h>
#include <sap_file_transfer.h>


#define TAG "X_rawsensordata_FTSender"

#define MY_APPLICATION_ASPID "/sample/filetransfer" // drgbtppg rawsensordata
#define MY_APPLICATION_CHANNELID 107


#define gboolean int

#define EDJ_FILE "edje/setting.edj"
#define GRP_MAIN "main"

char file_on_progress;

//gboolean find_peers();
//gboolean send_file();
//void     add_send_button();
gboolean initialize_sap();
//void     set_progress_bar_value(float percentage);
//void     show_ft_result_popup(char *message, gboolean pop_parent);
//void     cancel_file();

void naviframe_popped_cb();

struct priv {
	sap_agent_h agent;
	sap_file_transaction_h file_socket;
	sap_peer_agent_h peer_agent;
};

#endif /* __FT_MAIN_H__ */
