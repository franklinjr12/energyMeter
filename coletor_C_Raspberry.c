#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Defines
#define SAMPLEDIVISION 4
#define RESISTANCE 200.0
#define VREF 5.0
#define ADCRESOLUTION 256.0
#define TRANSFORMERRATIO 2000.0
#define SAMPLES 2000
//----------------------------------------------------------------------------------------


//Variables

int arquivoI2c;

int enderecoI2c;	// = 0x48;

int data[SAMPLES / SAMPLEDIVISION]; //data vector

float accumulator;	// = 0;
//----------------------------------------------------------------------------------------

//functions



void setup(){

	enderecoI2c = 0x48;

	accumulator = 0;

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


void loop(){

  //initializing accumulator and division loop
  
  accumulator = 0; //also known as current value in the end
  
  for(int j = 0; j < SAMPLEDIVISION; j++)
  {

    
    //sampling
    for(int i = 0; i < SAMPLES / SAMPLEDIVISION; i++)
    {
       data[i] = readAdc();
    }
   
    //----------------------------------------------------------------------------------------
    
  
    //taking voltage bias out due to offset
    float mean = 0;
    for(int i = 0; i < SAMPLES / SAMPLEDIVISION; i++)
          mean = mean + (float)data[i];
          
    mean = mean / (SAMPLES / SAMPLEDIVISION);

    //----------------------------------------------------------------------------------------

    for(int i = 0; i < SAMPLES / SAMPLEDIVISION; i++)
    {
    
      //Irms current. Absolute value
      if((float)data[i] - mean > 0)
        accumulator = accumulator + (float)data[i] - mean;
      
      else if (data[i] == 0);
        
      else
        accumulator = accumulator + (-1.0)*((float)(data[i]) - mean);
          
        
    }
  }//end of sample division loop
  //----------------------------------------------------------------------------------------

  
  //final calculations

  //transforming to values
  accumulator = accumulator * VREF / ((ADCRESOLUTION - 1.0) * RESISTANCE);

  
  

  //mean over all values
  accumulator = accumulator / ((float)SAMPLES);


	float calibrationConstante = 1.64483021518274;
	//calibrationConstante = 1;
  
  //transform to primary current with calibration constante
  accumulator = accumulator * TRANSFORMERRATIO * calibrationConstante;

  


  //low pass filter due to lack of precision
  //if(accumulator < 0.07) accumulator = 0; //lower than 70mA probabily is just noise
  //else accumulator = accumulator - 0.04; //subtract that noise

  //----------------------------------------------------------------------------------------






  //data output

  
  printf("Current: ");  printf("%f", accumulator);
  printf(" - Voltage: "); printf("127"); //fixed value cause still not having voltage sampling
  printf(" - Power: "); printf("%f", (accumulator * 127.0));
  printf("\n");


  //sending to webserver
  char sendServerStr[100] = "python POSTMAN.py oi_123 sensor_1 ";
  char numberConverted[30];

  sprintf(numberConverted, "%f", accumulator);

  strcat(sendServerStr, numberConverted);

  system(sendServerStr);


  //----------------------------------------------------------------------------------------

	/*
  //printing to file
  const char* nomeArquivo = "data.txt";
  const char* escrever = "a";
  FILE* arquivo = fopen(nomeArquivo, escrever);

  char r[10];
  accumulator *= 1000;
  snprintf(&r[0], sizeof(r), "%d", ((int)accumulator));
  fputs(&r[0], arquivo);
  fputc('\n', arquivo);


  fclose(arquivo);

	*/
}






//----------------------------------------------------------------------------------------


int main(){

	printf("Ola mundo!\n");



	setup();


	setupAdc();

	for(int i = 0; i < 100; i++)
		loop();


	return 0;
}

