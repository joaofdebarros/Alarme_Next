/*
 * app.h
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

#ifndef APP_H_
#define APP_H_

#include <stdint.h>   // Tipos padrão
#include <stdbool.h>  // Tipo booleano
#include "packet.h"
#include "cyabs_rtos.h"
#include "alarm.h"
#include "Connectivity.h"
#include "zonas.h"
#include "NotifySystem.h"
#include "timestamp.h"
#include "watchdog.h"
#include "eeprom.h"
#include "registerManager.h"
#include "4G.h"
#include "ExternalMemory.h"
#include "LongRange.h"
#include "firmwareUpdate.h"
#include "commum_bus.h"


#define SEC_TO_MS(seconds)              seconds*1E3

#define PDL_DC_TIMEOUT        SEC_TO_MS(10)

#define PARTITION_ARMED 1
#define PARTITION_DISARMED 0

typedef enum{
	PGM_DISARM_PARTITION = 0,
	PGM_ARM_PARTITION,
	PGM_ARM_DISARM_PARTITION,
	TOOGLE_RELE,
	SETOR_VIOLATION
}type_association_t;

typedef enum{
	RESOURCE_PARTITION = 0,
	RESOURCE_PGM,
	RESOURCE_SETOR
}type_resource_t;

typedef enum{
	MODE_TOOGLE = 1,
	MODE_PULSED,
	MODE_RETENTION
}app_mode_pgm_t;

typedef union{
    struct {
        type_resource_t type_source : 3; //se é pgm ou se é partição ou etc
        type_association_t type_trigger : 5;
    };
    uint8_t resource_type_byte;
}resource_type_t;

typedef union{
    struct {
        uint8_t setor_imediato : 1;
        uint8_t setor_temporizado : 1;
        uint8_t setor_seguidor : 1;
        uint8_t setor_inteligente : 1;
        uint8_t setor_reserved : 4;
    };
    uint8_t trigger_setor_byte;
}trigger_type_setor_t;

typedef union{
    struct {
        uint8_t setor_silenciososo : 1;
        uint8_t setor_auto_anulavel : 1;
        uint8_t setor_24 : 1;
        uint8_t setor_interior : 1;
        uint8_t setor_inibir : 1;
        uint8_t attributes_reserved : 3;
    };
    uint8_t attributes_setor_byte;
}attributes_setor_t;


typedef enum{
    APP_EVENT_NONE =                         0,
    APP_EVENT_CONNEC =                       0x00000001,
    APP_EVENT_ZONAS =                        0x00000002,
	APP_EVENT_NOTIFY =                       0x00000004,
	APP_EVENT_LTE =                          0x00000008,
	APP_EVENT_LR =                           0x00000010,
	APP_EVENT_TECLADO =                      0x00000020,
	APP_EVENT_WIFI_BLE =                     0x00000040,
	APP_EVENT_FOTA =                         0x00000080,
	APP_EVENT_TIMEOUT =                      0x00000100,
	APP_EVENT_LONGRANGE =                    0x00000200,
	APP_EVENT_SETORTIMEOUT =                 0x00000400,
	APP_EVENT_NOTIFYTIMEOUT =                0x00000800,
	APP_EVENT_SIRENE =                       0x00001000,
	APP_EVENT_SMART_PARTITION_TIMEOUT =      0x00002000,
	APP_EVENT_PGM =				  			 0x00004000,
	APP_EVENT_PGMRESPONSE =  	 	         0x00008000,
	APP_EVENT_BUTTON =  	 	             0x00010000,
	APP_EVENT_SYNC_TECLADO =  	 	         0x00020000,
	APP_EVENT_CENARIO =  	 	             0x00040000,
	APP_EVENT_POWER	=			  			 0x00080000,
    APP_EVENT_ALL =                          0x00FFFFFF
}pdl_event_e;

typedef enum{
    PDL_TIMESTAMP_CHECK_OK,
    PDL_TIMESTAMP_CHECK_FAIL
} pdl_timestamp_check_e;

typedef enum{
	Bluetooth = 0,
	Cloud
}Wireless_mode_t;

typedef enum{
	Module_4G = 0,
	Module_Wifi_BLE,
}Remote_Mode_t;

typedef struct{
    hEvents_t handle;
    uint32_t mask;
}pdl_event_t;

typedef struct{
    packet_error_t err;
    AlarmCmd_e lastCmd;
    AlarmCmd_e lastCmd_ASYNC;
    data_add_t add_param;
}app_packet_t;

typedef struct{
	bool state_arm;
	uint8_t command;
}State_Alarm_t;


typedef struct{
    struct{
        uint8_t aes[16];
        uint16_t puid;
    }user;
	packet_t packet;
	packet_void_t blePacket;
	uint32_t lastTimestamp;
}app_connectivity_t;

typedef struct{
	seg_data_t data_user;
	uint8_t count;
	uint8_t count_teclado;
	type_user_t type_user;
}sync_teclado_cloud_t;

typedef struct{
    struct{
        uint8_t aes[16];
        uint16_t puid;
    }user;
	packet_teclado_t packet;
	packet_teclado_void_t TecladoPacket;
	uint32_t lastTimestamp;

	bool init_register;
	uint8_t ID_TECLADO;
	uint8_t number_TECLADO_register;
	teclado_registrado_t app_teclado_registrado[3];
	uint8_t index_teclado;
	TecladoCmd_e lastCmd;

	uint8_t count_name;
	uint8_t count_partition;
	uint8_t number_user;

	sync_teclado_cloud_t sync_teclado_cloud;

	uint8_t Address_Assin;
}app_teclado_t;

typedef enum{
  CONTROL = 0,
  MOTION_DETECT,
  OPEN_CLOSE_DETECT,
  GATE
}Type_LR_T;

typedef enum{
  LONG_RANGE = 0,
  MID_RANGE
}Range_LR_t;

typedef union
{
    uint8_t Registerbyte;

    struct
    {
    	Type_LR_T Type                   :5;
    	Range_LR_t range                 :2;
        uint8_t reserved                 :1;
    } Status;

}
Register_Peripheral_Wireless_t;

typedef struct{
	uint32_t User_control;
	uint8_t ID_control;
	Register_Peripheral_Wireless_t Register_Control;
	bool receive_ID;
	bool init_register;
	bool error_Registercontrol;
	bool Status_Via_Teclado;
}app_control_t;

typedef union
{
    uint8_t Statusbyte;

    struct
    {
        uint8_t operation             :2;
        uint8_t statusCentral         :1;
        uint8_t reserved              :5;
    } Status;

}
SensorStatus_t;


typedef struct{
	uint8_t ID_sensor;
	uint32_t ID_Setor;
	uint32_t config_sensor;
	SensorStatus_t SensorStatus;
	Register_Peripheral_Wireless_t Register_Sensor;
	bool init_register;
	bool error_Registercontrol;
}app_sensor_t;

typedef enum{
  ABERTO = 1,
  ABRINDO,
  FECHADO,
  FECHANDO,
  SEMIABERTO,
  TRAVADO,
  LENDO_ABRE,
  LENDO_FECHA,
  INICIAL,
} gate_status_t;

typedef struct{
	uint8_t ID_gate;
	uint32_t ID_automation;
	Register_Peripheral_Wireless_t Register_Sensor;
	bool init_register;
	bool error_Registercontrol;
	gate_status_t gate_status_app;
}app_gate_t;

typedef struct {
    uint32_t id_setor;
} evento_violacao_t;

typedef enum {
    SETOR_INSTANTANEO,
    SETOR_TEMPORIZADO,
    SETOR_SEGUIDOR
} tipo_setor_t;



// Histórico de setores violados
typedef struct {
    uint32_t id_setor;
    uint32_t id_partition;
    bool Status_Violado;
    uint8_t num_detection;
    bool status_smart;
    bool status_smart_timeout;
    bool is_setor_smart;
    uint8_t ID_PGM;
    bool status_temporizada;
    bool status_autoanulavel;
} setor_violado_t;

typedef struct{
	uint32_t id_setor;
	uint8_t count_violation;
	bool canceled;
}setor_auto_anulavel_t;

typedef struct {
    bool temporizador_ativo;
    uint8_t time_atual;
    uint8_t tempo_entrada;
    uint8_t PartitionArmed;
    bool alarme_disparado;
    setor_violado_t setores_violados[20];
    setor_auto_anulavel_t setor_auto_anulavel[10];
    uint8_t num_setores_violados;
    uint8_t number_setor_smart;
    uint8_t count_smart_timeout;
    uint8_t number_to_anular;
    uint8_t tempo_smart;
    bool temporizador_ativo_smart;
} estado_particao_t;

typedef struct {
    uint32_t id_particao;
    estado_particao_t estado;
} particao_registrada_t;

typedef struct{
	estado_particao_t estado_particao;
	notify_struct_u notify_send;
}environment_t;

typedef struct{
	notify_struct_u n_app;
	bool status_notification;
	bool status_temporizado;
	bool status_seguidor;
	bool status_inteligente;
	uint32_t id_partition;
}app_notify_t;



typedef struct{
	bool init_register;
	uint8_t ID_PGM;
	uint8_t number_PGM_register;
	pgm_registrado_t app_pgm_registrado[5];
	bool toogle_pgm;
}app_pgm_t;

typedef enum
{
	MQTT_DISCONECTED = 0,
	MQTT_CONECTED
} status_mqtt_t;

typedef struct{
	status_mqtt_t status_MQTT_LTE;
	status_mqtt_t status_MQTT_CON;
}status_MQTT_t;

typedef enum{
	CENARIO_PARTITION = 0,
	CENARIO_PGM,
	CENARIO_GATE,
	CENARIO_SENSOR,
	CENARIO_VIOLATION
}type_cenario_t;

typedef enum{
	GATE_EVENT = 0,
	GATE_OPENED,
	GATE_OPENING,
	GATE_CLOSED,
	GATE_CLOSING,

	PARTITION_ARM = 10,
	PARTITION_DISARM,
	PARTITION_EVENT
}cenario_trigger_t;




typedef struct{
	uint8_t type;
	uint32_t ID_trigger_action;
	uint8_t mode;
	uint8_t event_command;
	uint8_t timeout;
}action_cenario_t;

typedef struct{
	uint32_t ID_cenario;
	action_cenario_t action_cenario[5];
}cenario_memoria_emulated_t;



typedef struct{
	bool status_trigger;
    bool temporizador_ativo;
    bool trigger_on;
    uint8_t time_atual;
    uint8_t pointer_action;
    uint8_t number_action;
    data_add_cenario_t action_cenario;
}cenario_t;



typedef struct{
	cy_event_t app_event;
	cy_rslt_t status_rtos_app;
	cy_thread_t task_app;

	uint8_t aes[16];

	app_connectivity_t app_connectivity;
	app_teclado_t app_teclado;
	pdl_event_t event;
	State_Alarm_t State_Alarm;

	uint8_t Mode_Cloud;
	app_packet_t packet;
	bool setting_program;
	Remote_Mode_t Remote_Mode;
	Wireless_mode_t Wireless_mode;
	app_control_t app_control;
	app_sensor_t app_sensor;
	environment_t environment;
	particao_registrada_t particoes_estado[8];
//	pgm_registrado_t pgm_registrado[5];
	uint8_t pgm_count;
	app_notify_t app_notify[NUM_NOTIFY];
	uint8_t count_notify;
	uint8_t Assync_id_job[16];
	bool status_sirene;

	app_pgm_t app_pgm;
	app_gate_t app_gate;

	status_MQTT_t status_MQTT;

	data_add_cenario_t cenario_register;

	cenario_memoria_emulated_t cenario_memory[2];
	cenario_t cenario_actual[3];
}app_h;



extern app_h app;

void opPDL_init();
void app_task(void *pvParameters);



#endif /* APP_H_ */
