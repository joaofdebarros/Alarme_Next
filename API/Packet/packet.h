/*
 * packet.h
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

#ifndef PACKET_PACKET_H_
#define PACKET_PACKET_H_

#include <stdint.h>   // Tipos padrão
#include <stdbool.h>  // Tipo booleano
#include "cy_pdl.h"
#include "cybsp.h"
#include "crc.h"
#include "aes.h"

#define COMMAND_LEN     1
#define TIMESTAMP_LEN   4
#define CRC_LEN         1
#define PRE_DATA_LEN    (COMMAND_LEN+TIMESTAMP_LEN)
#define POS_DATA_LEN    CRC_LEN
#define NON_DATA_SIZE   (PRE_DATA_LEN+POS_DATA_LEN)

#define PROP_ASYNC				(1 << 0)
#define PROP_DISCONNECT 		(1 << 1)

typedef enum{
    PACKET_OK,
    PACKET_FAIL_HEADER,
    PACKET_FAIL_TAIL,
    PACKET_FAIL_LENGTH,
    PACKET_FAIL_ENCRYPT,
    PACKET_FAIL_DECRYPT,
    PACKET_FAIL_UNKNOWN = 0xFF
}packet_error_e;

/* Error defined in Section 4.6.2 in ET001  */
typedef enum {
    PACKET_ERR_OK,
    PACKET_ERR_FAIL,
    PACKET_ERR_UNAUTHORIZED,
    PACKET_ERR_MEM_FULL,
    PACKET_ERR_TIMEOUT,
    PACKET_ERR_TIMESTAMP,
    PACKET_ERR_UNKNOWN = 0xFF,
    // internal errors
    PACKET_ERR_AES_KEY = 0xEE,
}packet_error_t;

#define LENGHT_HEADER  2  // lenght of header
#define LENGHT_IV  16  // lenght of IV

typedef struct {
    uint16_t header;
    uint8_t id;
    uint8_t len;
    uint8_t id_job[16];
    uint8_t iv[16];
    uint8_t data[256];
    uint16_t tail;
} packet_t;

typedef enum AlarmCmd_e{
	ALARMCMD_SET_INST = 1,
	ALARMCMD_EDIT_INST = 5,
	ALARMCMD_COMMIT = 4,
	ALARMCMD_ADD_USER = 2,
	ALARMCMD_SET_USER_TECL = 6,
	ALARMCMD_SET_USER_ROLE = 7,
	ALARMCMD_SET_USER_CONTROL = 0x51,
	ALARMCMD_KEY = 0x01,
	ALARMCMD_NOTIFY = 13,

	ALARMCMD_ARM = 8,
	ALARMCMD_ADD_OWNER = 9,
	ALARMCMD_FIRMWARE_UPDATE = 10,
	ALARMCMD_SETUP_SETOR = 11,
	ALARMCMD_SETUP_PARTITION = 12,
	ALARMCMD_SETUP_NOTIFY = 13,

	LRCMD_FORM_NETWORK = 14,
	LRCMD_JOINED_NETWORK = 15,
	LRCMD_DETECTED = 16,

	ALARMCMD_ADD_SENSOR = 20,
	LRCMD_JOINED_NETWORK_SENSOR = 21,
	LRCMD_DETECTED_SENSOR = 22,

	ALARMCMD_SETUP_SETOR_GENERAL = 23,

	ALARMCMD_ADD_PGM = 25,
	ALARMCMD_SET_PGM = 26,

	ALARMCMD_ADD_MODULE_GATE = 27,
	LRCMD_JOINED_NETWORK_GATE = 28,
	LRCMD_WRITE_CONTROL_GATE = 29,
	LRCMD_READ_STATUS_GATE = 30,

	ALARMCMD_SET_INST_TECLADO = 31,
	ALARMCMD_ADD_TECLADO = 32,
	ALARMCMD_GET_USER_TECLADO,

	ALARMCMD_ADD_CENARIO = 50,

	ALARMCMD_UNKNOWN = 0xFF,

}AlarmCmd_e;

typedef struct{
	uint32_t perm_user;
	uint8_t resource;
	uint8_t arm_desarm;
	uint8_t type_resource;
	uint8_t mode;
}alarm_arm_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint64_t code;
	uint8_t nome[16];
	uint32_t uts_inic;
	uint32_t uts_delta;
	uint16_t min_inic;
	uint16_t min_delta;
	uint8_t days;
}add_code_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t uuid[16];
}set_program_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
}commit_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint64_t code;
}edit_program_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t id[16];
	uint8_t nome[16];
}add_user_c;

typedef struct __attribute__ ((__packed__)){
	uint64_t code;
	uint32_t perm;
	uint32_t perm_user;
}add_user_tecl_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t node;
	uint8_t register_control;
}add_user_control_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t node;
	uint8_t button;
	uint16_t bat;
}detected_user_control_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t node;
	uint8_t register_sensor;
}register_sensor_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t node;
	uint8_t status_sensor;
	uint16_t bat;
}detect_sensor_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t status;
}start_user_control_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t perm_user;
	uint32_t uts_inic;
	uint32_t uts_delta;
	uint16_t min_inic;
	uint16_t min_delta;
	uint8_t days;
	uint8_t resource;
	uint8_t resource_Button1;
	uint8_t resource_Button2;
	uint8_t resource_Button3;
	uint8_t resource_Button4;
	uint8_t type_resource;
	uint8_t type_resource_Button1;
	uint8_t type_resource_Button2;
	uint8_t type_resource_Button3;
	uint8_t type_resource_Button4;
}add_user_role_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t size;
	uint32_t crc32;
}fota_step1_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t step;
	uint8_t block[208];
}fota_step2_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t ID_Setor;
	uint8_t nome[16];
	uint32_t ID_partition;
	uint8_t type_setor;//0:com fio      1:sem fio
	uint8_t trigger_type;
	uint8_t attributes_setor;
	uint8_t ID_PGM;
}set_setor_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t model_resistor;
	uint8_t setup_resistor;
}set_setor_gereral_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t nome[16];
	uint32_t ID_partition;
	uint8_t tempo_entrada;
	uint8_t tempo_saida;
	uint8_t tempo_autoarme;
	uint8_t dias_arme_auto;
	uint8_t dias_desarme_auto;
	uint16_t hora_arme_auto;
	uint16_t hora_dearme_auto;
	uint8_t numero_auto_anular;
	uint8_t tempo_setor_inteligente;
	uint8_t ID_PGM;
	uint32_t numero_conta;
	uint8_t binario_flags;
}set_partition_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t ID_sensor;
	uint32_t ID_Setor;
	uint32_t config_sensor;
	uint8_t Range; //0 - long range, 1 - mid range
	uint8_t type_sensor;// se é IVP, magnetico, interruptor
}set_sensor_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t ID_Setor;
}add_sensor_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t status;
	uint32_t ID_Notify;
}set_notify_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t ID_PGM;
	uint8_t index_rele;
	uint8_t type_trigger;
	uint8_t type_association;
	uint16_t time;
	uint8_t mode;
}set_pgm_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t ID_automation;
}add_gate_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t ID_gate;
	uint8_t command;
}write_gate_c;

typedef struct __attribute__ ((__packed__)){
	uint8_t node;
	uint8_t status;
}read_gate_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint8_t type_get;
	uint8_t status;
}get_user_teclado_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t ID_cenario;
	uint8_t number_action;
}init_cenario_c;

typedef struct __attribute__ ((__packed__)){
	uint32_t perm;
	uint32_t ID_cenario;
	uint8_t index_action;
	uint8_t type;
	uint32_t ID_trigger_action;
	uint8_t mode;
	uint8_t event_command;
	uint8_t timeout;
}add_cenario_c;

// Monster unions
typedef union{
	set_program_c setProgram_c;
	commit_c Commit_c;
	edit_program_c editProgram_c;
	add_code_c addCode_c;
	add_user_c addUser_c;
	add_user_tecl_c addUser_Tecl_c;
	add_user_role_c addUser_Role_c;
	alarm_arm_c alarmArm_c;
	fota_step1_c fotaStep1_c;
	fota_step2_c fotaStep2_c;

	start_user_control_c startUser_Control_c;
	add_user_control_c addUser_Control_c;
	detected_user_control_c detectedUser_Control_c;
	set_setor_c SetSetor_c;
	set_setor_gereral_c setSetor_gereral_c;
	set_partition_c SetPartition_c;
	add_sensor_c AddSensor_c;
	set_sensor_c SetSensor_c;
	register_sensor_c registerSensor_c;
	detect_sensor_c detectSensor_c;
	set_notify_c setNotify_c;
	set_pgm_c setPgm_c;

	add_gate_c AddGate_c;
	write_gate_c writeGate_c;
	read_gate_c readGate_c;

	get_user_teclado_c get_userTeclado_c;

	init_cenario_c initCenario_c;
	add_cenario_c addCenario_c;

	struct{
		uint8_t raw[220];
		uint16_t len;
	};
}pck_cmd_u;

typedef struct{
	    AlarmCmd_e cmd;
		uint32_t timestamp;
		pck_cmd_u data;
		uint8_t properties;
}packet_void_t;

typedef struct{
	    AlarmCmd_e Command;
        uint32_t timestamp;
        uint8_t data[250];
        uint8_t crc;
        uint16_t dLen;
}packet_struct_t;

typedef struct{
    uint8_t encrypted;
    uint16_t puid;
    uint8_t* key;
    uint8_t version;
    uint8_t *inData;
    uint16_t inLen;
    uint8_t* jobID;
}packet_build_params_t;

typedef struct{
    struct{
        uint8_t key[16];
        uint8_t holdKey[16];
    }aes;
    uint16_t Header;
    uint16_t Tail;
    //packet_key_commit_e commitFlag;
}packet_cfg_t;

packet_error_e packet_data_demount(uint8_t *datain, uint16_t len, packet_t *packet);
packet_error_e packet_data_mount(packet_build_params_t *params, packet_t *pck, uint8_t* serial, uint8_t *len);
uint8_t packet_buidPacket(packet_void_t *pck, uint8_t *data);

packet_error_e packet_decrypt_and_get(packet_t *in, uint8_t *key, packet_void_t *out);
packet_error_e packet_get_and_encrypt(packet_void_t *in, uint8_t version, uint16_t puid, uint8_t *key, packet_t *out, uint8_t *serialized, uint8_t *sLen, uint8_t *jobID);
packet_error_t packet_parse_data(uint8_t *data, uint16_t len, packet_struct_t *pck);
packet_error_t packet_parse_data_LORA(uint8_t *data, uint16_t len, packet_struct_t *pck);

////////////////////////////LORA///////////////////////////////////////////////////////
packet_error_e packet_decrypt_and_get_LORA(packet_t *in, uint8_t *key, packet_void_t *out);
packet_error_e packet_data_demount_LORA(uint8_t *datain, uint16_t len, packet_t *packet);
packet_error_e packet_get_and_encrypt_LORA(packet_void_t *in, uint8_t version, uint8_t *key, packet_t *out, uint8_t *serialized, uint8_t *sLen);
packet_error_e packet_data_mount_LORA(packet_build_params_t *params, packet_t *pck, uint8_t* serial, uint8_t *len);


#endif /* PACKET_PACKET_H_ */
