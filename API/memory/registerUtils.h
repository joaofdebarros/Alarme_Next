/*
 * registerUtils.h
 *
 *  Created on: 10 de mar. de 2025
 *      Author: diego.marinho
 */

#ifndef API_MEMORY_REGISTERUTILS_H_
#define API_MEMORY_REGISTERUTILS_H_

#include "cybsp.h"
#include "timestamp.h"
#include "dataManager.h"

typedef struct __attribute__ ((packed)) {
	union {
	    int16_t end;
	    int16_t indice; // para biometria, temos o indice
	};
	uint16_t count;
} list_mngr_t;

typedef struct __attribute__ ((__packed__)) {
	// mudar estrutura do code pra incluir uuid aqui
	uint32_t perm_solic;
	uint64_t senha_tecl;
	uint8_t uuid[16];
	uint32_t uts_inic;
	uint32_t uts_delta;
	uint16_t min_inic;
	uint16_t min_delta;
	uint8_t days;		// 1
	uint8_t aes[16];
} data_add_t;

typedef struct __attribute__ ((__packed__)) {
	// mudar estrutura do code pra incluir uuid aqui
	uint32_t perm_solic;
	uint8_t uuid[16];
	uint8_t name[16];
} data_add_remote_t;

typedef struct __attribute__ ((__packed__)) {
	// mudar estrutura do code pra incluir uuid aqui
	//uint64_t senha_tecl;
	uint32_t perm_solic;
	uint32_t perm_user;
	uint32_t uts_inic;
	uint32_t uts_delta;
	uint16_t min_inic;
	uint16_t min_delta;
	uint8_t days;
	uint8_t resource;
	uint8_t resource_button1;
	uint8_t resource_button2;
	uint8_t resource_button3;
	uint8_t resource_button4;
	uint8_t type_resource;
	uint8_t type_resource_Button1;
	uint8_t type_resource_Button2;
	uint8_t type_resource_Button3;
	uint8_t type_resource_Button4;
} data_add_user_t;

typedef struct __attribute__ ((__packed__)) {
	uint32_t perm;
	uint32_t ID_Setor;
	uint8_t nome[16];
	uint32_t ID_partition;
	uint8_t type_setor;//0:com fio      1:sem fio
	uint8_t trigger_type;
	uint8_t attributes_setor;
	uint8_t ID_PGM;
} data_add_setor_t;


typedef struct __attribute__ ((__packed__)) {
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
} data_add_partition_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t ID_sensor;
	uint32_t ID_Setor;
	uint32_t config_sensor;
	uint8_t Range_type; //se é long range, mid range e se é IVP, magnetico, interruptor
} data_add_sensor_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t ID_PGM;
	uint8_t index_rele;
	uint8_t type_trigger;
	uint8_t type_association;
	uint16_t time;
	uint8_t mode;
} data_add_pgm_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t ID_TECLADO;
} data_add_teclado_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t ID_gate;
	uint32_t ID_automation;
	uint8_t Range_type; //se é long range, mid range e se é IVP, magnetico, interruptor
} data_add_gate_t;

typedef struct __attribute__ ((__packed__)) {
    uint32_t ID_cenario;        // Identificador único do cenário
    uint8_t  qtd_acoes;         // Quantidade de ações válidas (1 a 4)
    uint8_t  valido;            // Flag de validade (0xAA = ativo, 0xFF = apagado)
    uint8_t  reservado[2];      // Alinhamento ou futuro uso

    // ---- Trigger ----
    uint8_t  trigger_type;
    uint32_t trigger_ID_trigger_action;
    uint8_t  trigger_mode;
    uint8_t  trigger_event_command;
    uint8_t  trigger_timeout;

    // ---- Ações (até 4) ----
    struct __attribute__((__packed__)) {
        uint8_t  type;
        uint32_t ID_trigger_action;
        uint8_t  mode;
        uint8_t  event_command;
        uint8_t  timeout;
    } acoes[MAX_ACTIONS_PER_SCENARIO];
} data_add_cenario_t;

typedef enum {
	ROLE_PROGRAM	= 0,
	ROLE_MASTER	= 1,
	ROLE_COMMUM	= 2,
} tipo_ekey_e;


///////////////////////////////////////////////////USUÁRIOS/////////////////////////////
typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){
		//uint64_t id; // 8
		uint64_t Tecl_Code; // 8
		uint32_t Access_code;
		uint8_t Control;
		char nome[16]; // 16
		char ID[16]; // 16
		uint32_t uts_inic; // uts incial que o id se torna valido // 4
		uint32_t uts_delta; // segundos a partir do inicial que o id eh valido // 4
		// caso o acesso seja delimitado por uma faixa de horario
		// se min inicial e delta forem == 0, o acesso eh valido pelo dia inteiro
		uint8_t f_unico : 1; // se o acesso unico ja ocorreu
		uint8_t f_inval : 1; // se o acesso se tornou invalido
		uint16_t min_inic	: 11; // minuto no dia a partir deste horario
		uint16_t min_delta	: 11; // delta de minutos para id ser invalido naquele dia // 3
		dias_t d_acesso; // 1
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
		//uint8_t aes[16];
	};
	struct __attribute__ ((__packed__)) {
		// o campo ID na estrutura abaixo eh composto pela
		// permissao, role e indice
		uint32_t perm; // 4
		tipo_ekey_e role : 2;
		uint16_t : 14;
		uint16_t indice; // nao utilizado, devido a exclusividade da permissao // 4
		uint8_t uuid[16]; // 52
	};
	uint32_t raw_data[MEM_DATA_SIZE]; // 52 bytes
} seg_data_t;


typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){
		char nome[16];
		uint32_t ID_setor;
		uint32_t ID_partition;
		uint8_t type_setor;
	    union {
	        struct {
	            uint8_t setor_imediato : 1;
	            uint8_t setor_temporizado : 1;
	            uint8_t setor_seguidor : 1;
	            uint8_t setor_inteligente : 1;
	            uint8_t setor_reserved : 4;
	        };
	        uint8_t trigger_type;
	    };
	    // Características binárias usando 'union' para economizar espaço
	    union {
	        struct {
	            uint8_t setor_silenciososo : 1;
	            uint8_t setor_auto_anulavel : 1;
	            uint8_t setor_24 : 1;
	            uint8_t setor_interior : 1;
	            uint8_t setor_inibir : 1;
	            uint8_t attributes_reserved : 3;
	        };
	        uint8_t attributes_setor;         // Armazenamento compactado dos 8 bits binários (uint8_t)
	    };
	    uint8_t ID_PGM;

	};
	uint32_t raw_data[MEM_DATA_SETOR_SIZE];

}seg_data_setor_t;


typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){
		char nome[16];
		uint32_t ID_partition;
		uint8_t tempo_entrada;
		uint8_t tempo_saida;
		uint8_t tempo_auto_arme;
		uint8_t dias_arme_auto;
		uint8_t dias_desarme_auto;
		uint16_t hora_arme_auto;
		uint16_t hora_desarme_auto;
		uint8_t numero_auto_anular;
		uint8_t tempo_setor_inteligente;
		uint8_t ID_PGM;
		uint32_t numero_conta;


	    // Flags binaris organizadas em union para otimização
	    union {
	        struct {
	            uint8_t habilita_partition : 1;
	            uint8_t auto_arme         : 1;
	            uint8_t funcao_lapso    : 1;
	            uint8_t arme_forcado    : 1;
	            uint8_t auto_arme_particao  : 1;
	            uint8_t habilita_particao_virtual : 1;
	            uint8_t particao_comum : 1;
	            uint8_t auto_arme_particao_comum   : 1;
	        };
	        uint8_t binario_flags;         // Armazenamento compactado dos 8 bits binários (uint8_t)
	    };
	};
	uint32_t raw_data[MEM_DATA_PARTITION_SIZE];

}seg_data_partition_t;

typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){

		uint8_t ID_Sensor;
		uint32_t ID_Setor;
		uint8_t Range_type; //se é long range, mid range e se é IVP, magnetico, interruptor


	    // Flags binaris organizadas em union para otimização
	    union {
	        struct {
	            uint8_t config_sensor : 1;
	            uint32_t Sensor_Reserved         : 31;
	        };
	        uint32_t binario_flags;         // Armazenamento compactado dos 16 bits binários (uint16_t)
	    };
	};
	uint32_t raw_data[MEM_DATA_SENSOR_SIZE];

}seg_data_sensor_t;


typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){

		uint8_t ID_PGM;
		uint8_t index_rele;
		uint8_t type_trigger;
		uint8_t type_association;
		uint16_t time;
		uint8_t mode;
	};
	uint32_t raw_data[MEM_DATA_PGM_SIZE];

}seg_data_pgm_t;

typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){

		uint8_t ID_TECLADO;

	};
	uint32_t raw_data[MEM_DATA_TECLADO_SIZE];

}seg_data_teclado_t;

typedef union __attribute__ ((__packed__)){
	struct __attribute__ ((__packed__)){

		uint8_t ID_gate;
		uint32_t ID_automation;
		uint8_t Range_type;

	};
	uint32_t raw_data[MEM_DATA_TECLADO_SIZE];

}seg_data_gate_t;


typedef union __attribute__((__packed__)) {
    struct __attribute__((__packed__)) {

        // ---- Cabeçalho do cenário ----
        uint32_t ID_cenario;        // Identificador único do cenário
        uint8_t  qtd_acoes;         // Quantidade de ações válidas (1 a 4)
        uint8_t  valido;            // Flag de validade (0xAA = ativo, 0xFF = apagado)
        uint8_t  reservado[2];      // Alinhamento ou futuro uso

        // ---- Trigger ----
        uint8_t  trigger_type;
        uint32_t trigger_ID_trigger_action;
        uint8_t  trigger_mode;
        uint8_t  trigger_event_command;
        uint8_t  trigger_timeout;

        // ---- Ações (até 4) ----
        struct __attribute__((__packed__)) {
            uint8_t  type;
            uint32_t ID_trigger_action;
            uint8_t  mode;
            uint8_t  event_command;
            uint8_t  timeout;
        } acoes[MAX_ACTIONS_PER_SCENARIO];

    };

    // Representação bruta para gravação direta na Flash
    uint32_t raw_data[MEM_DATA_CENARIO_SIZE];

} seg_data_cenario_t;


typedef struct{
	seg_data_setor_t seg_data_setor;
	seg_data_partition_t seg_data_partition;
}seg_data_envionment_t;


uint8_t seg_fill_data(seg_data_t* pSeg, data_add_t* data);
uint8_t seg_fill_data_remote(seg_data_t* pSeg, data_add_remote_t* data);
uint8_t seg_fill_data_user(seg_data_t* pSeg, data_add_user_t* data);
uint8_t seg_fill_data_setor(seg_data_setor_t* pSeg, data_add_setor_t* data);
uint8_t seg_fill_data_partition(seg_data_partition_t* pSeg, data_add_partition_t* data);
uint32_t perm_nova_get(uint32_t root_perm);

#endif /* API_MEMORY_REGISTERUTILS_H_ */
