/*
 * app.c
 *
 *  Created on: 24 de fev. de 2025
 *      Author: diego.marinho
 */

#include "app.h"

app_h app;

setup_zonas_t teste_zona;
mqtt_config_t mqtt_test;
uint32_t perm_emulado;
uint32_t perm_instalador;
uint32_t perm_master;



uint32_t readMemory;
uint8_t write_memory[150];
uint8_t read_memory[150];

extern seguranca_t seg;

//pgm_registrado_t all_pgm[5];

extern cy_stc_cryptolite_aes_buffers_t aesBuffers;
extern cy_stc_cryptolite_aes_state_t aes_state;
uint8_t dec_test[128];
/* Create buffer for transmit data */
uint8_t pSendData[256];
/* Create Buffers to read data */
uint8_t pReadData[260];
uint8_t FunctionStatus;

bool send_response = false;

uint8_t test_TX = 0xAA;

uint8_t status_Button = 4;

/* Assign CONNECTIVITY_TASK */
#define APP_TASK_NAME            ("Connectivity")
#define APP_TASK_STACK_SIZE      (configMINIMAL_STACK_SIZE)
#define APP_TASK_PRIORITY        (tskIDLE_PRIORITY )

estado_particao_t *buscar_estado_particao(uint32_t id, uint8_t *indice);
void registrar_particao(uint32_t id,uint8_t *indice);
void registrar_pgm(uint8_t uid0, uint8_t uid1, uint8_t uid2, uint8_t uid3,
                   uint8_t crc);
bool adicionar_setor_violado(estado_particao_t *estado, seg_data_setor_t id_setor, seg_data_partition_t id_partition, state_zona_t state_zona, notify_method_e notify_method, uint8_t *i_sensor_violated);

estado_particao_t *buscar_estado_particao(uint32_t id, uint8_t *indice) {
    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id) {
            if(indice != NULL){
            	*indice = i;
            }
            return &app.particoes_estado[i].estado;
        }
    }
    return NULL;
}

void registrar_particao(uint32_t id, uint8_t *indice) {
    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == 0) { // 0 = slot livre
        	app.particoes_estado[i].id_particao = id;
            memset(&app.particoes_estado[i].estado, 0, sizeof(estado_particao_t));
            if(indice != NULL){
            	*indice = i;
            }
            return;
        }
    }
}

void get_smart_setorNumber(uint32_t id, uint8_t *numero) {
	uint8_t count_aux = 0;
    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id) { // 0 = slot livre
        	for(uint8_t j = 0; j<10; j++){
        		if(app.particoes_estado[i].estado.setores_violados[j].status_smart == true){
        			count_aux++;
        		}
        	}
        	*numero = count_aux;
        	break;
        }
    }
}

void clear_smart_partition(uint32_t id) {
    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id) { // 0 = slot livre
        	for(uint8_t j = 0; j<10; j++){
        		if(app.particoes_estado[i].estado.setores_violados[j].status_smart == true){
        			app.particoes_estado[i].estado.setores_violados[j].status_smart = false;
        			app.particoes_estado[i].estado.setores_violados[j].num_detection = 0;
        			app.particoes_estado[i].estado.count_smart_timeout = 0;
        			//app.particoes_estado[i].estado.setores_violados[j].Status_Violado = false;
        			app.particoes_estado[i].estado.setores_violados[j].status_autoanulavel = false;
        		}
        	}
        	break;
        }
    }
}

void clear_violation_partition(uint32_t id) {
    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id) { // 0 = slot livre
        	for(uint8_t j = 0; j<10; j++){
        		if(app.particoes_estado[i].estado.setores_violados[j].Status_Violado == true){
        			app.particoes_estado[i].estado.setores_violados[j].Status_Violado = false;
        			app.particoes_estado[i].estado.setores_violados[j].status_temporizada = false;
        			app.particoes_estado[i].estado.setores_violados[j].status_autoanulavel = false;
        			app.particoes_estado[i].estado.setores_violados[j].num_detection = 0;
        			app.particoes_estado[i].estado.setores_violados[j].status_smart = false;
        		}
        	}
        	break;
        }
    }
}

void put_smart_setorNumber(uint32_t id_partition) {

    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id_partition) {
        	app.particoes_estado[i].estado.number_setor_smart++;
        	break;
        }
    }
}

void register_autoAnulavel(uint32_t id_partition, uint32_t id_setor) {

    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id_partition) {
        	for(uint8_t j = 0; j < 10; j++){
        		if(app.particoes_estado[i].estado.setor_auto_anulavel[j].id_setor == 0){
        			app.particoes_estado[i].estado.setor_auto_anulavel[j].id_setor = id_setor;
        			return;
        		}
        	}
        }
    }
}

void increment_autoAnulavel(uint32_t id_partition, uint32_t id_setor) {

    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id_partition) {
        	for(uint8_t j = 0; j < 10; j++){
        		if(app.particoes_estado[i].estado.setor_auto_anulavel[j].id_setor == id_setor){
        			app.particoes_estado[i].estado.setor_auto_anulavel[j].count_violation++;
        			if(app.particoes_estado[i].estado.setor_auto_anulavel[j].count_violation == app.particoes_estado[i].estado.number_to_anular){
        			//if(app.particoes_estado[i].estado.setor_auto_anulavel[j].count_violation == 3){
        				app.particoes_estado[i].estado.setor_auto_anulavel[j].canceled = true;
        			}
        			break;
        		}
        	}
        }
    }
}



bool GetStatus_autoAnulavel(uint32_t id_partition, uint32_t id_setor) {

    for (int i = 0; i < 8; i++) {
        if (app.particoes_estado[i].id_particao == id_partition) {
        	for(uint8_t j = 0; j < 10; j++){
        		if(app.particoes_estado[i].estado.setor_auto_anulavel[j].id_setor == id_setor){
        			return app.particoes_estado[i].estado.setor_auto_anulavel[j].canceled;
        		}
        	}
        }
    }
}

int _get_particao(uint8_t mask) {
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            return i; // retorna partição de 1 a 8
        }
    }
    return -1; // nenhum bit setado
}

// --- Utilitários ---
bool adicionar_setor_violado(estado_particao_t *estado, seg_data_setor_t data_setor, seg_data_partition_t data_partition, state_zona_t state_zona, notify_method_e notify_method, uint8_t *i_sensor_violated) {
	notify_struct_u n;
	notify_type_e notifyType;

	bool status_notification;

	for(int i = 0; i < 10; i++){
		if(estado->setores_violados[i].Status_Violado == false){
			estado->setores_violados[i].Status_Violado = true;
	        estado->setores_violados[i].id_setor = data_setor.ID_setor;
	        estado->setores_violados[i].id_partition = data_partition.ID_partition;
	        *i_sensor_violated = i;
	        status_notification = true;
	        estado->setores_violados[i].num_detection++;

	        if(data_setor.setor_inteligente == 1){
	        	estado->setores_violados[i].status_smart_timeout = true;
	        }
	        break;
		}
		else if(estado->setores_violados[i].Status_Violado == true){
			if(estado->setores_violados[i].id_setor == data_setor.ID_setor){
				estado->setores_violados[i].num_detection++;
				*i_sensor_violated = i;
				break;
			}

		}
	}

	if(status_notification == true){
		status_notification = false;
	    if(state_zona == ZONA_INFERIOR_VIOLADA || state_zona == ZONA_SUPERIOR_VIOLADA || state_zona == ZONA_SUPERIOR_VIOLADA){
	    	notifyType = TYPE_SETOR_VIOLATED;
	    }
	    else{
	    	notifyType = TYPE_SETOR_FRAUDE;
	    }

		n = nysy_generateStruct( notify_method, notifyType, (uint8_t*)data_setor.nome, 16,(uint8_t*)data_partition.nome);

		memcpy(&app.app_notify[app.count_notify].n_app, &n, sizeof(notify_struct_u));
		app.app_notify[app.count_notify].status_notification = true;

		if(data_setor.setor_temporizado == 1){
			app.app_notify[app.count_notify].status_temporizado = true;
			app.app_notify[app.count_notify].id_partition = data_partition.ID_partition;
		}

		if(data_setor.setor_seguidor == 1){
			app.app_notify[app.count_notify].status_seguidor = true;
			app.app_notify[app.count_notify].id_partition = data_partition.ID_partition;
		}

		if(data_setor.setor_inteligente == 1){
			app.app_notify[app.count_notify].status_inteligente = true;
			app.app_notify[app.count_notify].id_partition = data_partition.ID_partition;
		}

		app.count_notify = (app.count_notify + 1) % 10;
		return 1;
	}
	return 0;


	return true;
}

bool _get_count_notification(uint8_t *count_notify){
	for(uint8_t i = 0; i < NUM_NOTIFY; i++){
		if(app.app_notify[i].status_notification == true){
			if(count_notify != NULL)
				*count_notify = i;
			return true;
		}
	}
	return false;
}

bool _update_count_notification(uint32_t id_notify){
	for(uint8_t i = 0; i < NUM_NOTIFY; i++){
		if(app.app_notify[i].n_app.id == id_notify){
			if(app.app_notify[i].status_notification == true){
				app.app_notify[i].status_notification = false;
			}
			return true;
		}
	}
	return false;
}

void _clear_partition_notification(char *name){
	for(uint8_t i = 0; i < NUM_NOTIFY; i++){
		if(memcmp(app.app_notify[i].n_app.uuid2,name,16) == 0){
			app.app_notify[i].status_notification = false;
		}
	}
}

void _clear_setorTemporario_notification(uint32_t id_partition){
	for(uint8_t i = 0; i < NUM_NOTIFY; i++){
		if((app.app_notify[i].status_temporizado == true || app.app_notify[i].status_seguidor == true) && app.app_notify[i].status_notification == true){
			if(app.app_notify[i].id_partition == id_partition){
				app.app_notify[i].status_notification = false;
			}
		}
	}
}

void _clear_setorSmart_notification(uint32_t id_partition){
	for(uint8_t i = 0; i < NUM_NOTIFY; i++){
		if(app.app_notify[i].status_inteligente == true && app.app_notify[i].status_notification == true){
			if(app.app_notify[i].id_partition == id_partition){
				app.app_notify[i].status_notification = false;
			}
		}
	}
}

///////////////////////////////////////////function related cenario////////////////////////
int8_t __get_cenario_free(void){
	for(uint8_t i = 0; i < 3; i++){
		if(app.cenario_actual[i].status_trigger == false){
			return i;
		}
	}
	return -1;
}

bool __get_cenario_temporizadorAtivo(void){
	for(uint8_t i = 0; i < 3; i++){
		if(app.cenario_actual[i].temporizador_ativo == true){
			return true;
		}
	}
	return false;
}


bool __get_triggerCenarioTrue(uint32_t ID, type_cenario_t type_cenario, cenario_trigger_t cenario_trigger){
	bool status;
	int8_t index;
	seg_data_cenario_t data_cenario;
	//adicionar buscar na memoria

	for(uint8_t i = 0; i < 5; i++){
		get_cenario(i, &data_cenario);

		//memcpy(&cenario, &app.cenario_memory[i], sizeof(cenario_memoria_emulated_t));// no lugar dela buscar na memoria de n cenarios

		if(data_cenario.trigger_type == type_cenario){
			if(data_cenario.trigger_ID_trigger_action == ID){
				if(data_cenario.trigger_event_command == cenario_trigger){
					status = true;
				}
				else if(type_cenario == CENARIO_PARTITION && data_cenario.trigger_event_command == PARTITION_EVENT){
					status = true;
				}
				else if(type_cenario == CENARIO_GATE && data_cenario.trigger_event_command == GATE_EVENT){
					status = true;
				}
				else{
					status = false;
				}

				if(status == true){
					index = __get_cenario_free();
					if(index != -1){
						app.cenario_actual[index].status_trigger = true;
						app.cenario_actual[index].temporizador_ativo = true;
						app.cenario_actual[index].pointer_action = 0;
						app.cenario_actual[index].time_atual = 0;
						memcpy(&app.cenario_actual[index].action_cenario, &data_cenario, sizeof(seg_data_cenario_t));
						return status;
					}
					else{
						status = false;
					}
				}
				else{
					status = false;
				}
			}
			else{
				status = false;
			}
		}
	}

	return status;


}

void __work_gate_cenario(action_cenario_t *cenario_gate){
	seg_status_e status;
	status = seg_ident_valida_gate(cenario_gate->ID_trigger_action,NULL);
	if(status == SEG_OK){

		packet_void_t sendGate;


		sendGate.cmd = LRCMD_WRITE_CONTROL_GATE;
		//sendLR.data[0] = 1;
		sendGate.timestamp = 0;
		sendGate.data.raw[0] = cenario_gate->ID_trigger_action;
		sendGate.data.raw[1] = 0;
		sendGate.data.len = 2;

		app.packet.lastCmd = ALARMCMD_ADD_MODULE_GATE;
		app.packet.lastCmd_ASYNC = app.packet.lastCmd;


		__Send_Packet_LORA(&sendGate,NULL,1);
	}
}

void __work_pgm_cenario(action_cenario_t *cenario_pgm){
	pgm_cmd_t pgm_cmd;
	seg_status_e status;
	seg_data_pgm_t seg_data_pgm;

    pgm_cmd.ID = PGM_ID;
    status = seg_ident_valida_pgm(cenario_pgm->ID_trigger_action, &seg_data_pgm);

    if(status == SEG_OK){
        pgm_cmd.function = cenario_pgm->mode;
        //pgm_cmd.address = cenario_pgm->ID_trigger_action;
        pgm_cmd.address = seg_data_pgm.ID_PGM-seg_data_pgm.index_rele;
        //pgm_cmd.rele_number = cenario_pgm->chanel;
        pgm_cmd.rele_number = seg_data_pgm.index_rele;

        if(pgm_cmd.function == PGM_TOGGLE){
            pgm_cmd.timebase.state = cenario_pgm->event_command;
            pgm_cmd.timebase.time = 0;
        }
        else if(pgm_cmd.function == PGM_RETENTION){
            pgm_cmd.timebase.state = 1;
            pgm_cmd.timebase.time = cenario_pgm->event_command;
        }


        pgm_start(&pgm_cmd);
    }

}

void __work_partition_cenario(action_cenario_t *cenario_gate){
	uint8_t indice_cenario_partition;
	notify_method_e Method_LR;
	notify_type_e Type_LR;
	notify_struct_u n;
	uint8_t name_cenario[16];

	estado_particao_t *estado = buscar_estado_particao(cenario_gate->ID_trigger_action, &indice_cenario_partition);

	if(cenario_gate->event_command == PARTITION_DISARM){
		estado->PartitionArmed = PARTITION_DISARMED;
		Type_LR = TYPE_PARTITION_DISARMED;

		memset(name_cenario, 0, 16);

		memcpy(name_cenario, "CHEGUEI EM CASA", strlen("CHEGUEI EM CASA") + 1);
	}
	else if(cenario_gate->event_command == PARTITION_ARM){
		estado->PartitionArmed = PARTITION_ARMED;
		Type_LR = TYPE_PARTITION_ARMED;

		memset(name_cenario, 0, 16);

		memcpy(name_cenario, "SAI DE CASA", strlen("SAI DE CASA") + 1);
	}
	else if(cenario_gate->event_command == PARTITION_EVENT){
		if(estado->PartitionArmed == PARTITION_DISARMED){
			estado->PartitionArmed = PARTITION_ARMED;
			Type_LR = TYPE_PARTITION_ARMED;
		}
		else{
			estado->PartitionArmed = PARTITION_DISARMED;
			Type_LR = TYPE_PARTITION_DISARMED;
		}
	}
	if(estado->PartitionArmed == PARTITION_DISARMED){
		if(estado->alarme_disparado == true || estado->temporizador_ativo == true){
			estado->alarme_disparado = false;
			app.status_sirene = false;
			clear_violation_partition(cenario_gate->ID_trigger_action);


		    if(estado->temporizador_ativo == true){
		    	_clear_setorTemporario_notification(cenario_gate->ID_trigger_action);
		    }


			alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 100, ALARM_MODE_ONESHOT);
		}
		if(estado->temporizador_ativo_smart == true){
			estado->temporizador_ativo_smart = false;
			_clear_setorSmart_notification(cenario_gate->ID_trigger_action);
		}

	}

	Method_LR = METHOD_USER_CENARIO;



	seg_data_partition_t partition_notification;
	seg_ident_valida_partition(cenario_gate->ID_trigger_action,&partition_notification);

	n = nysy_generateStruct( Method_LR, Type_LR, name_cenario, 16, (uint8_t*)partition_notification.nome);

	memcpy(&app.app_notify[app.count_notify].n_app, &n, sizeof(notify_struct_u));
	app.app_notify[app.count_notify].status_notification = true;
	app.count_notify = (app.count_notify + 1) % 10;

    cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
}



void __work_cenario(action_cenario_t *cenario_work){
	if(cenario_work->type == CENARIO_PGM){
		__work_pgm_cenario(cenario_work);
	}
	else if(cenario_work->type == CENARIO_GATE){
		__work_gate_cenario(cenario_work);
	}
	else if(cenario_work->type == CENARIO_PARTITION){
		__work_partition_cenario(cenario_work);
	}
}


uint8_t __alarm_get_settings(){
    return app.setting_program;
}

void __alarm_set_settings(uint8_t x){
	app.setting_program = x;
}

uint8_t __app_ArmAlarm(void){
	app.State_Alarm.state_arm = 1;
	return 1;
}

uint8_t __app_DisarmAlarm(void){
	app.State_Alarm.state_arm = 0;
	return 1;
}

void __generate_ID(uint8_t * id){
	uint32_t randomNum;
    for (size_t i = 0; i < 16; i += 4) {
        if (Cy_Cryptolite_Trng(CRYPTOLITE, &randomNum) == CY_CRYPTOLITE_SUCCESS) {
            memcpy(id + i, &randomNum, 4);
        } else {
            // Tratar erro se necessário
        }
    }
}

void __Send_Packet_Wireless(packet_void_t *p, Wireless_mode_t WirelessMode, uint8_t *key, uint8_t WithResponse, uint8_t *jobID){
    static uint8_t __ble_sendReponse[128]; // using this global to reduce stack size fo the task
    uint8_t len;
    packet_t t;
    memset(t.data, 0, 256);  // Zerando 128 bytes manualmente
    t.id = 0;

   packet_get_and_encrypt(p, WithResponse, WirelessMode, key, &t, __ble_sendReponse, &len, jobID);

   if(app.Remote_Mode == 1){
	   packet_transmit_4G(__ble_sendReponse,len);
   }
   else{
	   packet_transmit_connectivity(__ble_sendReponse,len);
   }
}

void __work_sensor_wireless(seg_data_sensor_t *data_sensor){
	//seg_data_sensor_t data_sensor;
	seg_data_envionment_t envionmentSensor;
	uint8_t index_setor_violated;
	seg_data_pgm_t pgm_partition;
	pgm_cmd_t pgm_cmd;
	bool status_send_notify;
	bool status_auto_anulavel;

	//seg_ident_valida_sensor(*ID_sensor,&data_sensor);
	if(data_sensor->ID_Setor != 0){
		get_configuration_environment(data_sensor->ID_Setor,&envionmentSensor); //busca da memória as informações do setor

		estado_particao_t *estado = buscar_estado_particao(envionmentSensor.seg_data_partition.ID_partition, NULL);

		status_auto_anulavel = GetStatus_autoAnulavel(envionmentSensor.seg_data_setor.ID_partition,envionmentSensor.seg_data_setor.ID_setor);

		if((estado->PartitionArmed == 1 && envionmentSensor.seg_data_setor.setor_interior == 0 && status_auto_anulavel == false) || envionmentSensor.seg_data_setor.setor_24 == 1){
			status_send_notify = adicionar_setor_violado(estado, envionmentSensor.seg_data_setor, envionmentSensor.seg_data_partition, ZONA_INFERIOR_VIOLADA,METHOD_SENSOR_WIRELESS, &index_setor_violated);

			if(envionmentSensor.seg_data_setor.setor_temporizado == 1){
				if (!estado->temporizador_ativo && status_send_notify == true) {
					estado->temporizador_ativo = true;
					estado->time_atual = 0;
					estado->tempo_entrada = envionmentSensor.seg_data_partition.tempo_entrada;
					estado->alarme_disparado = false;

					estado->setores_violados[index_setor_violated].status_temporizada = true;
					estado->setores_violados[index_setor_violated].ID_PGM = envionmentSensor.seg_data_setor.ID_PGM;

					if(envionmentSensor.seg_data_setor.setor_auto_anulavel == 1){
						estado->setores_violados[index_setor_violated].status_autoanulavel = true;
					}


					alarm_setAlarm_os(&app.app_event, APP_EVENT_SETORTIMEOUT, 1000, ALARM_MODE_CONTINUOUS);
				}
			}
			else if(envionmentSensor.seg_data_setor.setor_seguidor == 1){
				if (!estado->temporizador_ativo) {
					cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
					estado->alarme_disparado = true;
				}
			}
			else if(envionmentSensor.seg_data_setor.setor_imediato == 1){
				cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
				estado->alarme_disparado = true;
				if(__get_triggerCenarioTrue(app.app_sensor.ID_sensor,CENARIO_VIOLATION,0)==true){
					if(__get_cenario_temporizadorAtivo() == true){
						//setar evento
						alarm_setAlarm_os(&app.app_event, APP_EVENT_CENARIO, 1000, ALARM_MODE_CONTINUOUS);
					}
				}
			}

			else if(envionmentSensor.seg_data_setor.setor_inteligente == 1 && estado->alarme_disparado == false){
				estado->setores_violados[index_setor_violated].ID_PGM = envionmentSensor.seg_data_setor.ID_PGM;
				alarm_setAlarm_os(&app.app_event, APP_EVENT_SMART_PARTITION_TIMEOUT, 1000, ALARM_MODE_ONESHOT);
				estado->temporizador_ativo_smart = true;
				estado->setores_violados[index_setor_violated].status_smart = true;

				if(envionmentSensor.seg_data_setor.setor_silenciososo == 0){
					app.status_sirene = true;
				}

				if(envionmentSensor.seg_data_setor.setor_auto_anulavel == 1){
					estado->setores_violados[index_setor_violated].status_autoanulavel = true;
				}


				if(estado->setores_violados[index_setor_violated].num_detection == 2){
					cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
					estado->alarme_disparado = true;
					estado->temporizador_ativo_smart = false;
					clear_smart_partition(envionmentSensor.seg_data_partition.ID_partition);
					status_send_notify = true;
				}
				else{
					uint8_t number_setorSmart;
					get_smart_setorNumber(envionmentSensor.seg_data_partition.ID_partition,&number_setorSmart);
					if(number_setorSmart == estado->number_setor_smart){
					//if(number_setorSmart == 2){
						cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
						estado->alarme_disparado = true;
						estado->temporizador_ativo_smart = false;
						clear_smart_partition(envionmentSensor.seg_data_partition.ID_partition);
					}
				}
			}

			if(estado->alarme_disparado == true && status_send_notify == true){

				if(envionmentSensor.seg_data_setor.ID_PGM != 0){
	        	    pgm_cmd.ID = PGM_ID;
			    	seg_ident_valida_pgm(envionmentSensor.seg_data_setor.ID_PGM, &pgm_partition);
			    	//if(pgm_partition.type_association == SETOR_VIOLATION){
			    		app_mode_pgm_t app_mode_pgm;
			    		app_mode_pgm = pgm_partition.mode;
		        	    pgm_cmd.function = app_mode_pgm;
		        	    pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
		        	    pgm_cmd.rele_number = pgm_partition.index_rele;
		        	    pgm_cmd.timebase.state = true;
		        	    pgm_cmd.timebase.time = pgm_partition.time*1000;

		        	    pgm_start(&pgm_cmd);
			    	//}
				}


				if(envionmentSensor.seg_data_setor.setor_silenciososo == 0){
					app.status_sirene = true;
					alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 100, ALARM_MODE_ONESHOT);
				}

				if(envionmentSensor.seg_data_setor.setor_auto_anulavel == 1){
					increment_autoAnulavel(envionmentSensor.seg_data_setor.ID_partition,envionmentSensor.seg_data_setor.ID_setor);
				}

			}

		}
	}

	else{//se não tiver na função de alarme
		if(__get_triggerCenarioTrue(app.app_sensor.ID_sensor,CENARIO_SENSOR,0)==true){
			if(__get_cenario_temporizadorAtivo() == true){
				//setar evento
				alarm_setAlarm_os(&app.app_event, APP_EVENT_CENARIO, 1000, ALARM_MODE_CONTINUOUS);
			}
		}
	}

}

void __work_setor_wire(void){
	uint8_t IDSetor;
	state_zona_t stateSetor;
	seg_data_envionment_t envionmentTest = {0};
	uint8_t index_setor_violated;
	seg_data_pgm_t pgm_partition;
	pgm_cmd_t pgm_cmd;
	bool status_send_notify;
	bool status_auto_anulavel;

	Get_SetorViolation(&IDSetor,&stateSetor);//identifica qual canal foi violado
	get_configuration_environment(IDSetor,&envionmentTest); //busca da memória as informações do setor

	estado_particao_t *estado = buscar_estado_particao(envionmentTest.seg_data_partition.ID_partition, NULL);
	if (!estado) {
		registrar_particao(envionmentTest.seg_data_partition.ID_partition, NULL);
		estado = buscar_estado_particao(envionmentTest.seg_data_partition.ID_partition, NULL);
	}

	//if (estado == NULL) continue; // Falha crítica se ainda for NULL

	if(stateSetor == ZONA_INVIOLADA){
		if(envionmentTest.seg_data_setor.setor_inteligente == 1){
		    for (int i = 0; i < 10; i++) {
		    	if(estado->setores_violados[i].id_setor == envionmentTest.seg_data_setor.ID_setor){
			        if (estado->setores_violados[i].status_smart_timeout == true) {
			        	estado->setores_violados[i].status_smart_timeout = false;
			        }
		    	}

		    }
		}
	}

	if(stateSetor != ZONA_INVIOLADA){
		status_auto_anulavel = GetStatus_autoAnulavel(envionmentTest.seg_data_setor.ID_partition,envionmentTest.seg_data_setor.ID_setor);
		if((estado->PartitionArmed == 1 && envionmentTest.seg_data_setor.setor_interior == 0 && status_auto_anulavel == false) || envionmentTest.seg_data_setor.setor_24 == 1){
			status_send_notify = adicionar_setor_violado(estado, envionmentTest.seg_data_setor, envionmentTest.seg_data_partition, stateSetor, METHOD_SENSOR_WIRE, &index_setor_violated);

			if(envionmentTest.seg_data_setor.setor_temporizado == 1){
				if (!estado->temporizador_ativo && status_send_notify == true) {
					estado->temporizador_ativo = true;
					estado->time_atual = 0;
					estado->tempo_entrada = envionmentTest.seg_data_partition.tempo_entrada;
					estado->alarme_disparado = false;

					estado->setores_violados[index_setor_violated].status_temporizada = true;
					estado->setores_violados[index_setor_violated].ID_PGM = envionmentTest.seg_data_setor.ID_PGM;

					if(envionmentTest.seg_data_setor.setor_auto_anulavel == 1){
						estado->setores_violados[index_setor_violated].status_autoanulavel = true;
					}

					alarm_setAlarm_os(&app.app_event, APP_EVENT_SETORTIMEOUT, 1000, ALARM_MODE_CONTINUOUS);
				}
			}
			else if(envionmentTest.seg_data_setor.setor_seguidor == 1){
				if (!estado->temporizador_ativo) {
					cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
					estado->alarme_disparado = true;
				}
			}
			else if(envionmentTest.seg_data_setor.setor_imediato == 1){
				cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
				estado->alarme_disparado = true;
			}

			else if(envionmentTest.seg_data_setor.setor_inteligente == 1 && estado->alarme_disparado == false){
				estado->setores_violados[index_setor_violated].ID_PGM = envionmentTest.seg_data_setor.ID_PGM;
				alarm_setAlarm_os(&app.app_event, APP_EVENT_SMART_PARTITION_TIMEOUT, 1000, ALARM_MODE_ONESHOT);
				estado->temporizador_ativo_smart = true;
				estado->setores_violados[index_setor_violated].status_smart = true;

				if(envionmentTest.seg_data_setor.setor_silenciososo == 0){
					app.status_sirene = true;
				}

				if(envionmentTest.seg_data_setor.setor_auto_anulavel == 1){
					estado->setores_violados[index_setor_violated].status_autoanulavel = true;
				}

				if(estado->setores_violados[index_setor_violated].num_detection == 2){
					cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
					estado->alarme_disparado = true;
					estado->temporizador_ativo_smart = false;
					clear_smart_partition(envionmentTest.seg_data_partition.ID_partition);
					status_send_notify = true;
				}
				else{
					uint8_t number_setorSmart;
					get_smart_setorNumber(envionmentTest.seg_data_partition.ID_partition,&number_setorSmart);
					if(number_setorSmart == estado->number_setor_smart){
					//if(number_setorSmart == 2){
						cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
						estado->alarme_disparado = true;
						estado->temporizador_ativo_smart = false;
						clear_smart_partition(envionmentTest.seg_data_partition.ID_partition);
					}
				}
			}

			if(estado->alarme_disparado == true && status_send_notify == true){

				if(envionmentTest.seg_data_setor.ID_PGM != 0){
	        	    pgm_cmd.ID = PGM_ID;
			    	seg_ident_valida_pgm(envionmentTest.seg_data_setor.ID_PGM, &pgm_partition);
			    	//if(pgm_partition.type_association == SETOR_VIOLATION){
			    		app_mode_pgm_t app_mode_pgm;
			    		app_mode_pgm = pgm_partition.mode;
		        	    pgm_cmd.function = app_mode_pgm;
		        	    pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
		        	    pgm_cmd.rele_number = pgm_partition.index_rele;
		        	    pgm_cmd.timebase.state = true;
		        	    pgm_cmd.timebase.time = pgm_partition.time*1000;

		        	    pgm_start(&pgm_cmd);
			    	//}
				}
				if(envionmentTest.seg_data_setor.setor_silenciososo == 0){
					app.status_sirene = true;
					alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 300, ALARM_MODE_ONESHOT);
				}

				if(envionmentTest.seg_data_setor.setor_auto_anulavel == 1){
					increment_autoAnulavel(envionmentTest.seg_data_setor.ID_partition,envionmentTest.seg_data_setor.ID_setor);
				}

			}


		}
	}
}

void __Send_Packet_LORA(packet_void_t *p, uint8_t *key, uint8_t WithResponse){
    static uint8_t __ble_sendReponse[128]; // using this global to reduce stack size fo the task
    uint8_t len;
    packet_t t;
    memset(t.data, 0, 256);  // Zerando 128 bytes manualmente
    t.id = 0;

   packet_get_and_encrypt_LORA(p, WithResponse, key, &t, __ble_sendReponse, &len);

   packet_transmit_longRange(__ble_sendReponse,len);

}

void __Send_Packet_lte(packet_void_t *p, uint8_t *key, uint8_t WithResponse){
    static uint8_t __ble_sendReponse[128]; // using this global to reduce stack size fo the task
    uint8_t len;
    packet_t t;
    memset(t.data, 0, 256);  // Zerando 128 bytes manualmente
    t.id = 2;

    packet_get_and_encrypt_LORA(p, WithResponse, key, &t, __ble_sendReponse, &len);

   packet_transmit_4G(__ble_sendReponse,len);
}

/*
 * Check if received BLE packet is in sync with internal clock
 * To avoid repeat attack
 */
pdl_timestamp_check_e __pdl_check_timestamp(uint32_t pckTimestamp, seg_status_e reg){
    pdl_timestamp_check_e ret;
    int32_t diff;

    ret = PDL_TIMESTAMP_CHECK_FAIL;
    // check if the timestamp of the system is updated
    // need to be after 28/11/2022, 12:12:25
    if (rtc_get_timestamp() > 1669637545){
        if (pckTimestamp > app.app_connectivity.lastTimestamp){
            // check if time is on the interval of +- 40 seconds
            if (reg <= SEG_OK_OWNER){
            	rtc_set_timestamp(pckTimestamp);
                app.app_connectivity.lastTimestamp = pckTimestamp;
                ret = PDL_TIMESTAMP_CHECK_OK;
            }
            else{
                diff = rtc_get_timestamp() - pckTimestamp;
                // if diff > 0 then the timestamp is in front of the packet
                // else if diff < 0 then the timestamp is in back of the packet
                if (diff <= 30){
                    // the timing is between 2 minutes up, because the
                    // software reset that can be occoured on door lock
                    // and one mintute back
                	app.app_connectivity.lastTimestamp = pckTimestamp;
                    ret = PDL_TIMESTAMP_CHECK_OK;
                }
            }
        }
    }
    else{
        // if timestamp of the system is wrong, than, do not check for timestamp
        ret = PDL_TIMESTAMP_CHECK_OK;
    }

    return ret;
}

/*
 * Special case of the async by central response
 */
void __async_wireless_response(){
    packet_void_t sendBack;
    packet_error_t pError;

    memset(&sendBack, 0, sizeof(packet_void_t));
    pError = PACKET_ERR_UNKNOWN;
    sendBack.cmd = app.packet.lastCmd_ASYNC;
    sendBack.data.len = 1;

    switch (app.packet.lastCmd_ASYNC){
    case ALARMCMD_SET_USER_CONTROL:
        pError = PACKET_ERR_OK;
        sendBack.data.len = 1;

        app.packet.lastCmd_ASYNC = ALARMCMD_UNKNOWN;

        break;
    case LRCMD_JOINED_NETWORK:
    	if(app.app_control.error_Registercontrol == true){
    		app.app_control.error_Registercontrol = false;
    		pError = PACKET_ERR_FAIL;
    	}
    	else{
    		pError = PACKET_ERR_OK;
    	}
    	sendBack.data.raw[1] = app.app_control.ID_control;
    	sendBack.data.raw[2] = app.app_control.Register_Control.Registerbyte;
    	sendBack.data.len = 3;
    	app.packet.lastCmd = LRCMD_JOINED_NETWORK;
    	break;
    case ALARMCMD_ADD_SENSOR:
        pError = PACKET_ERR_OK;
        sendBack.data.len = 1;

        app.packet.lastCmd_ASYNC = ALARMCMD_UNKNOWN;

        break;
    case ALARMCMD_ADD_MODULE_GATE:
        pError = PACKET_ERR_OK;
        sendBack.data.len = 1;

        app.packet.lastCmd_ASYNC = ALARMCMD_UNKNOWN;

        break;
    case LRCMD_JOINED_NETWORK_SENSOR:
    	if(app.app_sensor.error_Registercontrol == true){
    		app.app_sensor.error_Registercontrol = false;
    		pError = PACKET_ERR_FAIL;
    	}
    	else{
    		pError = PACKET_ERR_OK;
    	}
    	sendBack.data.raw[1] = app.app_sensor.ID_sensor;
    	sendBack.data.raw[2] = app.app_sensor.Register_Sensor.Registerbyte;
    	sendBack.data.len = 3;
    	app.packet.lastCmd = LRCMD_JOINED_NETWORK_SENSOR;
    	break;
    case LRCMD_JOINED_NETWORK_GATE:
    	if(app.app_gate.error_Registercontrol == true){
    		app.app_gate.error_Registercontrol = false;
    		pError = PACKET_ERR_FAIL;
    	}
    	else{
    		pError = PACKET_ERR_OK;
    	}
    	sendBack.data.raw[1] = app.app_gate.ID_gate;
    	sendBack.data.raw[2] = app.app_gate.Register_Sensor.Registerbyte;
    	sendBack.data.len = 3;
    	app.packet.lastCmd = LRCMD_JOINED_NETWORK_GATE;
    	break;

    case ALARMCMD_ADD_PGM:
    	pError = PACKET_ERR_OK;

    	sendBack.data.raw[1] = app.app_pgm.number_PGM_register;

        for(uint8_t i = 0; i < app.app_pgm.number_PGM_register; i++){
        	sendBack.data.raw[i*2+2] = app.app_pgm.app_pgm_registrado[i].num;
        	sendBack.data.raw[i*2+3] = app.app_pgm.app_pgm_registrado[i].crc;
        }

        sendBack.data.len = (app.app_pgm.number_PGM_register)*2 + 2;


    	break;

    case ALARMCMD_ADD_TECLADO:
    	pError = PACKET_ERR_OK;

    	sendBack.data.raw[1] = 1;
    	sendBack.data.raw[2] = app.app_teclado.app_teclado_registrado[app.app_teclado.index_teclado].num;
    	sendBack.data.raw[3] = app.app_teclado.app_teclado_registrado[app.app_teclado.index_teclado].crc;
    	sendBack.data.len = 4;

    	break;
    case LRCMD_READ_STATUS_GATE:
    	pError = PACKET_ERR_OK;
    	sendBack.data.raw[1] = app.app_gate.ID_gate;
    	sendBack.data.raw[2] = app.app_gate.gate_status_app;
    	sendBack.data.len = 3;
    	app.packet.lastCmd = LRCMD_READ_STATUS_GATE;
    	break;

    default:
        sendBack.data.len = 0;
        break;
    }



	sendBack.data.raw[0] = pError;
	if (sendBack.data.len > 0){
		send_response = true;
		//app.Remote_Mode = 0;
		__Send_Packet_Wireless(&sendBack,app.Wireless_mode,app.aes,1,app.Assync_id_job);
	}

}

void __async_teclado_response(){
    packet_teclado_void_t sendBack;
    packet_error_t pError;
    sendBack.cmd = app.packet.lastCmd_ASYNC;
    sendBack.data.len = 1;

    switch (app.packet.lastCmd_ASYNC){
    case LRCMD_JOINED_NETWORK:
		app.app_control.Status_Via_Teclado = false;
		sendBack.data.len = 1;

		sendBack.cmd = TECLADOCMD_REGISTER_CONTROL;
		sendBack.timestamp = 35;

		pError = PACKET_ERR_OK;
		app.app_control.Status_Via_Teclado = false;

    	break;
    }
    sendBack.data.raw[0] = pError;
    Send_Packet_Teclado(&sendBack, app.aes, TECLADO_ID, app.app_teclado.Address_Assin, TECLADO_WORK);
}

void __lte_work(){
	packet_void_t *received;
	packet_void_t sendBack;
	packet_error_t pError;

    sendBack.data.len = 0;
    sendBack.timestamp = 0;

	app.app_connectivity.user.puid = app.app_connectivity.packet.id;

    if(packet_decrypt_and_get_LORA(&app.app_connectivity.packet, NULL, &app.app_connectivity.blePacket) == PACKET_FAIL_DECRYPT){
    	pError = PACKET_ERR_AES_KEY;
    }

    received = &app.app_connectivity.blePacket;

    memset(&sendBack, 0, sizeof(packet_void_t));
    pError = PACKET_ERR_UNKNOWN;
    sendBack.cmd = received->cmd;
    sendBack.data.len = 1;
    switch (received->cmd) {
    case 40:
    	if(received->data.len == 1){
    		sendBack.data.raw[1] = '4';
    		sendBack.data.raw[2] = 'c';
    		sendBack.data.raw[3] = ':';
    		sendBack.data.raw[4] = '1';
    		sendBack.data.raw[5] = '1';
    		sendBack.data.raw[6] = ':';
    		sendBack.data.raw[7] = 'a';
    		sendBack.data.raw[8] = 'e';
    		sendBack.data.raw[9] = ':';
    		sendBack.data.raw[10] = 'f';
    		sendBack.data.raw[11] = '2';
    		sendBack.data.raw[12] = ':';
    		sendBack.data.raw[13] = '6';
    		sendBack.data.raw[14] = '2';
    		sendBack.data.raw[15] = ':';
    		sendBack.data.raw[16] = '7';
    		sendBack.data.raw[17] = '6';

    		sendBack.data.len = 18;
    		pError = PACKET_ERR_OK;

    	}
    	break;
    case 41:
    	if(received->data.len == 1){
    		pError = PACKET_ERR_OK;
    		sendBack.data.len = 1;
    		app.status_MQTT.status_MQTT_LTE = received->data.raw[0];
    	}
    	break;
    default:
    	break;
    }

    app.packet.lastCmd = received->cmd;

	sendBack.data.raw[0] = pError;
	if (sendBack.data.len > 0){
		__Send_Packet_lte(&sendBack,NULL,1);
	}
}


void __longRange_work(){
	uint8_t name[16];
	packet_void_t *received;
	packet_void_t sendBack;
	packet_error_t pError;
	uint32_t perm_user; //adicionar a permissão do usuário
	seg_status_e checkRegister;
	notify_method_e Method_LR;
	notify_type_e Type_LR;
	notify_struct_u n;

    sendBack.data.len = 0;
    sendBack.timestamp = 0;

    app.app_connectivity.user.puid = app.app_connectivity.packet.id;

    if(packet_decrypt_and_get_LORA(&app.app_connectivity.packet, NULL, &app.app_connectivity.blePacket) == PACKET_FAIL_DECRYPT){
    	pError = PACKET_ERR_AES_KEY;
    }

    received = &app.app_connectivity.blePacket;

    memset(&sendBack, 0, sizeof(packet_void_t));
    pError = PACKET_ERR_UNKNOWN;
    sendBack.cmd = received->cmd;
    sendBack.data.len = 1;
    switch (received->cmd) {
    case LRCMD_FORM_NETWORK:
    	if (received->data.len == 1 && (app.app_control.init_register == true || app.app_sensor.init_register == true || app.app_gate.init_register == true)){ //cadastro do controle remoto
    		if(received->data.raw[0] == 0){
    			pError = 0;
    			sendBack.data.len = 0;
    			__async_wireless_response();
    		}

    	}
    	break;

    case LRCMD_JOINED_NETWORK:
    	if(received->data.len == 2 && app.app_control.init_register == true){
    		app.app_control.ID_control = received->data.addUser_Control_c.node;

    		//verificar se realmente é um sensor
    		app.app_control.Register_Control.Registerbyte = received->data.addUser_Control_c.register_control;

    		perm_user = app.app_control.User_control;
    		checkRegister = seg_edit_user(MEM_USER_CONTROL, 0, (uint8_t *)&perm_user, NULL, (uint8_t *)&received->data.addUser_Control_c.node, &app.app_connectivity.user.puid);
    		if(checkRegister == SEG_OK){ //adicionar uma condição caso o controle  já esteja cadastrado
    			pError = 0;
    		}
    		else{
    			pError = 1;
    			app.app_control.error_Registercontrol = true;
    		}


    		app.app_control.init_register = false;
    		checkRegister = seg_ble_commit(BLE_COMMIT_EDIT, NULL);//depois isso vai para o commit. Quando simulo cadastro pelo

			sendBack.data.len = 1;
			app.packet.lastCmd_ASYNC = LRCMD_JOINED_NETWORK;

			if(app.app_control.Status_Via_Teclado == false)
				__async_wireless_response();
			else{
				__async_teclado_response();
			}

    	}
    	break;

    case LRCMD_DETECTED:
    	if(received->data.len == 4){
    		uint8_t resource_Button = 0;
    		uint8_t indice_partition;
    		//uint8_t type_resource;
    		seg_data_pgm_t pgm_partition;
    		pgm_cmd_t pgm_cmd;
    		resource_type_t type_resource;


    		app.app_control.ID_control = received->data.detectedUser_Control_c.node;
    		checkRegister = seg_control_val_acesso(MEM_USER_CONTROL,name,&app.app_control.ID_control, received->data.detectedUser_Control_c.button,&resource_Button,&type_resource.resource_type_byte);
    		//type_trigger = (resource_type_t)type_resource;
    		if(checkRegister == SEG_OK){
    			pError = PACKET_ERR_OK; //enviar apenas ok

    			if(type_resource.type_source == RESOURCE_PARTITION){
        			indice_partition = _get_particao(resource_Button);

        			if(app.particoes_estado[indice_partition].id_particao != 0){
        				pError = PACKET_ERR_OK; //enviar apenas ok
            			estado_particao_t *estado = &app.particoes_estado[indice_partition].estado;

            			if(type_resource.type_trigger == PGM_DISARM_PARTITION){
            				estado->PartitionArmed = PARTITION_DISARMED;
            				Type_LR = TYPE_PARTITION_DISARMED;
            			}
            			else if(type_resource.type_trigger == PGM_ARM_PARTITION){
            				estado->PartitionArmed = PARTITION_ARMED;
            				Type_LR = TYPE_PARTITION_ARMED;
            			}
            			else if(type_resource.type_trigger == PGM_ARM_DISARM_PARTITION){
                			if(estado->PartitionArmed == PARTITION_DISARMED){
                				estado->PartitionArmed = PARTITION_ARMED;
                				Type_LR = TYPE_PARTITION_ARMED;
                			}
                			else{
                				estado->PartitionArmed = PARTITION_DISARMED;
                				Type_LR = TYPE_PARTITION_DISARMED;
                			}
            			}


            			if(estado->PartitionArmed == PARTITION_DISARMED){
            				if(estado->alarme_disparado == true || estado->temporizador_ativo == true){
            					estado->alarme_disparado = false;
            					app.status_sirene = false;
            					clear_violation_partition(app.particoes_estado[indice_partition].id_particao);


            				    if(estado->temporizador_ativo == true){
            				    	_clear_setorTemporario_notification(app.particoes_estado[indice_partition].id_particao);
            				    }


            					alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 100, ALARM_MODE_ONESHOT);
            				}
            				if(estado->temporizador_ativo_smart == true){
            					estado->temporizador_ativo_smart = false;
        						_clear_setorSmart_notification(app.particoes_estado[indice_partition].id_particao);
        					}

            				Type_LR = TYPE_PARTITION_DISARMED;
            			}

            			Method_LR = METHOD_USER_CONTROL;

            			seg_data_partition_t partition_notification;
            			seg_ident_valida_partition(app.particoes_estado[indice_partition].id_particao,&partition_notification);

            			n = nysy_generateStruct( Method_LR, Type_LR, name, 16, (uint8_t*)partition_notification.nome);

            			memcpy(&app.app_notify[app.count_notify].n_app, &n, sizeof(notify_struct_u));
            			app.app_notify[app.count_notify].status_notification = true;
            			app.count_notify = (app.count_notify + 1) % 10;

            		    cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);

            		    //tratar PGM
            		    if(partition_notification.ID_PGM != 0){
        	        	    pgm_cmd.ID = PGM_ID;
            		    	seg_ident_valida_pgm(partition_notification.ID_PGM, &pgm_partition);
            		    	if((pgm_partition.type_association == PGM_DISARM_PARTITION && Type_LR == TYPE_PARTITION_DISARMED) || (pgm_partition.type_association == PGM_ARM_PARTITION && Type_LR == TYPE_PARTITION_ARMED) || pgm_partition.type_association == PGM_ARM_DISARM_PARTITION){
				        	    pgm_cmd.function = pgm_partition.mode;
				        	    pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
				        	    pgm_cmd.rele_number = pgm_partition.index_rele;
				        	    pgm_cmd.timebase.state = true;
				        	    pgm_cmd.timebase.time = pgm_partition.time*1000;

				        	    pgm_start(&pgm_cmd);

            		    	}
            		    }
        			}
        			else{
        				pError = 1;
        			}
    			}
    			else if(type_resource.type_source == RESOURCE_PGM){
    				seg_ident_valida_pgm(resource_Button, &pgm_partition);

    				pgm_cmd.ID = PGM_ID;
    				pgm_cmd.function = pgm_partition.mode;
    				pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
    				pgm_cmd.rele_number = pgm_partition.index_rele;
    				pgm_cmd.timebase.state = true;
    				pgm_cmd.timebase.time = pgm_partition.time*1000;

    				pgm_start(&pgm_cmd);

    			}


    		}
    		else{
    			pError = 1;
    		}
    	}
    	sendBack.data.len = 1;
    	break;

    case LRCMD_JOINED_NETWORK_SENSOR:
    	if(received->data.len == 2 && app.app_sensor.init_register == true){
    		data_add_sensor_t data_add_sensor;
    		app.app_sensor.ID_sensor = received->data.registerSensor_c.node;

    		//verificar se realmente é um sensor
    		app.app_sensor.Register_Sensor.Registerbyte = received->data.registerSensor_c.register_sensor;

    		data_add_sensor.ID_Setor = app.app_sensor.ID_Setor;
    		data_add_sensor.ID_sensor = app.app_sensor.ID_sensor;
    		data_add_sensor.Range_type = app.app_sensor.Register_Sensor.Registerbyte;
    		data_add_sensor.config_sensor = 0;


    		//data_add_sensor.ID_sensor = 2;
    		checkRegister = seg_cad_sensor_from_cmd(&data_add_sensor);
    		cy_rtos_delay_milliseconds(200);
    		seg_data_sensor_t teste_sensor;
    		seg_ident_valida_sensor(data_add_sensor.ID_sensor,&teste_sensor);
    		if(checkRegister == SEG_OK){ //adicionar uma condição caso o controle  já esteja cadastrado
    			pError = 0;
    		}
    		else{
    			pError = 1;
    			app.app_sensor.error_Registercontrol = true;
    		}
    		app.app_sensor.init_register = false;
    		//checkRegister = seg_ble_commit(BLE_COMMIT_SETOR, NULL);//depois isso vai para o commit. Quando simulo cadastro pelo

			sendBack.data.len = 1;
			app.packet.lastCmd_ASYNC = LRCMD_JOINED_NETWORK_SENSOR;
    		__async_wireless_response();
    	}
    	break;

    case LRCMD_DETECTED_SENSOR:
    	if(received->data.len == 4){
    		seg_data_sensor_t data_sensor;
    		app.app_sensor.ID_sensor = received->data.detectSensor_c.node;
    		checkRegister = seg_ident_valida_sensor(app.app_sensor.ID_sensor,&data_sensor);
			if(checkRegister == SEG_OK){
				pError = 0;
				__work_sensor_wireless(&data_sensor);
			}
			else{
				pError = 1;
			}

    	}
    	sendBack.data.len = 1;
    	break;
    case LRCMD_JOINED_NETWORK_GATE:
    	if(received->data.len == 2 && app.app_gate.init_register == true){
    		data_add_gate_t data_add_gate;

    		app.app_gate.ID_gate = received->data.registerSensor_c.node;
    		app.app_gate.Register_Sensor.Registerbyte = received->data.registerSensor_c.register_sensor;

    		data_add_gate.ID_gate = app.app_gate.ID_gate;
    		data_add_gate.ID_automation = app.app_gate.ID_automation;
    		data_add_gate.Range_type = app.app_gate.Register_Sensor.Registerbyte;

    		checkRegister = seg_cad_gate_from_cmd(&data_add_gate);
    		cy_rtos_delay_milliseconds(200);
    		seg_data_gate_t teste_gate;
    		seg_ident_valida_gate(data_add_gate.ID_gate,&teste_gate);

    		if(checkRegister == SEG_OK){ //adicionar uma condição caso o controle  já esteja cadastrado
    			pError = 0;
    		}
    		else{
    			pError = 1;
    			app.app_gate.error_Registercontrol = true;
    		}
    		app.app_gate.init_register = false;

			sendBack.data.len = 1;
			app.packet.lastCmd_ASYNC = LRCMD_JOINED_NETWORK_GATE;
    		__async_wireless_response();
    	}
    	break;
    case LRCMD_WRITE_CONTROL_GATE:
    	sendBack.data.len = 0;
    	break;

    case LRCMD_READ_STATUS_GATE:
    	gate_status_t gate_status;
    	seg_data_gate_t seg_data_gate;
    	app.app_gate.ID_gate = received->data.readGate_c.node;

    	checkRegister = seg_ident_valida_gate(app.app_gate.ID_gate,&seg_data_gate);

    	checkRegister = SEG_OK;

    	if(checkRegister == SEG_OK){
    		cenario_trigger_t cenario_trigger;
    		gate_status = received->data.readGate_c.status;
    		app.app_gate.gate_status_app = gate_status;

			app.packet.lastCmd_ASYNC = LRCMD_READ_STATUS_GATE;
    		app.Wireless_mode = Cloud;
    		if(app.status_MQTT.status_MQTT_LTE == MQTT_CONECTED){
    			app.Remote_Mode = 1;
    		}
    		else{
    			app.Remote_Mode = 0;
    		}
    		__async_wireless_response();

    		cenario_trigger = gate_status;

			if(__get_triggerCenarioTrue(app.app_gate.ID_gate,CENARIO_GATE,cenario_trigger)==true){
				if(__get_cenario_temporizadorAtivo() == true){
					//setar evento
					alarm_setAlarm_os(&app.app_event, APP_EVENT_CENARIO, 1000, ALARM_MODE_CONTINUOUS);
				}
			}
    	}

    	sendBack.data.len = 0;
    	break;

    }





    app.packet.lastCmd = received->cmd;

	sendBack.data.raw[0] = pError;
	if (sendBack.data.len > 0){
		__Send_Packet_LORA(&sendBack,NULL,1);
	}
}

void __teclado_work(){
	packet_teclado_void_t *received,sendBack;
	packet_error_t pError;
	seg_status_e checkRegister;
	uint8_t address, function;
	uint8_t name[16];

    if(packet_decrypt_and_get_teclado(&app.app_teclado.packet, app.aes, &app.app_teclado.TecladoPacket) == PACKET_FAIL_DECRYPT){
    	pError = PACKET_ERR_AES_KEY;
    }

    received = &app.app_teclado.TecladoPacket;

    checkRegister = seg_ident_valida_teclado(app.app_teclado.packet.address,NULL);
    checkRegister = SEG_OK;

    //checkRegister; checar se o endereço foi cadastrado
    sendBack.data.len = 0;
    sendBack.timestamp = 0;

    if(checkRegister == SEG_OK || app.app_teclado.packet.function == TECLADO_REGISTER){
        sendBack.cmd = received->cmd;
        sendBack.data.len = 0;
        switch(received->cmd){
        case TECLADOCMD_REGISTER:
        	if (received->data.len == 1){

        		registrar_teclado(received->data.registerTeclado_c.id,&app.app_teclado.index_teclado);
    			data_add_teclado_t teclado_register;

    			teclado_register.ID_TECLADO = received->data.registerTeclado_c.id;

    			checkRegister = seg_cad_teclado_from_cmd(&teclado_register);

    			seg_data_teclado_t test_teclado;

    			seg_ident_valida_teclado(teclado_register.ID_TECLADO,&test_teclado);

    			if(checkRegister == SEG_OK){
    				pError = PACKET_ERR_OK;
            		app.app_teclado.app_teclado_registrado[app.app_teclado.index_teclado].crc = received->data.registerTeclado_c.id;
            		app.app_teclado.app_teclado_registrado[app.app_teclado.index_teclado].num = app.app_teclado.index_teclado;

            		app.packet.lastCmd_ASYNC = ALARMCMD_ADD_TECLADO;
            		__async_wireless_response();
    			}
    			else{
    				pError = PACKET_ERR_FAIL;
    			}

    			address = received->data.registerTeclado_c.id;
    			function = TECLADO_WORK;
    			pError = PACKET_ERR_OK;
    			sendBack.cmd = TECLADOCMD_COMMIT;

        		//SendTeclado.timestamp = rtc_get_timestamp();
    			sendBack.timestamp = 35;
    			sendBack.data.len = 1;

        	}
        	break;
        case TECLADOCMD_ADD_INST:
        	if (received->data.len == 4 && __alarm_get_settings() == 0){
        		uint32_t perm;
        		uint8_t uuid[16];

        		pError = PACKET_ERR_OK;
        		sendBack.data.len = 1;

        		uuid[0] = 1;
        		uuid[15] = 10;//gerar uuid

        		checkRegister = seg_ble_root_cad(uuid, NULL, &perm);
        		if(checkRegister == SEG_OK){
        			pError = PACKET_ERR_OK;
        			seg_code_root(perm, received->data.add_instaladorTeclado_c.code);
        			__alarm_set_settings(1);
        			packet_void_t sendTeclado;

        			address = app.app_teclado.packet.address;
        			function = TECLADO_WORK;
        			sendBack.cmd = TECLADOCMD_ADD_INST;
        			sendBack.timestamp = 35;
        			sendBack.data.len = 1;

//        			sendTeclado.cmd = ALARMCMD_SET_INST;
//        			sendTeclado.timestamp = rtc_get_timestamp();
//        			memcpy(&sendTeclado.data.raw[0], uuid, 16);
//        			memcpy(&sendTeclado.data.raw[16], &perm, 4);
//        			memcpy(&sendTeclado.data.raw[20], (uint8_t*)&received->data.add_instaladorTeclado_c.code, 8);
//        			sendTeclado.data.len = 28;
//        			app.Wireless_mode = Cloud;
//        			__Send_Packet_Wireless(&sendTeclado,app.Wireless_mode,app.aes,0,app.app_connectivity.packet.id_job);
        		}

        	}
        	break;
        case TECLADOCMD_ADD_COMUM:
        	if (received->data.len == 8){
        		checkRegister = seg_ident_valida(MEM_CLASS_TECL, MEM_USER_VIRTUAL, (uint64_t)received->data.add_userTeclado_c.code_requester, 0, NULL, NULL);
        		checkRegister = SEG_OK_ROOT;
        		if(checkRegister == SEG_OK_ROOT && received->data.add_userTeclado_c.code != seg.ble_root.senha){
        			pError = PACKET_ERR_OK;
        			packet_void_t data_emulated;
        			const uint8_t name[16] = "USER";
        			uint32_t perm_teclado;

        			data_emulated.data.addUser_c.perm = 0;

        			memset(data_emulated.data.addUser_c.id, 0, sizeof(data_emulated.data.addUser_c.id));
        			__generate_ID(data_emulated.data.addUser_c.id);

        			memset(data_emulated.data.addUser_c.nome, 0, sizeof(data_emulated.data.addUser_c.nome));

        			snprintf((char*)data_emulated.data.addUser_c.nome, 16, "%s %d", name, app.app_teclado.count_name);

        			app.app_teclado.count_name++;

        			data_add_remote_t *data = (data_add_remote_t*)&data_emulated.data.addUser_c.perm;


        			checkRegister = seg_cad_user_from_cmd(MEM_CLASS_REMOTE,data,ROLE_MASTER,&perm_teclado,NULL,NULL);

        			if(checkRegister == SEG_OK){
        				memset(&data_emulated.data.addUser_Tecl_c, 0, sizeof(data_emulated.data.addUser_Tecl_c));
            			data_emulated.data.addUser_Tecl_c.perm = 0;
            			data_emulated.data.addUser_Tecl_c.perm_user = perm_teclado;
            			data_emulated.data.addUser_Tecl_c.code = received->data.add_userTeclado_c.code;
            			//data_emulated.data.addUser_Tecl_c.resource = 1; //usuário cadastrado via teclado

            			data_add_user_t *data2 = (data_add_user_t*)&(data_emulated.data.addUser_Tecl_c.perm);
                		checkRegister = seg_edit_user(MEM_USER_TECL, 0, (uint8_t *)&data_emulated.data.addUser_Tecl_c.perm_user, data2, (uint8_t *)&data_emulated.data.addUser_Tecl_c.code, NULL);
                		if(checkRegister == SEG_OK){
                			checkRegister = seg_ble_commit(BLE_COMMIT_EDIT, NULL);
                			app.app_teclado.number_user++;
                			if(checkRegister != SEG_OK){
                				pError = PACKET_ERR_UNAUTHORIZED;
                			}
                		}
                		else{
                			pError = PACKET_ERR_UNAUTHORIZED;
                		}


        			}
        			else{
        				pError = PACKET_ERR_UNAUTHORIZED;
        			}




        		}
        		else{
        			pError = PACKET_ERR_UNAUTHORIZED;
        		}

    			address = app.app_teclado.packet.address;
    			function = TECLADO_WORK;
    			sendBack.cmd = TECLADOCMD_ADD_COMUM;
    			sendBack.timestamp = 35;
    			sendBack.data.len = 1;
        	}
        	break;
        case TECLADOCMD_COMMIT:
        	if (app.app_teclado.lastCmd == TECLADOCMD_REGISTER){
        		sendBack.data.len = 0;
        	}
        	break;
        case TECLADOCMD_AUTHENTICATE:
        	if (received->data.len == 4){
        		checkRegister = seg_ident_valida(MEM_CLASS_TECL, MEM_USER_VIRTUAL, (uint64_t)received->data.add_userTeclado_c.code_requester, 0, NULL, NULL);
        		checkRegister = SEG_OK_ROOT;
        		if(checkRegister == SEG_OK_ROOT){
        			pError = PACKET_ERR_OK;
        		}
        		else{
        			pError = PACKET_ERR_UNAUTHORIZED;
        		}

        	}

			address = app.app_teclado.packet.address;
			function = TECLADO_WORK;
			sendBack.cmd = TECLADOCMD_AUTHENTICATE;
			sendBack.timestamp = 35;
			sendBack.data.len = 1;
        	break;

        case TECLADOCMD_ARM_DISARM:
        	if (received->data.len == 6){
        		checkRegister = seg_ident_valida(MEM_CLASS_TECL, MEM_USER_TECL, (uint64_t)received->data.arm_userTeclado_c.code_requester, 0, name, NULL);

        		if(checkRegister <= SEG_OK){
        			uint8_t indice_partition;
        			uint8_t partition;
        			bool arm_disarm;
        			notify_method_e Method_LR;
        			notify_type_e Type_LR;
        			notify_struct_u n;
        			pError = PACKET_ERR_OK;

        			partition = received->data.arm_userTeclado_c.particao;
        			arm_disarm = received->data.arm_userTeclado_c.arm_disarm;

        			indice_partition = _get_particao(partition);

        			if(app.particoes_estado[indice_partition].id_particao != 0){
        				pError = PACKET_ERR_OK; //enviar apenas ok
            			estado_particao_t *estado = &app.particoes_estado[indice_partition].estado;
            			cenario_trigger_t cenario_trigger;

            			if(arm_disarm == 1){
            				estado->PartitionArmed = PARTITION_DISARMED;
            				Type_LR = TYPE_PARTITION_DISARMED;
            				cenario_trigger = PARTITION_DISARM;
            			}
            			else if(arm_disarm == 0){
            				estado->PartitionArmed = PARTITION_ARMED;
            				Type_LR = TYPE_PARTITION_ARMED;
            				cenario_trigger = PARTITION_ARM;
            			}

            			if(estado->PartitionArmed == PARTITION_DISARMED){
            				if(estado->alarme_disparado == true || estado->temporizador_ativo == true){
            					estado->alarme_disparado = false;
            					app.status_sirene = false;
            					clear_violation_partition(app.particoes_estado[indice_partition].id_particao);


            				    if(estado->temporizador_ativo == true){
            				    	_clear_setorTemporario_notification(app.particoes_estado[indice_partition].id_particao);
            				    }


            					alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 100, ALARM_MODE_ONESHOT);
            				}
            				if(estado->temporizador_ativo_smart == true){
            					estado->temporizador_ativo_smart = false;
        						_clear_setorSmart_notification(app.particoes_estado[indice_partition].id_particao);
        					}

            				Type_LR = TYPE_PARTITION_DISARMED;
            			}

            			Method_LR = METHOD_USER_TECLADO;

            			if(__get_triggerCenarioTrue(app.particoes_estado[indice_partition].id_particao,CENARIO_PARTITION,cenario_trigger)==true){
            				if(__get_cenario_temporizadorAtivo() == true){
            					//setar evento
            					alarm_setAlarm_os(&app.app_event, APP_EVENT_CENARIO, 1000, ALARM_MODE_CONTINUOUS);
            				}
            			}


            			seg_data_partition_t partition_notification;
            			seg_ident_valida_partition(app.particoes_estado[indice_partition].id_particao,&partition_notification);

            			n = nysy_generateStruct( Method_LR, Type_LR, name, 16, (uint8_t*)partition_notification.nome);

            			memcpy(&app.app_notify[app.count_notify].n_app, &n, sizeof(notify_struct_u));
            			app.app_notify[app.count_notify].status_notification = true;
            			app.count_notify = (app.count_notify + 1) % 10;

            		    cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);

            		    //tratar PGM
            		    if(partition_notification.ID_PGM != 0){
            	    		seg_data_pgm_t pgm_partition;
            	    		pgm_cmd_t pgm_cmd;
        	        	    pgm_cmd.ID = PGM_ID;
            		    	seg_ident_valida_pgm(partition_notification.ID_PGM, &pgm_partition);
            		    	if((pgm_partition.type_association == PGM_DISARM_PARTITION && Type_LR == TYPE_PARTITION_DISARMED) || (pgm_partition.type_association == PGM_ARM_PARTITION && Type_LR == TYPE_PARTITION_ARMED) || pgm_partition.type_association == PGM_ARM_DISARM_PARTITION){
				        	    pgm_cmd.function = pgm_partition.mode;
				        	    pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
				        	    pgm_cmd.rele_number = pgm_partition.index_rele;
				        	    pgm_cmd.timebase.state = true;
				        	    pgm_cmd.timebase.time = pgm_partition.time*1000;

				        	    pgm_start(&pgm_cmd);

            		    	}
            		    }
        			}
        		}
        		else{
        			pError = PACKET_ERR_FAIL;
        		}

    			address = app.app_teclado.packet.address;
    			function = TECLADO_WORK;
    			sendBack.cmd = TECLADOCMD_ARM_DISARM;
    			sendBack.timestamp = 35;
    			sendBack.data.len = 1;

        	}
        	break;
        case TECLADOCMD_GET_NOME:
        	if (received->data.len == 1){
        		uint8_t indice_user;
        		uint8_t name[16];
        		seg_status_e status_name;
        		indice_user = received->data.get_nameTeclado_c.indice_user;
        		status_name = get_name_user(indice_user, name);
        		if(status_name == SEG_OK){
        			pError = PACKET_ERR_OK;
        			memcpy(&sendBack.data.raw[1], name, 16);
        			sendBack.data.len = 17;
        		}
        		else{
        			pError = PACKET_ERR_FAIL;
        			sendBack.data.len = 1;
        		}

    			address = app.app_teclado.packet.address;
    			function = TECLADO_WORK;
    			sendBack.cmd = TECLADOCMD_GET_NOME;
    			sendBack.timestamp = 35;

        	}
        	break;
        case TECLADOCMD_UPDATE_NOME:
        	if (received->data.len == 17){
        		seg_status_e status_update;
        		status_update = set_name_user(received->data.update_nameTeclado_c.indice_user,received->data.update_nameTeclado_c.name);
        		if(status_update == SEG_OK){
        			pError = PACKET_ERR_OK;
        		}
        		else{
        			pError = PACKET_ERR_FAIL;
        		}

        		sendBack.data.len = 1;

    			address = app.app_teclado.packet.address;
    			function = TECLADO_WORK;
    			sendBack.cmd = TECLADOCMD_UPDATE_NOME;
    			sendBack.timestamp = 35;
        	}
        	break;
        case TECLADOCMD_ADD_CONTROL:
        	if (received->data.len == 4){
        		uint32_t perm;
        		packet_void_t sendLR;
        		perm = get_perm_user(received->data.add_instaladorTeclado_c.code);
        		if(perm != 0){
        			app.app_control.User_control = perm;

            		app.app_control.init_register = true;

            		sendLR.cmd = LRCMD_FORM_NETWORK;

            		sendLR.timestamp = 0;
            		sendLR.data.raw[0] = 0;
            		sendLR.data.raw[1] = 255;
            		sendLR.data.raw[2] = 0; //cadastro de controle
            		sendLR.data.len = 3;

            		__Send_Packet_LORA(&sendLR,NULL,1);

            		pError = PACKET_ERR_OK;

            		app.app_control.Status_Via_Teclado = true;
            		app.app_teclado.Address_Assin = app.app_teclado.packet.address;


        		}
        		else{
        			pError = PACKET_ERR_FAIL;
        		}
        		sendBack.data.len = 1;

    			address = app.app_teclado.packet.address;
    			function = TECLADO_WORK;
    			sendBack.cmd = TECLADOCMD_ADD_CONTROL;
    			sendBack.timestamp = 35;

        	}
        	break;
        case TECLADOCMD_REGISTER_CONTROL:
        	if (received->data.len == 1){
        		sendBack.data.len = 0;
        	}
        	break;
        case TECLADOCMD_ADD_PARTITION:
        	if (received->data.len == 1){

    			pError = PACKET_ERR_OK;
    			packet_void_t data_emulated;
    			const uint8_t name[16] = "PARTITION";
    			//uint32_t perm_teclado;

    			memset(&data_emulated.data.SetPartition_c, 0, sizeof(data_emulated.data.SetPartition_c));

    			data_emulated.data.SetPartition_c.perm = 0;

    			memset(data_emulated.data.SetPartition_c.nome, 0, sizeof(data_emulated.data.SetPartition_c.nome));

    			snprintf((char*)data_emulated.data.SetPartition_c.nome, 16, "%s %d", name, app.app_teclado.count_partition);

    			app.app_teclado.count_partition++;

    			data_add_partition_t *data = (data_add_partition_t*)&data_emulated.data.SetPartition_c.perm;

    			uint32_t temp;

    			Cy_Cryptolite_Trng(CRYPTOLITE, &temp);

    			//data_emulated.data.SetPartition_c.ID_partition = temp;
    			data_emulated.data.SetPartition_c.ID_partition = 0x123456FF;


        		checkRegister = seg_cad_partition_from_cmd(data);

        		seg_data_partition_t partition_read;

        		seg_ident_valida_partition(data_emulated.data.SetPartition_c.ID_partition,&partition_read);

        		if(checkRegister == SEG_OK){
        			uint8_t indice_partition;
        			pError = PACKET_ERR_OK;
        			estado_particao_t *estado = buscar_estado_particao(data_emulated.data.SetPartition_c.ID_partition, &indice_partition);
        			if (!estado) {
        				registrar_particao(data_emulated.data.SetPartition_c.ID_partition, &indice_partition);
        			}

        			//app.particoes_estado[indice_partition].estado.number_to_anular = received->data.SetPartition_c.numero_auto_anular;
        			//app.particoes_estado[indice_partition].estado.tempo_smart = received->data.SetPartition_c.tempo_setor_inteligente;
        		}

        		sendBack.data.len = 1;
        		pError = PACKET_ERR_OK;

        		address = app.app_teclado.packet.address;
        		function = TECLADO_WORK;
        		sendBack.cmd = TECLADOCMD_ADD_PARTITION;
        		sendBack.timestamp = 35;


        	}
        	break;
        case TECLADOCMD_GET_PARTITION:
        	if (received->data.len == 1){
        		uint8_t indice_user;
        		uint8_t name[16];
        		seg_data_partition_t partition_get;
        		seg_status_e status_name;
        		indice_user = received->data.get_nameTeclado_c.indice_user;
        		status_name = get_partition(indice_user, &partition_get);



        		if(status_name == SEG_OK){
        			pError = PACKET_ERR_OK;
        			memcpy(&sendBack.data.raw[1], &partition_get, sizeof(seg_data_partition_t));
        			sendBack.data.len = sizeof(seg_data_partition_t) + 1;
        		}
        		else{
        			pError = PACKET_ERR_FAIL;
        			sendBack.data.len = 1;
        		}

    			address = app.app_teclado.packet.address;
    			function = TECLADO_WORK;
    			sendBack.cmd = TECLADOCMD_GET_PARTITION;
    			sendBack.timestamp = 35;

        	}
        	break;
        }
    }

    app.app_teclado.lastCmd = received->cmd;

	sendBack.data.raw[0] = pError;
	if (sendBack.data.len > 0){
		cy_rtos_delay_milliseconds(200);
		Send_Packet_Teclado(&sendBack, app.aes, TECLADO_ID, address, function);
	}

	//////////////////////emula cadastro controle
	if(app.app_control.Status_Via_Teclado == true){
		app.packet.lastCmd_ASYNC = LRCMD_JOINED_NETWORK;
		cy_rtos_delay_milliseconds(1000);
		__async_teclado_response();
	}


}

void __conn_work(){
	packet_void_t *received,sendBack;
	packet_void_t sendLR;
	packet_error_t pError;
	seg_status_e checkRegister;
	pdl_timestamp_check_e tsCheck = PDL_TIMESTAMP_CHECK_OK;
	uint32_t perm;
	uint8_t name[16];
	notify_method_e Method_work;
	notify_type_e Type_work;
	notify_struct_u n_work;

    sendBack.data.len = 0;
    sendBack.timestamp = 0;

    app.app_connectivity.user.puid = app.app_connectivity.packet.id;

    if(packet_decrypt_and_get(&app.app_connectivity.packet, app.aes, &app.app_connectivity.blePacket) == PACKET_FAIL_DECRYPT){
    	pError = PACKET_ERR_AES_KEY;
    }

    received = &app.app_connectivity.blePacket;

    checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addCode_c.perm, 0, NULL, &app.app_connectivity.user.puid);
    tsCheck = __pdl_check_timestamp(received->timestamp, checkRegister);

    tsCheck = PDL_TIMESTAMP_CHECK_OK;

    //bypassTime = (BLECMD_FIRMWARE_UPDATE + BLECMD_GET_TIMESTAMP + BLECMD_SET_TIMESTAMP) > 0;
    if (tsCheck == PDL_TIMESTAMP_CHECK_OK ){
        pError = PACKET_ERR_UNKNOWN;
        sendBack.cmd = received->cmd;
        sendBack.data.len = 1;
        switch (received->cmd) {
        case ALARMCMD_EDIT_INST:
        	if (received->data.len == 12){
        		sendBack.data.len = 1;
        		if (seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.editProgram_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid) == SEG_OK_ROOT){
        			checkRegister = seg_code_root(received->data.editProgram_c.perm, received->data.editProgram_c.code); //gravar senha na memoria
        			pError = PACKET_ERR_OK;
        		}
        		else{
        			pError = PACKET_ERR_UNAUTHORIZED;
        		}
        	}
        	else{
        		pError = PACKET_ERR_UNAUTHORIZED;
        	}
        	sendBack.data.len = 1;
        	break;
        case ALARMCMD_ADD_USER:
        	if (received->data.len == 36){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addUser_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
        			data_add_remote_t *data = (data_add_remote_t*)&(received->data.addUser_c.perm);
        			checkRegister = seg_cad_user_from_cmd(MEM_CLASS_REMOTE,data,ROLE_MASTER,&perm,NULL,NULL);

        			if(checkRegister == SEG_OK){
            			pError = PACKET_ERR_OK;
            			memcpy(&(sendBack.data.raw[1]), &perm, 4);
            			sendBack.data.len = 5;
        			}
        			else if (checkRegister == SEG_POS_OCUPADA){
        				pError = PACKET_ERR_MEM_FULL;
        			}
                    else{
                        pError = PACKET_ERR_UNAUTHORIZED;
                    }
        		}
        	}
        	break;
        case ALARMCMD_SET_USER_TECL:
        	if (received->data.len == 16){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addUser_Tecl_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
            		//data_add_user_t *data = (data_add_user_t*)&(received->data.addUser_Tecl_c.perm);
            		checkRegister = seg_edit_user(MEM_USER_TECL, 0, (uint8_t *)&received->data.addUser_Tecl_c.perm_user, NULL, (uint8_t *)&received->data.addUser_Tecl_c.code, &app.app_connectivity.user.puid);
            		if(checkRegister == SEG_OK){
            			pError = PACKET_ERR_OK;
            		}
            		else if(checkRegister == SEG_ERR_CAD){
            			pError = PACKET_ERR_FAIL;
            		}
            		sendBack.data.len = 1;
        		}
        	}
        	break;
        case ALARMCMD_SET_USER_ROLE:
        	if (received->data.len == 31){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addUser_Role_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
            		data_add_user_t *data = (data_add_user_t*)&(received->data.addUser_Role_c.perm);
            		checkRegister = seg_edit_user(MEM_USER_ROLE, NULL, &received->data.addUser_Role_c.perm_user, data, NULL, &app.app_connectivity.user.puid);
            		if(checkRegister == SEG_OK){
            			pError = PACKET_ERR_OK;
            		}
            		else if(checkRegister == SEG_ERR_CAD){
            			pError = PACKET_ERR_FAIL;
            		}
            		sendBack.data.len = 1;
        		}
        	}
        	break;
        case ALARMCMD_SET_USER_CONTROL:
        	if (received->data.len == 22){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addUser_Role_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
        			app.app_control.User_control = received->data.addUser_Role_c.perm_user;

            		app.app_control.init_register = true;
            		sendBack.properties = PROP_ASYNC;
            		sendLR.cmd = LRCMD_FORM_NETWORK;
            		//sendLR.data[0] = 1;
            		sendLR.timestamp = 0;
            		sendLR.data.raw[0] = 0;
            		sendLR.data.raw[1] = 255;
            		sendLR.data.raw[2] = 0; //cadastro de controle
            		sendLR.data.len = 3;

            		app.packet.lastCmd = ALARMCMD_SET_USER_CONTROL;
            		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

            		if(app.Wireless_mode == 1){
            			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
            		}

            		sendBack.data.len = 0;
            		__Send_Packet_LORA(&sendLR,NULL,1);
        		}
        	}
        	break;
        case LRCMD_JOINED_NETWORK:
    		if(app.Wireless_mode == 1){
    			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
    		}
        	sendBack.data.len = 0;
        	break;
        case LRCMD_JOINED_NETWORK_SENSOR:
    		if(app.Wireless_mode == 1){
    			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
    		}
        	sendBack.data.len = 0;
        	break;
        case LRCMD_JOINED_NETWORK_GATE:
    		if(app.Wireless_mode == 1){
    			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
    		}
        	sendBack.data.len = 0;
        	break;
        case ALARMCMD_ARM:
            if (received->data.len == 9) {
            	checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.alarmArm_c.perm_user, received->timestamp, name, &app.app_connectivity.user.puid);
            	checkRegister = SEG_OK;
            	if(checkRegister <= SEG_OK){
        			uint8_t indice_partition_app;
            		seg_data_pgm_t pgm_partition;
            		pgm_cmd_t pgm_cmd;
            		//resource_type_t type_resource;

            		//type_resource = (resource_type_t)received->data.alarmArm_c.type_resource;

            		checkRegister = seg_control_val_acesso(MEM_USER_VIRTUAL,name,(uint8_t*)&received->data.alarmArm_c.perm_user, 0,NULL,NULL);


            		if(received->data.alarmArm_c.type_resource == RESOURCE_PARTITION){
            			indice_partition_app = _get_particao(received->data.alarmArm_c.resource);

            			if(app.particoes_estado[indice_partition_app].id_particao != 0){
            				estado_particao_t *estado = &app.particoes_estado[indice_partition_app].estado;

            				//if((resource_user & received->data.alarmArm_c.resource) == received->data.alarmArm_c.resource){
            				estado->PartitionArmed = received->data.alarmArm_c.arm_desarm;
            				//}

            				if(estado->alarme_disparado == true && estado->PartitionArmed == 0 && estado->temporizador_ativo == true){
            					estado->alarme_disparado = false;
            					app.status_sirene = false;
            					clear_violation_partition(app.particoes_estado[indice_partition_app].id_particao);


            					if(estado->temporizador_ativo == true){
            						_clear_setorTemporario_notification(app.particoes_estado[indice_partition_app].id_particao);
            					}

            					alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 700, ALARM_MODE_ONESHOT);
            				}

            				if(estado->temporizador_ativo_smart == true && estado->PartitionArmed == 0){
            					estado->temporizador_ativo_smart = false;
        						_clear_setorSmart_notification(app.particoes_estado[indice_partition_app].id_particao);
        					}


            				Type_work = (estado->PartitionArmed == 1) ? TYPE_PARTITION_ARMED : (estado->PartitionArmed == 0) ? TYPE_PARTITION_DISARMED : 0;
            				Method_work = METHOD_USER_APP;

            				seg_data_partition_t partition_notification;
            				seg_ident_valida_partition(app.particoes_estado[indice_partition_app].id_particao,&partition_notification);

            				n_work = nysy_generateStruct( Method_work, Type_work, name, 16, (uint8_t*)partition_notification.nome);

            				memcpy(&app.app_notify[app.count_notify].n_app, &n_work, sizeof(notify_struct_u));
            				app.app_notify[app.count_notify].status_notification = true;
            				app.count_notify = (app.count_notify + 1) % 10;
            				//sendNotification(&app.app_notify[app.count_notify].n_app);
            				//cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
            				alarm_setAlarm_os(&app.app_event, APP_EVENT_NOTIFY, 500, ALARM_MODE_ONESHOT);

            				//tratar PGM
            				if(partition_notification.ID_PGM != 0){
            					pgm_cmd.ID = PGM_ID;
            					seg_ident_valida_pgm(partition_notification.ID_PGM, &pgm_partition);
            					if((pgm_partition.type_association == PGM_DISARM_PARTITION && Type_work == TYPE_PARTITION_DISARMED) || (pgm_partition.type_association == PGM_ARM_PARTITION && Type_work == TYPE_PARTITION_ARMED) || pgm_partition.type_association == PGM_ARM_DISARM_PARTITION){
            						pgm_cmd.function = PGM_PULSED;
            						pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
            						pgm_cmd.rele_number = pgm_partition.index_rele;
            						pgm_cmd.timebase.state = true;
            						pgm_cmd.timebase.time = pgm_partition.time*1000;

            						pgm_start(&pgm_cmd);

            					}
            				}
            			}
            		}
            		else if(received->data.alarmArm_c.type_resource == RESOURCE_PGM){
            			seg_ident_valida_pgm(received->data.alarmArm_c.resource, &pgm_partition);
            			pgm_cmd.ID = PGM_ID;
            			pgm_cmd.function = received->data.alarmArm_c.mode;
            			pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
            			pgm_cmd.rele_number = pgm_partition.index_rele;
            			pgm_cmd.timebase.state = received->data.alarmArm_c.arm_desarm;
            			pgm_cmd.timebase.time = pgm_partition.time*1000;
            			pgm_start(&pgm_cmd);

            		}



            		pError = PACKET_ERR_OK;



            	}
            	else{
            		pError = PACKET_ERR_FAIL;
            	}
            	sendBack.data.len = 1;
            }
            break;
        case ALARMCMD_ADD_OWNER:
        	if (received->data.len == 36){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addUser_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
        			seg_owner_cad(received->data.addUser_c.nome, received->data.addUser_c.id,&perm);

        			pError = PACKET_ERR_OK;
        			memcpy(&(sendBack.data.raw[1]), &perm, 4);
        			memcpy(&(sendBack.data.raw[5]), &seg.ble_owner.senha, 8);
        			sendBack.data.len = 13;
        		}
        	}
        	break;
        case ALARMCMD_SETUP_SETOR:
        	checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.SetSetor_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        	if (received->data.len == 32 && checkRegister <= SEG_OK_OWNER){ //se for setor flexivel

        		data_add_setor_t *data = (data_add_setor_t*)&(received->data.SetSetor_c.perm);

        		if(received->data.SetSetor_c.type_setor == 0){//se for setor com fio
            		checkRegister = seg_cad_setor_from_cmd(data);

            		if(checkRegister == SEG_OK){
            			pError = PACKET_ERR_OK;
            			seg_ble_commit(BLE_COMMIT_SETOR,NULL);
            		}
        			else if (checkRegister == SEG_POS_OCUPADA){
        				pError = PACKET_ERR_MEM_FULL;
        			}
                    else{
                        pError = PACKET_ERR_UNAUTHORIZED;
                    }

            		//verificação
            		seg_data_setor_t testeSetor;
            		checkRegister = seg_ident_valida_setor(received->data.SetSetor_c.ID_Setor,&testeSetor);
        		}
        		else if(received->data.SetSetor_c.type_setor == 1){//se for setor sem fio
            		checkRegister = seg_cad_setor_from_cmd(data);

            		if(checkRegister == SEG_OK){
            			pError = PACKET_ERR_OK;
            			seg_ble_commit(BLE_COMMIT_SETOR,NULL);
            		}
        			else if (checkRegister == SEG_POS_OCUPADA){
        				pError = PACKET_ERR_MEM_FULL;
        			}
                    else{
                        pError = PACKET_ERR_UNAUTHORIZED;
                    }
        		}



        	}
        	else{
        		pError = PACKET_ERR_UNAUTHORIZED;
        	}
        	sendBack.data.len = 1;
        	break;
        case ALARMCMD_SETUP_SETOR_GENERAL:
        	if(received->data.len == 6){
            	setup_t setup_general_setor;
            	setup_general_setor.Model_resistor = received->data.setSetor_gereral_c.model_resistor;
            	setup_general_setor.Type_Zona = received->data.setSetor_gereral_c.setup_resistor;
            	Set_Configuration_setor(&setup_general_setor);
            	pError = PACKET_ERR_OK;
            	sendBack.data.len = 1;
        	}

        	break;

        case ALARMCMD_SETUP_PARTITION:
        	checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.SetPartition_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        	if (received->data.len == 41 && checkRegister <= SEG_OK_OWNER){ //se for setor flexivel
        		data_add_partition_t *data = (data_add_partition_t*)&(received->data.SetPartition_c.perm);

        		checkRegister = seg_cad_partition_from_cmd(data);

        		if(checkRegister == SEG_OK){
        			uint8_t indice_partition;
        			pError = PACKET_ERR_OK;
        			estado_particao_t *estado = buscar_estado_particao(received->data.SetPartition_c.ID_partition, &indice_partition);
        			if (!estado) {
        				registrar_particao(received->data.SetPartition_c.ID_partition, &indice_partition);
        			}

        			app.particoes_estado[indice_partition].estado.number_to_anular = received->data.SetPartition_c.numero_auto_anular;
        			app.particoes_estado[indice_partition].estado.tempo_smart = received->data.SetPartition_c.tempo_setor_inteligente;


        			sendBack.data.len = 2;
        			sendBack.data.raw[1] = indice_partition;

        		}
    			else if (checkRegister == SEG_POS_OCUPADA){
    				pError = PACKET_ERR_MEM_FULL;
    				sendBack.data.len = 1;
    			}
                else{
                    pError = PACKET_ERR_UNAUTHORIZED;
                    sendBack.data.len = 1;
                }

        		//verificação
        		seg_data_partition_t testePartition;
        		checkRegister = seg_ident_valida_partition((received->data.SetPartition_c.ID_partition),&testePartition);
        	}
        	else{
        		pError = PACKET_ERR_UNAUTHORIZED;
        		sendBack.data.len = 1;
        	}

        	break;
        case ALARMCMD_ADD_SENSOR:
        	if (received->data.len == 8){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.AddSensor_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
        			app.app_sensor.init_register = true;
        			app.app_sensor.ID_Setor = received->data.AddSensor_c.ID_Setor;
        			app.app_sensor.config_sensor = 0;

            		sendBack.properties = PROP_ASYNC;
            		sendLR.cmd = LRCMD_FORM_NETWORK;
            		//sendLR.data[0] = 1;
            		sendLR.timestamp = 0;
            		sendLR.data.raw[0] = 0;
            		sendLR.data.raw[1] = 255;
            		sendLR.data.raw[2] = 1; //cadastro de sensor
            		sendLR.data.len = 3;

            		app.packet.lastCmd = ALARMCMD_ADD_SENSOR;
            		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

            		if(app.Wireless_mode == 1){
            			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
            		}

            		sendBack.data.len = 0;
            		__Send_Packet_LORA(&sendLR,NULL,1);
        		}
        	}
        	break;
        case ALARMCMD_ADD_MODULE_GATE:
        	if (received->data.len == 8){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.AddGate_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
        			app.app_gate.init_register = true;
        			app.app_gate.ID_automation = received->data.AddGate_c.ID_automation;

               		sendBack.properties = PROP_ASYNC;
                	sendLR.cmd = LRCMD_FORM_NETWORK;

            		sendLR.timestamp = 0;
            		sendLR.data.raw[0] = 0;
            		sendLR.data.raw[1] = 255;
            		sendLR.data.raw[2] = 2; //cadastro de modulo gate
            		sendLR.data.len = 3;

            		app.packet.lastCmd = ALARMCMD_ADD_MODULE_GATE;
            		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

            		if(app.Wireless_mode == 1){
            			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
            		}

            		sendBack.data.len = 0;
            		__Send_Packet_LORA(&sendLR,NULL,1);
        		}
        	}
        	break;
        case LRCMD_WRITE_CONTROL_GATE:
        	if (received->data.len == 6){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.writeGate_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK){
        			checkRegister = seg_ident_valida_gate(received->data.writeGate_c.ID_gate,NULL);
        			if(checkRegister == SEG_OK){
        				pError = PACKET_ERR_OK;

            			packet_void_t sendGate;

                		sendBack.properties = PROP_ASYNC;
                		sendGate.cmd = LRCMD_WRITE_CONTROL_GATE;
                		//sendLR.data[0] = 1;
                		sendGate.timestamp = 0;
                		sendGate.data.raw[0] = received->data.writeGate_c.ID_gate;
                		sendGate.data.raw[1] = received->data.writeGate_c.command;
                		sendGate.data.len = 2;

                		app.packet.lastCmd = ALARMCMD_ADD_MODULE_GATE;
                		app.packet.lastCmd_ASYNC = app.packet.lastCmd;


                		__Send_Packet_LORA(&sendGate,NULL,1);
        			}
        			else{
        				pError = PACKET_ERR_FAIL;
        			}
        		}
        		else{
        			pError = PACKET_ERR_FAIL;
        		}

        		sendBack.data.len = 1;

        	}


        	break;
        case LRCMD_READ_STATUS_GATE:
        	if (received->data.len == 1){
        		if(received->data.startUser_Control_c.status == 0){
        			sendBack.data.len = 0;
        		}

        	}
        	break;

        case ALARMCMD_FIRMWARE_UPDATE:
        	sendBack.data.len = 1;
        	if (received->data.len == 12){
        		FirmwareUpdate_startWrite(0, received->data.fotaStep1_c.size, received->data.fotaStep1_c.crc32);
        		pError = PACKET_ERR_OK;
        		sendBack.data.len = 0;

           		//pError = PACKET_ERR_OK;//isso não vai no firmware, é apenas para teste
                //sendBack.data.len = 1;
        	}
        	else if (received->data.len >= 32 && received->data.len <= (MAX_BLOCK_SIZE+4)){
                fota_state_e e;
                e = FirmwareUpdate_nextWrite(received->data.fotaStep2_c.block, received->data.fotaStep2_c.step, received->data.len-4);
                e = FOTA_WRITING;

                if (e == FOTA_WRITING){
                    pError = PACKET_ERR_OK;
                    sendBack.data.len = 0; // isso vai no firmware

//            		pError = PACKET_ERR_OK;//isso não vai no firmware, é apenas para teste
//            		sendBack.data.len = 1;
                }
                else{
                    pError = PACKET_ERR_FAIL;

                    //sendBack.properties = PROP_DISCONNECT;
                }
        	}
            else {
                pError = PACKET_ERR_UNKNOWN;
                //log_send("Firmware Update Error Unknown", LOG_ERROR);
                //sendBack.properties = PROP_DISCONNECT;
            }
        	break;
        case ALARMCMD_SETUP_NOTIFY:
    		bool status_notification;
        	if (received->data.len == 5 && received->data.setNotify_c.status == 0){
        		_update_count_notification(received->data.setNotify_c.ID_Notify);

        		status_notification = _get_count_notification(NULL);

        		if(status_notification == true){
        			//cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
        			alarm_setAlarm_os(&app.app_event, APP_EVENT_NOTIFY, 100, ALARM_MODE_ONESHOT);
        		}
        	}

        	alarm_cancelAlarm(APP_EVENT_NOTIFYTIMEOUT);
        	sendBack.data.len = 0;

        	break;

        case ALARMCMD_ADD_PGM:
			sendBack.properties = PROP_ASYNC;
    		app.packet.lastCmd = ALARMCMD_ADD_PGM;
    		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

    		if(app.Wireless_mode == 1){
    			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
    		}

    		pgm_cmd_t pgm_cmd;
    	    pgm_cmd.ID = PGM_ID;
    	    pgm_cmd.function = PGM_REGISTER;

    	    pgm_start(&pgm_cmd);
    	    sendBack.data.len = 0;
        	break;

        case ALARMCMD_SET_PGM:
        	uint8_t end_pgm;
        	if (received->data.len == 11){
    			data_add_pgm_t *data_pgm = (data_add_pgm_t*)&(received->data.setPgm_c.ID_PGM);

    			end_pgm = received->data.setPgm_c.ID_PGM + received->data.setPgm_c.index_rele;

				seg_edit_pgm(&end_pgm, data_pgm);


        		pError = PACKET_ERR_OK;
        		sendBack.data.len = 1;
        	}
        	break;

        case ALARMCMD_ADD_TECLADO:
			sendBack.properties = PROP_ASYNC;
    		app.packet.lastCmd = ALARMCMD_ADD_TECLADO;
    		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

    		if(app.Wireless_mode == 1){
    			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
    		}
    		teclado_start_register();
    	    sendBack.data.len = 0;
        	break;

        case ALARMCMD_SET_INST_TECLADO:
        	uint8_t status;
        	if (received->data.len == 1){
        		status = received->data.startUser_Control_c.status;

        	}
        	break;

        case ALARMCMD_GET_USER_TECLADO:
        	if (received->data.len == 6){
        		checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.get_userTeclado_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);
        		if(checkRegister <= SEG_OK_OWNER){
        			if(received->data.get_userTeclado_c.type_get == 0){//pacote de requisição
        				//app.app_teclado.number_user = 5;
        				pError = PACKET_ERR_OK;
            			sendBack.data.raw[1] = app.app_teclado.number_user;
            			sendBack.data.len = 2;
            			if(app.app_teclado.number_user > 0){
            				alarm_setAlarm_os(&app.app_event, APP_EVENT_SYNC_TECLADO, 100, ALARM_MODE_ONESHOT);
            			}
        			}
        			else{//pacote de ack
        				if(received->data.get_userTeclado_c.status == 0){
        					//limpar flag de usuário de teclado
        					clean_sync_user_teclado(app.app_teclado.sync_teclado_cloud.count); //limpar tudo
        					app.app_teclado.number_user = 0;
        					//alarm_setAlarm_os(&app.app_event, APP_EVENT_SYNC_TECLADO, 100, ALARM_MODE_ONESHOT);
        				}
        				else{

        				}
        			}

        		}
        	}
        	break;

        case ALARMCMD_ADD_CENARIO:
        	checkRegister = seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.addCenario_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid);

        	if (received->data.len == 9 && checkRegister <= SEG_OK_OWNER){
        		app.cenario_register.ID_cenario = received->data.initCenario_c.ID_cenario;
        		app.cenario_register.qtd_acoes = received->data.initCenario_c.number_action;
        		sendBack.data.len = 0;
        	}

        	if (received->data.len == 17 && checkRegister <= SEG_OK_OWNER && received->data.addCenario_c.ID_cenario == app.cenario_register.ID_cenario){
        		if(received->data.addCenario_c.index_action == 0){
        			app.cenario_register.trigger_type = received->data.addCenario_c.type;
        			app.cenario_register.trigger_ID_trigger_action = received->data.addCenario_c.ID_trigger_action;
        			app.cenario_register.trigger_mode = received->data.addCenario_c.mode;
        			app.cenario_register.trigger_event_command = received->data.addCenario_c.event_command;
        			app.cenario_register.trigger_timeout = received->data.addCenario_c.timeout;
        		}
        		else{
        			uint8_t index = received->data.addCenario_c.index_action - 1;
        			app.cenario_register.acoes[index].type = received->data.addCenario_c.type;
        			app.cenario_register.acoes[index].ID_trigger_action = received->data.addCenario_c.ID_trigger_action;
        			app.cenario_register.acoes[index].mode = received->data.addCenario_c.mode;
        			app.cenario_register.acoes[index].event_command = received->data.addCenario_c.event_command;
        			app.cenario_register.acoes[index].timeout = received->data.addCenario_c.timeout;

        			if(received->data.addCenario_c.index_action == app.cenario_register.qtd_acoes){
        				seg_cad_cenario_from_cmd(&app.cenario_register);
        			}
        		}

        	}
        	break;

        case ALARMCMD_COMMIT:
        	switch (app.packet.lastCmd){
        	case ALARMCMD_ADD_USER:
        		pError = PACKET_ERR_OK;
        		sendBack.data.len = 1;
        		break;
        	case ALARMCMD_SET_USER_TECL:
        		checkRegister = seg_ble_commit(BLE_COMMIT_EDIT, NULL);
                if (checkRegister == SEG_OK){
                    pError = PACKET_ERR_OK;
                    sendBack.data.len = 1;
                }
                else{
                    pError = PACKET_ERR_FAIL;
                }
        		break;
        	case ALARMCMD_SET_USER_ROLE:
        		checkRegister = seg_ble_commit(BLE_COMMIT_EDIT, NULL);
                if (checkRegister == SEG_OK){
                    pError = PACKET_ERR_OK;
                    sendBack.data.len = 1;
                }
                else{
                    pError = PACKET_ERR_FAIL;
                }
        		break;

        	case ALARMCMD_ADD_OWNER:
        		checkRegister = seg_ble_commit(BLE_COMMIT_OWNER, NULL);
                if (checkRegister == SEG_OK){
                    pError = PACKET_ERR_OK;
                    sendBack.data.len = 1;
                }
                else{
                    pError = PACKET_ERR_FAIL;
                }
        		break;
        	case ALARMCMD_SET_USER_CONTROL:
        		checkRegister = seg_ble_commit(BLE_COMMIT_EDIT, NULL);
                if (checkRegister == SEG_OK){
                    pError = PACKET_ERR_OK;
                    sendBack.data.len = 1;
                }
                else{
                    pError = PACKET_ERR_FAIL;
                }
        		break;
        	case LRCMD_JOINED_NETWORK:
                pError = PACKET_ERR_OK;
                sendBack.data.len = 1;
        		break;
        	case LRCMD_JOINED_NETWORK_SENSOR:
                pError = PACKET_ERR_OK;
                sendBack.data.len = 1;
                //seg_ble_commit(BLE_COMMIT_SETOR,NULL);
        		break;
        	case ALARMCMD_ADD_PGM:
        		pError = PACKET_ERR_OK;
        		sendBack.data.len = 1;
        		break;
        	}

        	break;
        }

    }

    app.packet.lastCmd = received->cmd;

	sendBack.data.raw[0] = pError;
	if (sendBack.data.len > 0){
		send_response = true;
		__Send_Packet_Wireless(&sendBack,app.Wireless_mode,app.aes,1,app.app_connectivity.packet.id_job);
	}

    if(sendBack.properties == PROP_ASYNC){
    	app.packet.lastCmd_ASYNC = app.packet.lastCmd;
    }
}

void __conn_settings_work(){
	packet_void_t *received,sendBack;
	packet_error_t pError;
	seg_status_e checkRegister;
	pdl_timestamp_check_e tsCheck = PDL_TIMESTAMP_CHECK_OK;

	uint32_t perm;
	seg_status_e checkSeg;

    sendBack.data.len = 0;
    sendBack.timestamp = 0;

    app.app_connectivity.user.puid = app.app_connectivity.packet.id;

    if(packet_decrypt_and_get(&app.app_connectivity.packet, app.aes, &app.app_connectivity.blePacket) == PACKET_FAIL_DECRYPT){
    	pError = PACKET_ERR_AES_KEY;
    }

    received = &app.app_connectivity.blePacket;
    //alarm_setAlarm_os(app.event.handle, APP_EVENT_TIMEOUT, PDL_DC_TIMEOUT, ALARM_MODE_ONESHOT);

    memset(&sendBack, 0, sizeof(packet_void_t));
    pError = PACKET_ERR_UNKNOWN;
    sendBack.cmd = received->cmd;
    sendBack.data.len = 1;
    switch (received->cmd) {

    case ALARMCMD_SET_INST:
    	if (received->data.len == 16){
    		checkSeg = seg_ble_root_cad(received->data.setProgram_c.uuid, NULL, &perm);
    		if (checkSeg == SEG_OK){
    			pError = PACKET_ERR_OK;
    			memcpy(&(sendBack.data.raw[1]), &perm, 4);
    			memcpy(&(sendBack.data.raw[5]), &seg.ble_root.senha, 8);
    			sendBack.data.len = 13;
    		}
    		else{
    			pError = PACKET_ERR_FAIL;
    		}

    	}
    	else{
    		pError = PACKET_ERR_FAIL;
    	}
    	break;
    case ALARMCMD_COMMIT:
    	if (received->data.len == 4 && seg_ident_valida(MEM_CLASS_REMOTE, MEM_USER_VIRTUAL, received->data.Commit_c.perm, received->timestamp, NULL, &app.app_connectivity.user.puid) == SEG_OK_ROOT){
    		if (app.packet.lastCmd == ALARMCMD_SET_INST){
    			seg_ble_commit(BLE_COMMIT_INST, NULL);
    			pError = PACKET_ERR_OK;
    			__alarm_set_settings(1);
    		}
    		else{
    			pError = PACKET_ERR_UNAUTHORIZED;
    		}
    	}
    	else{
    		pError = PACKET_ERR_UNAUTHORIZED;
    	}
    	sendBack.data.len = 1;
    	break;
    }

    app.packet.lastCmd = received->cmd;

	sendBack.data.raw[0] = pError;
	if (sendBack.data.len > 0){
		send_response = true;
		__Send_Packet_Wireless(&sendBack,app.Wireless_mode,app.aes,1,app.app_connectivity.packet.id_job);
	}
}

void __fota_work(){
    packet_void_t sendBack;
    fota_pending_e fPend;
    uint8_t reset = 0;

    memset(&sendBack, 0, sizeof(packet_void_t));
    fPend = FirmwareUpdate_getState();
    sendBack.timestamp = 0;
    sendBack.cmd = ALARMCMD_FIRMWARE_UPDATE;
    switch (fPend){
    case FOTA_PEND_RESPONSE_DONE:
        //hNvs_Write(NVS_FOTA_INFO, &pdl.system.fotaRegion.raw, 1);
        //sendBack.properties = PROP_DISCONNECT;
        reset = 1;
        /* no break */
    case FOTA_PEND_RESPONSE_OK:
        sendBack.data.raw[0] = PACKET_ERR_OK;
        sendBack.data.len = 1;
        break;
    case FOTA_PEND_RESPONSE_FAIL:
        sendBack.data.raw[0] = PACKET_ERR_FAIL;
        sendBack.data.len = 1;
        //sendBack.properties = PROP_DISCONNECT;
        break;
    default:
        // do nothing
        break;
    }

	if (sendBack.data.len > 0){
		send_response = true;
		__Send_Packet_Wireless(&sendBack,app.Wireless_mode,app.aes,1,app.app_connectivity.packet.id_job);
	}

	if (reset){
		cy_rtos_delay_milliseconds(150);
		//System_Reset(); adicionar um reset
	}

//    __ble_send_response(&sendBack, pdl.BLE.user.puid, pdl.BLE.user.aes, pdl.BLE.Packet.version);
//    if (reset){
//        hTask_sleep(150);
//        System_Reset();
//    }
}



void __pgmResponse_Work() {
	get_pgm_info(app.app_pgm.app_pgm_registrado, &app.app_pgm.number_PGM_register);

	for(uint8_t i = 0; i < app.app_pgm.number_PGM_register; i++){
		for(uint8_t j = 0; j < 4; j++){
			data_add_pgm_t pgm_register;

    		pgm_register.ID_PGM = app.app_pgm.app_pgm_registrado[i].crc + j;
    		pgm_register.index_rele = j;
    		pgm_register.time = 0;
    		pgm_register.type_association = 0;
    		pgm_register.type_trigger = 0;
    		seg_cad_pgm_from_cmd(&pgm_register);
		}
	}

	app.packet.lastCmd_ASYNC = ALARMCMD_ADD_PGM;
	__async_wireless_response(pgm_register);
}


void opPDL_init(){
	app.event.mask = 0x00FFFFFF;
	app.status_rtos_app = cy_rtos_thread_create(&app.task_app, app_task,APP_TASK_NAME,NULL,4500,APP_TASK_PRIORITY,NULL);
	app.status_rtos_app = cy_rtos_event_init(&app.app_event);
	commum_bus_init();
	pgm_init(&app.app_event,APP_EVENT_PGMRESPONSE);
	power_init(&app.app_event, APP_EVENT_POWER);
    teclado_init(&app.app_event,APP_EVENT_TECLADO,app.aes);
	alarm_init(200);
    /* Initialize WDT */
    //initialize_wdt();
	connectivity_init(APP_EVENT_CONNEC);
	FirmwareUpdate_init(&app.app_event,APP_EVENT_FOTA);
	zonas_init(&app.app_event,APP_EVENT_ZONAS);
	LTE_init(&app.app_event,APP_EVENT_LTE);
	app.particoes_estado[0].id_particao = 0;
	initNotificationSystem(&app.app_event,APP_EVENT_NOTIFY);

	LongRange_init(&app.app_event,APP_EVENT_LR);

//	rtc_init();

	seg.buff_senha[0] = 1;
	seg.buff_senha[1] = 2;
	seg.buff_senha[2] = 3;
	seg.buff_senha[3] = 4;
	seg.buff_senha[4] = 5;
	seg.buff_senha[5] = 6;
	seg.buff_senha[6] = 7;
	seg.buff_senha[7] = 8;
	seg.i = 8;

	seg.f_nova_senha = 0;


	seg.ble_root.senha = 0xFFFFFFFF12345678;

	//seg.ble_root.perm = 0xCDA71FC2;
	seg.ble_root.perm = 0xC21FA7CD;
	__alarm_set_settings(1);

	at25sf_init(0,(uint32_t)AT25_CS_PORT,AT25_CS_PIN);

	//EraseSector();
//
//	readMemory = at25sf_read_id();
//
//	write_memory[0] = 0xAA;
//	write_memory[9] = 0xBB;
//	write_memory[122] = 0xCC;
//	write_memory[127] = 0x11;
//
////	//at25sf_chip_erase();
//
//	at25sf_block_erase(0,2);
//	//at25sf_read(0,read_memory,10);
	//at25sf_write(0,write_memory,128);
//	//at25sf_write(0x100,write_memory,124);
//
//	ProgramPage(256,write_memory,128);
	//at25sf_read(0,read_memory,128);
//	ProgramPage(256,write_memory,128);
//	fota_metadata_u testemeta;
//	S25FL_ReadPage(0,testemeta.raw,128);
//	uint8_t i = 0;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;
//	S25FL_ReadPage(256+i*256,read_memory,128);
//	i++;


	memcpy(app.aes, (uint8_t[]){0xD7, 0xE0, 0x3F, 0xFB, 0xC4, 0x20, 0x1B, 0x37,
	                            0x30, 0xB0, 0x21, 0xFA, 0x76, 0xBE, 0xBA, 0x0E},
	       sizeof(app.aes));


	//MQTT_CONFIG(config);
	//alarm_setAlarm_os(&app.app_event, APP_EVENT_LTE, 10000, ALARM_MODE_ONESHOT);
	//alarm_setAlarm_os(&app.app_event, APP_EVENT_TIMEOUT, 1000, ALARM_MODE_CONTINUOUS);
	eeprom_init();

	seg_data_t load[1] = {0};
	int16_t i;
	uint16_t start_end;

	eeprom_erase();

	Cy_GPIO_Write(LED1_PORT, LED1_PIN,0);


//	uint8_t KEY_TEST1[16] = {
//	    0x90, 0x74, 0xDE, 0x69,
//	    0x89, 0x1F, 0xF5, 0x0B,
//	    0x81, 0x46, 0x44, 0xD2,
//	    0x68, 0x2A, 0x9B, 0x3E
//	};
//
//	uint8_t dadoCru_crip[16]={
//	    0x22, 0x41, 0x01, 0x69,
//	    0x89, 0x1F, 0xF5, 0x0B,
//	    0x81, 0x46, 0x44, 0xD2,
//	    0x68, 0x2A, 0x9B, 0x5F
//	};
//	uint8_t dado_crip[16];
//	uint8_t verify_decry[16];
//
//	memset(dado_crip,0,16);
//
//	Cy_Cryptolite_Aes_Init(CRYPTOLITE, KEY_TEST1, &aes_state, &aesBuffers);
//	Cy_Cryptolite_Aes_Ecb(CRYPTOLITE,dado_crip,dadoCru_crip,&aes_state);
//    /* ... check for errors... */
//    Cy_Cryptolite_Aes_Free(CRYPTOLITE, &aes_state);
//
//    //memset(dado_crip,0,16);
//
//    mbedtls_aes_init(&ecb_context);
//    //mbedtls_aes_setkey_enc(&ecb_context, KEY_TEST1, 128);
//    //mbedtls_aes_crypt_ecb(&ecb_context, MBEDTLS_AES_ENCRYPT, dadoCru_crip, dado_crip);
//    mbedtls_aes_setkey_dec(&ecb_context, KEY_TEST1, 128);
//    mbedtls_aes_crypt_ecb(&ecb_context, MBEDTLS_AES_DECRYPT, dado_crip, verify_decry);
}



/*******************************************************************************
 * Function Name: connectivity_task
 ********************************************************************************
 * Summary:
 *  This RTOS task toggles the User LED each time the semaphore is obtained.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  The RTOS task never returns.
 *
 *******************************************************************************/
void app_task(void *pvParameters)
{
    notify_struct_u n_send;

    uint8_t name[16];
    mqtt_config_t config = {
        .broker_address = "mqtt.ppaoncloud.com:1883",
        .username = "ppaAlarmHdw",
        .password = "tHWEP3RbZVw97deY"
    };

    packet_void_t sendNotify;

    packet_void_t sendBack;
	seg_data_pgm_t pgm_partition;
	pgm_cmd_t pgm_cmd;

    (void) pvParameters;
    for(;;)
    {
    	app.status_rtos_app = cy_rtos_event_waitbits(&app.app_event,&app.event.mask,false,false,portMAX_DELAY);
        /* Reset WDT */
        //Cy_WDT_ClearWatchdog();

    	if (app.event.mask & APP_EVENT_CONNEC){
    		if (__alarm_get_settings() == 0){
    			__conn_settings_work();
    		}
    		else if(__alarm_get_settings() == 1){
    			__conn_work();
    		}
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_CONNEC);
    	}
    	else if(app.event.mask & APP_EVENT_LONGRANGE){
    		__longRange_work();
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_LONGRANGE);
    	}
    	else if(app.event.mask & APP_EVENT_TIMEOUT){
    		//packet_transmit_longRange(&test_TX);
            sendBack.data.raw[0] = PACKET_ERR_OK;
            sendBack.data.len = 1;
    		send_response = true;
    		sendBack.cmd = ALARMCMD_ARM;
    		sendBack.timestamp = 0;
    		__Send_Packet_Wireless(&sendBack,app.Wireless_mode,app.aes,1,app.app_connectivity.packet.id_job);
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_TIMEOUT);
    	}
    	else if(app.event.mask & APP_EVENT_ZONAS){
    		__work_setor_wire();
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_ZONAS);
    	}
    	else if(app.event.mask & APP_EVENT_SETORTIMEOUT){
    		bool ActiveSetorTimeout = false;

    		for (int i = 0; i < 8; i++) {
    			if (app.particoes_estado[i].id_particao != 0) {
        			estado_particao_t *estado = &app.particoes_estado[i].estado;
        			if (estado->temporizador_ativo && !estado->alarme_disparado) {
        				estado->time_atual++;
        				ActiveSetorTimeout = true;
        				if(estado->time_atual == estado->tempo_entrada){
        					//cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
        					estado->alarme_disparado = true;
        					//estado->sirene_disparado = true;

        					for(uint8_t j = 0; j<10;j++){
        						if(estado->setores_violados[j].status_temporizada == true){
        							estado->setores_violados[j].status_temporizada = false;
        							if(app.particoes_estado[i].estado.setores_violados[j].ID_PGM != 0){
            			        	    pgm_cmd.ID = PGM_ID;
            					    	seg_ident_valida_pgm(app.particoes_estado[i].estado.setores_violados[j].ID_PGM, &pgm_partition);
            					    	if(pgm_partition.type_association == SETOR_VIOLATION){
            				        	    pgm_cmd.function = pgm_partition.mode;
            				        	    pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
            				        	    pgm_cmd.rele_number = pgm_partition.index_rele;
            				        	    pgm_cmd.timebase.state = true;
            				        	    pgm_cmd.timebase.time = pgm_partition.time*1000;

            				        	    pgm_start(&pgm_cmd);
            					    	}
        							}
                					if(app.particoes_estado[i].estado.setores_violados[j].status_autoanulavel == true){
                						increment_autoAnulavel(app.particoes_estado[i].id_particao,app.particoes_estado[i].estado.setores_violados[j].id_setor);
                					}
        						}
        					}


        					app.status_sirene = true;
        					alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 5, ALARM_MODE_ONESHOT);
        					alarm_setAlarm_os(&app.app_event, APP_EVENT_NOTIFY, 100, ALARM_MODE_ONESHOT);
        					estado->temporizador_ativo = false;

        				}
        				if(estado->PartitionArmed == 0){
        					estado->alarme_disparado = false;
        					estado->temporizador_ativo = false;

        				}
        			}
    			}
    		}
    		if(ActiveSetorTimeout == false){
    			alarm_cancelAlarm(APP_EVENT_SETORTIMEOUT);
    		}
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_SETORTIMEOUT);
    	}
    	else if(app.event.mask & APP_EVENT_SMART_PARTITION_TIMEOUT){
    		bool reestart_timer = false;

    		for(uint8_t i = 0; i < 8; i++){
    			for(uint8_t j = 0; j < 10; j++){
        			if(app.particoes_estado[i].estado.setores_violados[j].status_smart == true){
        				reestart_timer = true;
        				app.particoes_estado[i].estado.count_smart_timeout++;
        				//if(app.particoes_estado[i].estado.setores_violados[j].count_smart_timeout == app.particoes_estado[i].estado.tempo_entrada){
        				if(app.particoes_estado[i].estado.count_smart_timeout == app.particoes_estado[i].estado.tempo_smart){
        					if(app.particoes_estado[i].estado.setores_violados[j].status_smart_timeout == true){
        						cy_rtos_event_setbits(&app.app_event,APP_EVENT_NOTIFY);
        						app.particoes_estado[i].estado.setores_violados[j].status_smart_timeout = false;
        						app.particoes_estado[i].estado.alarme_disparado = true;

        						if(app.status_sirene == true){
        							alarm_setAlarm_os(&app.app_event, APP_EVENT_SIRENE, 100, ALARM_MODE_ONESHOT);
        						}

        						if(app.particoes_estado[i].estado.setores_violados[j].ID_PGM != 0){
        			        	    pgm_cmd.ID = PGM_ID;
        					    	seg_ident_valida_pgm(app.particoes_estado[i].estado.setores_violados[j].ID_PGM, &pgm_partition);
        					    	if(pgm_partition.type_association == SETOR_VIOLATION){
        				        	    pgm_cmd.function = pgm_partition.mode;
        				        	    pgm_cmd.address = pgm_partition.ID_PGM - pgm_partition.index_rele;
        				        	    pgm_cmd.rele_number = pgm_partition.index_rele;
        				        	    pgm_cmd.timebase.state = true;
        				        	    pgm_cmd.timebase.time = pgm_partition.time*1000;

        				        	    pgm_start(&pgm_cmd);
        					    	}
        						}
            					if(app.particoes_estado[i].estado.setores_violados[j].status_autoanulavel == true){
            						increment_autoAnulavel(app.particoes_estado[i].id_particao,app.particoes_estado[i].estado.setores_violados[j].id_setor);
            					}
        					}

        					else{
        						_clear_setorSmart_notification(app.particoes_estado[i].estado.setores_violados[j].id_partition);
        						app.particoes_estado[i].estado.setores_violados[j].Status_Violado = false;
        					}

        					clear_smart_partition(app.particoes_estado[i].estado.setores_violados[j].id_partition);
        					app.particoes_estado[i].estado.temporizador_ativo_smart = false;


        				}
        				if(app.particoes_estado[i].estado.PartitionArmed == 0){
        					app.particoes_estado[i].estado.setores_violados[j].Status_Violado = false;
        					clear_smart_partition(app.particoes_estado[i].estado.setores_violados[j].id_partition);
        				}
        				//app.particoes_estado.estado.setores_violados[i].num_detection = 0;
        			}
    			}
    		}
    		if(reestart_timer == true){
    			alarm_setAlarm_os(&app.app_event, APP_EVENT_SMART_PARTITION_TIMEOUT, 1000, ALARM_MODE_ONESHOT);
    		}
    		else{
    			alarm_cancelAlarm(APP_EVENT_SMART_PARTITION_TIMEOUT);
    		}
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_SMART_PARTITION_TIMEOUT);
    	}
    	else if(app.event.mask & APP_EVENT_NOTIFY){
    		uint8_t count_notify;
    		bool status_notification;
    		status_notification = _get_count_notification(&count_notify);

    		if(status_notification == true){
    			memcpy(&app.environment.notify_send, &app.app_notify[count_notify].n_app, sizeof(notify_struct_u));
    			sendNotify.cmd = ALARMCMD_NOTIFY;
    			sendNotify.timestamp = rtc_get_timestamp();
    			memcpy(&sendNotify.data.raw[0], &app.environment.notify_send, sizeof(notify_struct_u));
    			sendNotify.data.len = 42;
        		app.Wireless_mode = Cloud;
        		if(app.status_MQTT.status_MQTT_LTE == MQTT_CONECTED){
        			app.Remote_Mode = 1;
        		}
        		else{
        			app.Remote_Mode = 0;
        		}

        		__Send_Packet_Wireless(&sendNotify,app.Wireless_mode,app.aes,0,app.app_connectivity.packet.id_job);
        		//alarm_setAlarm_os(&app.app_event, APP_EVENT_NOTIFYTIMEOUT, 5000, ALARM_MODE_ONESHOT);
    		}

    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_NOTIFY);

    	}
    	else if(app.event.mask == APP_EVENT_NOTIFYTIMEOUT){
    		//sendNotification(&app.environment.notify_send);
    	}
    	else if(app.event.mask & APP_EVENT_LTE){
    		__lte_work();
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_LTE);
    	}
    	else if(app.event.mask & APP_EVENT_WIFI_BLE){

			cy_rtos_event_clearbits(&app.app_event, APP_EVENT_WIFI_BLE);
    	}
    	else if(app.event.mask & APP_EVENT_SIRENE){
    		if(app.status_sirene == true){
    			Cy_GPIO_Write(LED2_PORT, LED2_PIN,0);
    		}
    		else{
    			Cy_GPIO_Write(LED2_PORT, LED2_PIN,1);
    		}

    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_SIRENE);

    	}
    	else if(app.event.mask == APP_EVENT_TECLADO){
    		__teclado_work();
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_TECLADO);
    	}

    	else if(app.event.mask == APP_EVENT_FOTA){
    		__fota_work();
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_FOTA);
    	}

    	else if (app.event.mask == APP_EVENT_PGMRESPONSE) {
    		__pgmResponse_Work();
    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_PGMRESPONSE);
    	}

    	else if(app.event.mask == APP_EVENT_SYNC_TECLADO){


//			clean_sync_user_teclado(app.app_teclado.sync_teclado_cloud.count);
//			app.app_teclado.number_user--;
//			alarm_setAlarm_os(&app.app_event, APP_EVENT_SYNC_TECLADO, 100, ALARM_MODE_ONESHOT);




			app.app_teclado.sync_teclado_cloud.type_user = get_sync_user_teclado(app.app_teclado.sync_teclado_cloud.count,&app.app_teclado.sync_teclado_cloud.data_user);
			app.app_teclado.sync_teclado_cloud.count++;

			if(app.app_teclado.sync_teclado_cloud.type_user == USER_TECLADO ){
				app.app_teclado.sync_teclado_cloud.count_teclado++;

				sendBack.data.raw[0] = app.app_teclado.sync_teclado_cloud.count_teclado;
				memcpy(&sendBack.data.raw[1],&app.app_teclado.sync_teclado_cloud.data_user,sizeof(seg_data_t));
				sendBack.data.len = sizeof(seg_data_t)+1;

				alarm_setAlarm_os(&app.app_event, APP_EVENT_SYNC_TECLADO, 100, ALARM_MODE_ONESHOT);
			}
			else if(app.app_teclado.sync_teclado_cloud.type_user == USER_END){
				app.app_teclado.sync_teclado_cloud.count = 0;
				sendBack.data.len = 1;

				if(app.app_teclado.sync_teclado_cloud.count_teclado == app.app_teclado.number_user){
				//if(app.app_teclado.number_user == 0){
					sendBack.data.raw[0] = 0; //enviou todos
				}
				else{
					sendBack.data.raw[0] = 0xFF; //houve algum erro
				}
				app.app_teclado.sync_teclado_cloud.count_teclado = 0;
				//app.app_teclado.number_user = 0;
			}

			sendBack.cmd = ALARMCMD_GET_USER_TECLADO;
			sendBack.timestamp = rtc_get_timestamp();
			app.Wireless_mode = Cloud;
			__Send_Packet_Wireless(&sendBack,app.Wireless_mode,app.aes,0,app.app_connectivity.packet.id_job);

			cy_rtos_event_clearbits(&app.app_event, APP_EVENT_SYNC_TECLADO);

    	}
    	else if(app.event.mask == APP_EVENT_CENARIO){
    		for(uint8_t i = 0; i < 3; i++){
    			uint8_t pointer;
    			if(app.cenario_actual[i].temporizador_ativo == true){
        			pointer = app.cenario_actual[i].pointer_action;
        			app.cenario_actual[i].time_atual++;
        			action_cenario_t tmp;
        			if(app.cenario_actual[i].trigger_on == false){
        				if(app.cenario_actual[i].action_cenario.trigger_timeout == app.cenario_actual[i].time_atual){

        					app.cenario_actual[i].trigger_on = true;

        					tmp.type = app.cenario_actual[i].action_cenario.acoes[pointer].type;
        					tmp.ID_trigger_action = app.cenario_actual[i].action_cenario.acoes[pointer].ID_trigger_action;
        					tmp.mode = app.cenario_actual[i].action_cenario.acoes[pointer].mode;
        					tmp.event_command = app.cenario_actual[i].action_cenario.acoes[pointer].event_command;
        					tmp.timeout = app.cenario_actual[i].action_cenario.acoes[pointer].timeout;


        					__work_cenario(&tmp);

            				app.cenario_actual[i].time_atual = 0;
            				app.cenario_actual[i].pointer_action = pointer;

            				if(pointer == app.cenario_actual[i].action_cenario.qtd_acoes-1){
            					//significa que acabaram as ações
            					app.cenario_actual[i].pointer_action = 0;
            					app.cenario_actual[i].status_trigger = false;
            					app.cenario_actual[i].temporizador_ativo = false;
            					app.cenario_actual[i].trigger_on = false;
            				}
        				}
        			}
        			else{
        				if(app.cenario_actual[i].action_cenario.acoes[pointer].timeout == app.cenario_actual[i].time_atual){
        					pointer++;

        					tmp.type = app.cenario_actual[i].action_cenario.acoes[pointer].type;
        					tmp.ID_trigger_action = app.cenario_actual[i].action_cenario.acoes[pointer].ID_trigger_action;
        					tmp.mode = app.cenario_actual[i].action_cenario.acoes[pointer].mode;
        					tmp.event_command = app.cenario_actual[i].action_cenario.acoes[pointer].event_command;
        					tmp.timeout = app.cenario_actual[i].action_cenario.acoes[pointer].timeout;

        					__work_cenario(&tmp);
        					//pointer++;
            				app.cenario_actual[i].time_atual = 0;
            				app.cenario_actual[i].pointer_action = pointer;
            				if(pointer == app.cenario_actual[i].action_cenario.qtd_acoes-1){
            					//significa que acabaram as ações
            					app.cenario_actual[i].pointer_action = 0;
            					app.cenario_actual[i].status_trigger = false;
            					app.cenario_actual[i].temporizador_ativo = false;
            					app.cenario_actual[i].trigger_on = false;
            				}
        				}
        			}

    			}
    		}

    		if(__get_cenario_temporizadorAtivo() == false){
    			alarm_cancelAlarm(APP_EVENT_CENARIO);
    		}


    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_CENARIO);

    	}
    	else if(app.event.mask == APP_EVENT_BUTTON){
    		packet_void_t received_test_storage;   // memória real
    		packet_void_t *received_test = &received_test_storage;

    		if(status_Button == 0){
    			status_Button = 1;
    			packet_void_t sendLR;
    			app.app_gate.init_register = true;
    			app.app_gate.ID_automation = 0xAABBCCDD;

           		sendBack.properties = PROP_ASYNC;
            	sendLR.cmd = LRCMD_FORM_NETWORK;

        		sendLR.timestamp = 0;
        		sendLR.data.raw[0] = 0;
        		sendLR.data.raw[1] = 255;
        		sendLR.data.raw[2] = 2; //cadastro de modulo gate
        		sendLR.data.len = 3;

        		app.packet.lastCmd = ALARMCMD_ADD_MODULE_GATE;
        		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

        		if(app.Wireless_mode == 1){
        			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
        		}

        		sendBack.data.len = 0;
        		__Send_Packet_LORA(&sendLR,NULL,1);
    		}

    		else if(status_Button == 10){
    			status_Button = 1;
    			packet_void_t sendLR;

    			app.app_sensor.init_register = true;
    			app.app_sensor.ID_Setor = 0;
    			app.app_sensor.config_sensor = 0;

        		sendBack.properties = PROP_ASYNC;
        		sendLR.cmd = LRCMD_FORM_NETWORK;
        		//sendLR.data[0] = 1;
        		sendLR.timestamp = 0;
        		sendLR.data.raw[0] = 0;
        		sendLR.data.raw[1] = 255;
        		sendLR.data.raw[2] = 1; //cadastro de sensor
        		sendLR.data.len = 3;

        		app.packet.lastCmd = ALARMCMD_ADD_SENSOR;
        		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

        		if(app.Wireless_mode == 1){
        			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
        		}

        		sendBack.data.len = 0;
        		__Send_Packet_LORA(&sendLR,NULL,1);
    		}

    		else if(status_Button == 1){
    			status_Button = 2;

    			received_test->data.initCenario_c.ID_cenario = 0x1020;
    			received_test->data.initCenario_c.number_action = 2;

        		app.cenario_register.ID_cenario = received_test->data.initCenario_c.ID_cenario;
        		app.cenario_register.qtd_acoes = received_test->data.initCenario_c.number_action;


        		////////////////////////trigger///////////////////////////

    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 0;
    			received_test->data.addCenario_c.type = CENARIO_GATE; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.app_gate.ID_gate; //Id partição
    			received_test->data.addCenario_c.mode = 0; //não aplicavel
    			received_test->data.addCenario_c.event_command = GATE_OPENED;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.trigger_type = received_test->data.addCenario_c.type;
    			app.cenario_register.trigger_ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.trigger_mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.trigger_event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.trigger_timeout = received_test->data.addCenario_c.timeout;


    			///////////////////////////////////////ação 1///////////////////////////////
    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 1;
    			received_test->data.addCenario_c.type = CENARIO_PARTITION; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.particoes_estado[0].id_particao; //Id partição
    			received_test->data.addCenario_c.mode = 0; //não aplicavel
    			received_test->data.addCenario_c.event_command = PARTITION_DISARM;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.acoes[0].type = received_test->data.addCenario_c.type;
    			app.cenario_register.acoes[0].ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.acoes[0].mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.acoes[0].event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.acoes[0].timeout = received_test->data.addCenario_c.timeout;

    			///////////////////////////////////////ação 2///////////////////////////////
    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 2;
    			received_test->data.addCenario_c.type = CENARIO_PGM; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = 52; //Id partição
    			received_test->data.addCenario_c.mode = MODE_TOOGLE; //numero do rele
    			received_test->data.addCenario_c.event_command = 0;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.acoes[1].type = received_test->data.addCenario_c.type;
    			app.cenario_register.acoes[1].ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.acoes[1].mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.acoes[1].event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.acoes[1].timeout = received_test->data.addCenario_c.timeout;


    			/////////////////////////////cadastro//////////////
    			seg_cad_cenario_from_cmd(&app.cenario_register);

    			/////////////////////////////test read////////////
    			seg_data_cenario_t test_cenario;
    			get_cenario(0, &test_cenario);


    		}
    		else if(status_Button == 2){
    			status_Button = 3;

    			received_test->data.initCenario_c.ID_cenario = 0x1021;
    			received_test->data.initCenario_c.number_action = 2;

        		app.cenario_register.ID_cenario = received_test->data.initCenario_c.ID_cenario;
        		app.cenario_register.qtd_acoes = received_test->data.initCenario_c.number_action;


        		////////////////////////trigger///////////////////////////

    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 0;
    			received_test->data.addCenario_c.type = CENARIO_GATE; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.app_gate.ID_gate; //Id partição
    			received_test->data.addCenario_c.mode = 0; //não aplicavel
    			received_test->data.addCenario_c.event_command = GATE_CLOSED;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.trigger_type = received_test->data.addCenario_c.type;
    			app.cenario_register.trigger_ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.trigger_mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.trigger_event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.trigger_timeout = received_test->data.addCenario_c.timeout;


    			///////////////////////////////////////ação 1///////////////////////////////
    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 1;
    			received_test->data.addCenario_c.type = CENARIO_PARTITION; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.particoes_estado[0].id_particao; //Id partição
    			received_test->data.addCenario_c.mode = 0; //não aplicavel
    			received_test->data.addCenario_c.event_command = PARTITION_ARM;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.acoes[0].type = received_test->data.addCenario_c.type;
    			app.cenario_register.acoes[0].ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.acoes[0].mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.acoes[0].event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.acoes[0].timeout = received_test->data.addCenario_c.timeout;

    			///////////////////////////////////////ação 2///////////////////////////////
    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 2;
    			received_test->data.addCenario_c.type = CENARIO_PGM; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = 52; //Id partição
    			received_test->data.addCenario_c.mode = MODE_TOOGLE; //numero do rele
    			received_test->data.addCenario_c.event_command = 1;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.acoes[1].type = received_test->data.addCenario_c.type;
    			app.cenario_register.acoes[1].ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.acoes[1].mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.acoes[1].event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.acoes[1].timeout = received_test->data.addCenario_c.timeout;


    			/////////////////////////////cadastro//////////////
    			seg_cad_cenario_from_cmd(&app.cenario_register);

    			/////////////////////////////test read////////////
    			seg_data_cenario_t test_cenario;
    			get_cenario(1, &test_cenario);


    		}
    		else if(status_Button == 3){
    			status_Button = 4;

    			received_test->data.initCenario_c.ID_cenario = 0x1022;
    			received_test->data.initCenario_c.number_action = 1;

        		app.cenario_register.ID_cenario = received_test->data.initCenario_c.ID_cenario;
        		app.cenario_register.qtd_acoes = received_test->data.initCenario_c.number_action;


        		////////////////////////trigger///////////////////////////

    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 0;
    			received_test->data.addCenario_c.type = CENARIO_VIOLATION; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.app_sensor.ID_sensor; //Id partição
    			received_test->data.addCenario_c.mode = 0; //não aplicavel
    			received_test->data.addCenario_c.event_command = 0;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.trigger_type = received_test->data.addCenario_c.type;
    			app.cenario_register.trigger_ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.trigger_mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.trigger_event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.trigger_timeout = received_test->data.addCenario_c.timeout;



    			///////////////////////////////////////ação 1///////////////////////////////
    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 1;
    			received_test->data.addCenario_c.type = CENARIO_PGM; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = 54; //Id partição
    			received_test->data.addCenario_c.mode = PGM_RETENTION; //numero do rele
    			received_test->data.addCenario_c.event_command = 6;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.acoes[0].type = received_test->data.addCenario_c.type;
    			app.cenario_register.acoes[0].ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.acoes[0].mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.acoes[0].event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.acoes[0].timeout = received_test->data.addCenario_c.timeout;


    			/////////////////////////////cadastro//////////////
    			seg_cad_cenario_from_cmd(&app.cenario_register);

    			/////////////////////////////test read////////////
    			seg_data_cenario_t test_cenario;
    			get_cenario(2, &test_cenario);


    		}
    		else if(status_Button == 4){
    			status_Button = 15;

    			received_test->data.initCenario_c.ID_cenario = 0x1023;
    			received_test->data.initCenario_c.number_action = 1;

        		app.cenario_register.ID_cenario = received_test->data.initCenario_c.ID_cenario;
        		app.cenario_register.qtd_acoes = received_test->data.initCenario_c.number_action;


        		////////////////////////trigger///////////////////////////

    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 0;
    			received_test->data.addCenario_c.type = CENARIO_SENSOR; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.app_sensor.ID_sensor; //Id partição
    			received_test->data.addCenario_c.mode = 0; //não aplicavel
    			received_test->data.addCenario_c.event_command = 0;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.trigger_type = received_test->data.addCenario_c.type;
    			app.cenario_register.trigger_ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.trigger_mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.trigger_event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.trigger_timeout = received_test->data.addCenario_c.timeout;



    			///////////////////////////////////////ação 1///////////////////////////////
    			received_test->data.addCenario_c.perm = 0;
    			received_test->data.addCenario_c.index_action = 1;
    			received_test->data.addCenario_c.type = CENARIO_GATE; //referente a partição
    			received_test->data.addCenario_c.ID_trigger_action = app.app_gate.ID_gate; //Id partição
    			received_test->data.addCenario_c.mode = 0; //0
    			received_test->data.addCenario_c.event_command = 0;
    			received_test->data.addCenario_c.timeout = 5;

    			app.cenario_register.acoes[0].type = received_test->data.addCenario_c.type;
    			app.cenario_register.acoes[0].ID_trigger_action = received_test->data.addCenario_c.ID_trigger_action;
    			app.cenario_register.acoes[0].mode = received_test->data.addCenario_c.mode;
    			app.cenario_register.acoes[0].event_command = received_test->data.addCenario_c.event_command;
    			app.cenario_register.acoes[0].timeout = received_test->data.addCenario_c.timeout;


    			/////////////////////////////cadastro//////////////
    			seg_cad_cenario_from_cmd(&app.cenario_register);

    			/////////////////////////////test read////////////
    			seg_data_cenario_t test_cenario;
    			get_cenario(0, &test_cenario);


    		}
    		else if(status_Button == 3){
    			status_Button = 4;
    			sendBack.properties = PROP_ASYNC;
        		app.packet.lastCmd = ALARMCMD_ADD_PGM;
        		app.packet.lastCmd_ASYNC = app.packet.lastCmd;

        		if(app.Wireless_mode == 1){
        			memcpy(app.Assync_id_job,app.app_connectivity.packet.id_job,16);
        		}

        		pgm_cmd_t pgm_cmd;
        	    pgm_cmd.ID = PGM_ID;
        	    pgm_cmd.function = PGM_REGISTER;

        	    pgm_start(&pgm_cmd);
        	    sendBack.data.len = 0;
    		}


    		cy_rtos_event_clearbits(&app.app_event, APP_EVENT_BUTTON);
    	}

    	app.event.mask = 0x00FFFFFF;

    }
}


