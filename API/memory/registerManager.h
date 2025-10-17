/*
 * registerManager.h
 *
 *  Created on: 10 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef API_MEMORY_REGISTERMANAGER_H_
#define API_MEMORY_REGISTERMANAGER_H_

#include "cybsp.h"
#include "dataManager.h"
#include "registerUtils.h"
#include "stdio.h"

#include "cyabs_rtos.h"

#define SEG_LEN_SENHA			16
#define SEG_LEN_SPY_SENHA		12
#define SEG_LEN_BUFF_SENHA		SEG_LEN_SENHA

//#define SEG_IDENT_INVALID       ~(0x0ULL)
#define SEG_IDENT_INVALID       (0x0ULL)
#define SEG_CODE_INVALID        SEG_IDENT_INVALID

// senha root + 5 acessos * 2 (senha = double word)
#define SEG_BUFF_SENHAS_WORD	(1+SEG_QTDE_SENHAS) * 2
#define REGISTER_CODE_MINIMUM_DIGITS 4


#define SEG_SIZE_ADD_CODE		41

typedef enum {
	USER_END = 0,
	USER_TECLADO,
	USER_APP
}type_user_t;

typedef enum {
	SEG_OK_ROOT		= 0,
	SEG_OK_OWNER,
	SEG_OK,
	SEG_INVALIDA,
	SEG_CODE_SHORT,
	SEG_POS_OCUPADA,
	SEG_ERR_CAD,
	SEG_HARD_RESET
} seg_status_e;

typedef enum {
	BLE_COMMIT_INST = 0,
	BLE_COMMIT_OWNER,
	BLE_COMMIT_SALVA,
	BLE_COMMIT_EDIT,
	BLE_COMMIT_APAGA,
	BLE_COMMIT_DEL_ALL_EK,
	BLE_COMMIT_NEW_ROOT,
	BLE_COMMIT_SETOR,
} remote_commit_op;

typedef union {
	struct __attribute__ ((__packed__)) {
		uint32_t perm;      // 4
		uint8_t uuid[16];   // 16
		uint64_t senha; // 8
	};
	uint32_t raw[7];
} ble_root_t;

typedef union {
	struct __attribute__ ((__packed__)) {
		uint32_t perm;      // 4
		uint8_t uuid[16];   // 16
		uint8_t name[16];
		uint64_t senha; // 8
	};
	uint32_t raw[11];
} ble_owner_t;


typedef struct __attribute__ ((__packed__)){

    struct{
        uint8_t pending;
        uint16_t e;
        uint16_t index;
        mem_access_class_e cls;
    }com_single;

	ble_root_t ble_root; // acesso root ekey
	ble_owner_t ble_owner;
	conv_double_t senha_root; // codigo root da fechadura

	uint64_t senha_hard_reset;

	seg_data_t data_aux;
	seg_data_setor_t data_aux_setor;
	seg_data_partition_t data_aux_partition;
	seg_data_sensor_t data_aux_sensor;
	seg_data_pgm_t data_aux_pgm;
	seg_data_teclado_t data_aux_teclado;
	seg_data_gate_t data_aux_gate;
	seg_data_cenario_t data_aux_cenario;

	list_mngr_t ekey;

	// se esta listando root
	uint8_t f_list_root;

	// se ha necessidade de trocar a senha do root
	uint8_t f_troca_senha_root;
	uint8_t f_nova_senha_root;
	uint8_t f_cfrm_senha_root;

	// o indice da senha que esta sendo trocada
	uint8_t f_nova_senha;
	uint8_t f_cfrm_senha;
	//uint8_t f_i_troca_senha		: 3;

	uint8_t i;
	uint8_t buff_senha[SEG_LEN_BUFF_SENHA];
} seguranca_t;




seg_status_e seg_cad_user_from_cmd(mem_access_class_e class, volatile void* data, tipo_ekey_e role, volatile void* new_user_id, uint16_t *id, uint16_t *puid);
seg_status_e seg_ident_valida(mem_access_class_e class, mem_access_class_ident_e class_iden, uint64_t ident, uint32_t uts, uint8_t *nome, uint16_t *id);
seg_status_e seg_ident_valida_setor(uint32_t id, seg_data_setor_t *data_setor);
seg_status_e seg_ident_valida_partition(uint32_t id, seg_data_partition_t *data_setor);
seg_status_e seg_ident_valida_sensor(uint8_t id, seg_data_sensor_t *data_setor);
seg_status_e get_configuration_environment(uint32_t id_setor, seg_data_envionment_t *data_environment);
seg_status_e seg_code_val_acesso(uint8_t *nome);
seg_status_e seg_control_val_acesso(mem_access_class_ident_e mem_access_class_ident, uint8_t *nome, uint8_t *ID, uint8_t button, uint8_t *resource, uint8_t *resource_type);
void seg_inic_buff_senha(void);
uint8_t seg_eh_nova_senha(void);
seg_status_e seg_ble_root_cad(uint8_t name[16], uint8_t *key, uint32_t* new_perm);
seg_status_e seg_owner_cad(uint8_t name[16], uint8_t id[16], uint32_t* new_perm);
seg_status_e seg_ble_commit(remote_commit_op op, uint8_t *outData);
seg_status_e seg_code_root(uint32_t perm, uint64_t code);
void seg_code_root_salva_mem(void);
seg_status_e seg_edit_user(mem_access_class_ident_e class, uint32_t perm, uint8_t *ident, void* data, uint8_t *code, uint16_t *puid);

seg_status_e seg_cad_setor_from_cmd(void* data);
seg_status_e seg_cad_partition_from_cmd(void* data);
seg_status_e seg_cad_sensor_from_cmd(data_add_sensor_t* data);
seg_status_e seg_cad_pgm_from_cmd(data_add_pgm_t* data);
seg_status_e seg_cad_teclado_from_cmd(data_add_teclado_t* data);
seg_status_e seg_edit_pgm(uint8_t *ident, data_add_pgm_t* data);
seg_status_e seg_ident_valida_pgm(uint8_t id, seg_data_pgm_t *data_pgm);
seg_status_e seg_ident_valida_teclado(uint8_t id, seg_data_teclado_t *data_teclado);
seg_status_e seg_ident_valida_gate(uint8_t id, seg_data_gate_t *data_gate);

seg_status_e seg_cad_gate_from_cmd(data_add_gate_t* data);
seg_status_e seg_cad_cenario_from_cmd(data_add_cenario_t* data);

type_user_t get_sync_user_teclado(uint16_t index, seg_data_t *data);
seg_status_e clean_sync_user_teclado(uint16_t index);
seg_status_e get_name_user(uint16_t index, uint8_t *nome);
seg_status_e set_name_user(uint16_t index, uint8_t *nome);
uint32_t get_perm_user(uint32_t senha_teclado);

seg_status_e get_partition(uint16_t index, seg_data_partition_t *data);
seg_status_e get_cenario(uint16_t index, seg_data_cenario_t *data);


#endif /* API_MEMORY_REGISTERMANAGER_H_ */
