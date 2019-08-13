// Reference: idevicelocation, libimobiledevice API
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/service.h>

void clean(lockdownd_client_t client, idevice_t phone);
void service_error();
void usage();

int main(int argc, char *argv[]){
	idevice_t phone = NULL;
	lockdownd_client_t client = NULL;
	service_client_t service_client = NULL;
	lockdownd_service_descriptor_t service = NULL;
	char **device_list;
	int count = 0;
	
	if(argc < 2){
		usage();
	}

	idevice_error_t status = idevice_get_device_list(&device_list,&count);
	if(IDEVICE_E_SUCCESS != status){
		fprintf(stderr,"Devices not found\n");
		exit(-1);
	}
	char device_id[100];
	strcpy(device_id,device_list[--count]);

	status = idevice_new(&phone,device_id);
	if(IDEVICE_E_SUCCESS != status){
		fprintf(stderr,"Couldn't connect\n");
		exit(-1);
	}

	status = lockdownd_client_new_with_handshake(phone,&client,"ioshack");
	if(LOCKDOWN_E_SUCCESS != status){
		fprintf(stderr,"Couldn't lockdown\n");
		clean(client,phone);
	}else{
		printf("[!] CONNECTED DEVICE UDID: %s\n",device_id);
	}

	status = lockdownd_start_service(client,"com.apple.dt.simulatelocation",&service);
	if(LOCKDOWN_E_SUCCESS != status){
		fprintf(stderr,"Couldn't start com.apple.dt.simulatelocation\n");
		clean(client,phone);
	}else{
		printf("[!] Start com.apple.dt.simulatelocation\n");
	}
	
	service_error_t service_err = service_client_new(phone,service,&service_client);
	if(service_err){
		fprintf(stderr,"Couldn't create service client\n");
		exit(-1);
	}

	// stop GPS faking
	if(strcmp(argv[1],"-s") == 0){
		printf("[!] STOP\n");
		uint32_t stop_msg = htobe32(1);
		service_err = service_send(service_client,(void*)&stop_msg,sizeof(stop_msg),0);
		if(service_err){
			service_error();
		}
		exit(0);
	}else{
		FILE *fp;
		char latitude[50];
		char longtitude[50];
		if((fp = fopen(argv[1],"r")) == NULL){
			fprintf(stderr,"IO Error\n");
			exit(-1);
		}

		while((fscanf(fp,"%s %s",latitude,longtitude)) != EOF){
			// start GPS faking
			uint32_t start_msg = htobe32(0);
			uint32_t latitude_len = htobe32(strlen(latitude));
			uint32_t longtitude_len = htobe32(strlen(longtitude));
			service_err = service_send(service_client,(void*)&start_msg,sizeof(start_msg),0);
			if(service_err){
				service_error();
			}

			service_err = service_send(service_client,(void*)&latitude_len,sizeof(latitude_len),0);
			service_err = service_send(service_client,latitude,strlen(latitude),0);
			if(service_err){
				service_error();
			}
			printf("[*] UPDATED LATITUDE: %s\n",latitude);

			service_err = service_send(service_client,(void*)&longtitude_len,sizeof(longtitude_len),0);
			service_err = service_send(service_client,longtitude,strlen(longtitude),0);
			if(service_err){
				service_error();
			}
			printf("[*] UPDATED LONGTITUDE: %s\n",longtitude);
			printf("sleep 3 sec...\n\n");
			sleep(3);

			// clean

			if(service_client){
				service_client_free(service_client);
			}
			service_client = NULL;
			if(service){
				lockdownd_service_descriptor_free(service);
			}
			service = NULL;
			if(client){
				lockdownd_client_free(client);
				client = NULL;
			}
			//

			// shit, redefine
			status = idevice_new(&phone,device_id);
			if(IDEVICE_E_SUCCESS != status){
				fprintf(stderr,"Couldn't connect\n");
				exit(-1);
			}
			status = lockdownd_client_new_with_handshake(phone,&client,"ioshack");
			if(LOCKDOWN_E_SUCCESS != status){
				fprintf(stderr,"Couldn't lockdown\n");
				clean(client,phone);
			}
			status = lockdownd_start_service(client,"com.apple.dt.simulatelocation",&service);
			if(LOCKDOWN_E_SUCCESS != status){
				fprintf(stderr,"Couldn't start com.apple.dt.simulatelocation\n");
				clean(client,phone);
			}
			service_error_t service_err = service_client_new(phone,service,&service_client);
			if(service_err){
				fprintf(stderr,"Couldn't create service client\n");
				exit(-1);
			}
			//
		}
	}
	return 0;
}

void clean(lockdownd_client_t client, idevice_t phone){
	if(client){
		lockdownd_client_free(client);
	}
	idevice_free(phone);
}

void service_error(){
	fprintf(stderr,"Couldn't send data\n");
	exit(-1);
}

void usage(){
	printf("./pkwalker <coordinate file>	: start GPS faking\n./pkwalker -s	: stop GPS faking\n");
	exit(0);
}