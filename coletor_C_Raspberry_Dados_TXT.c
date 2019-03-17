#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <stdio.h>
#include <stdlib.h>

//Defines
#define SAMPLEDIVISION 4
#define RESISTANCE 200.0
#define VREF 5.0
#define ADCRESOLUTION 255.0
#define TRANSFORMERRATIO 2000.0
#define SAMPLES 2000
//----------------------------------------------------------------------------------------



//Variables

int arquivoI2c;

int enderecoI2c;	// = 0x48;
//----------------------------------------------------------------------------------------

//functions



void setup(){

	enderecoI2c = 0x48;


}


int setupAdc(){

	char* diretorioArquivoI2c = "/dev/i2c-1";

	arquivoI2c = open(diretorioArquivoI2c, O_RDWR);

	if(arquivoI2c < 0){

		printf("Erro ao abrir\n");
		return 1;

	}

	if(ioctl(arquivoI2c, I2C_SLAVE, enderecoI2c) < 0){
		
		printf("Erro ao comunicar com escravo\n");
		return 1;

	}

}

int readAdc(){

	unsigned char dado[1] = {1};

	if(write(arquivoI2c, dado, 1) != 1){

		printf("Erro ao escrever\n");
		return -1;
	}


	read(arquivoI2c, dado, 1);

	return (int) dado[0];

}


int main(){

	printf("Ola mundo!\n");

	setup();
	setupAdc();
  //printing to file
  const char* nomeArquivo = "data.txt";
  const char* escrever = "a";
  FILE* arquivo;// = fopen(nomeArquivo, escrever);
  	float val;
	const int offsetVoltage = 3.411765;
	while(1){
		
		val = readAdc();
		val = (val * ((float)(5.0/255.0))) - offsetVoltage;
		val = val * TRANSFORMERRATIO / RESISTANCE;
		
		arquivo = fopen(nomeArquivo, escrever);
		char r[10];
		val *= 1000000; //val now im uA
		snprintf(&r[0], sizeof(r), "%d", ((int)val));
		fputs(&r[0], arquivo);
		fputc('\n', arquivo);


		fclose(arquivo);
		
		printf("%f\n", val);
		
	}


	return 0;
}
