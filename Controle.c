#include<reg52.h>
#include<stdio.h>
#include<stdlib.h>

/////////////////////MEUS PINOS////////////////////////

//para o controle
sbit b_esquerda	= P1^3;
sbit b_frente 		= P1^4;
sbit b_direita		= P1^5;
sbit b_traz			= P1^6;
sbit b_acao       = P1^7;
sbit b_start      = P3^2;

sbit luz 	= P3^4;		//Sinal luminoso em nivel alto=1 apagado
sbit out_d1 = P2^4;		//verifica qndo cada driver estará em funcionamento
sbit out_d2 = P0^6;		//verifica qndo cada driver estará em funcionamento
sbit out_d3 = P0^5;		//verifica qndo cada driver estará em funcionamento
sbit botao0 = P0^0;		//botao externo
sbit botao1 = P0^1;		//botao externo
sbit trg_d2 = P0^7;		//Trigger driver 2
sbit trg_d1 = P2^0;		//Trigger driver 1
sbit sirene = P1^0;     //sirene
sbit bomba1 = P1^2;		//saida da bomba 1 que joga ahua pra dentro
sbit bomba2 = P1^1;		//saida que ativa a bomba que joga agua para fora do reservatório
sbit sensor = P3^3;     //entrada do sensor000000

/////////////////////MINHAS VARIAVEIS//////////////////////////
long unsigned distancia;
long unsigned fuso=9000;

unsigned char dado=0;
signed char indice=0;
bit enable=0;
bit motor;
bit motores=0;
bit autonomo=0;
int posicao=0;			//variavel para posicionar as distancias depois de serem colhidas
int maximo=0;

/////////////////////////MATRIZ///////////////
idata unsigned char temp[12];			//matriz para armazenamento de distancias recebidas
idata signed long m1[22];
idata signed long m2[22];

/////////////////////MINHAS ROTINAS////////////////////////////
void ler_serial(void) interrupt 4;

void motor_a();						//motores para automaticos

void espera_1();						//rotina para esperar cada motor executar seu movimento
void espera_2();
void espera_3();

void motor3_c();
void motor3_cf();
void motor3_b();
void motor3_bf();

void time(int h);						//rotina de tempo aproximadamente 1/2 segundo
void posicao_driver();				//rotina que solicita a captura a posição do driver

void furar();							//rotina de furação completa

void automatico();
void scanear_controle();
void frente();
void direita();
void esquerda();
void traz();

/////////////////////////////////MEU PROGRAMA PRINCIPAL///////////////////////////////
void main (void)
{
	//inicialização dos atuadores do robo (tudo desativado)
	bomba1=0;					//mantem a bomba1 desligada
	bomba2=0;					//mantem a bomba2 desligada
	sirene=0;					//desliga a sirene
	luz=1; 						//desliga lampada
	trg_d1=0;					//inicializa os trigger's em zero
	trg_d2=0;

	//inicializazção da serial = 9600
	PCON = 0;
	TCON = 0x40;				//habilita o uso do timer do MC
	TH1  = 0xFD;				//timer 1 com valor respectivo p/ baud rate de 9600
	TMOD = 0x20;				//programa timer em modo 1 com auto-reload
	SCON = 0x52;				//habilita modo1 da serial comunicação assincrona, seta bit REN e TI

	//inicialização de interrupções Geral = Serial = Externa1
	EA=1;   						//seta bit que habilita todas interrupções
	ES=0;							//deixa interrupção serial desabilitada no inicio
//	EX1=0;						//desabilita a interrupção externa pelo pino 13 P3^3 timer 1
	PS=1;							//define a interrupção serial no grupo de alta prioridade
	PX1=0;						//define a interrupção externa1 P3^3 no grupo de baixa prioridade

	//Diminui o consumo de corrente de todos os Drivers, deixando os motores desacionados
	printf("1ST1 ");
	printf("2ST1 ");
	printf("3ST1 ");
	printf("4ST1 ");

	while (1)
	{
		while(b_acao && autonomo)
			{
				printf("1ST0 ");
				printf("2ST0 ");			//se for a segunda passagem por essa parte do programa o robo irá	
	  		 	automatico();				//repetir passos que já estejam gravados na memória
	 		}
		while(b_start==1)					 //botão Start
			{
				while(b_start==1);		//talvez um time antes dessa linha de tempo
				time(2);						//espera um monte de segundos
				//habilita perfifericos
//	sensor	EX1=1;						//habilita a interrupção externa pelo pino 13 P3^3 timer 1
				printf("1ST0 ");
				printf("2ST0 ");
				time(1);
				
				scanear_controle();
				time(1);
				while(b_start==1);
				automatico();

   			//desabilita perfifericos
				luz=1;
				sirene=0;
				printf("1ST1 ");
				printf("2ST1 ");
			}
   	while(botao0==1)				//Botão esquerdo no Robô Destrava os Motores
			{
				printf("1ST1 ");
				printf("2ST1 ");
				printf("3ST1 ");
				printf("4ST1 ");
			}
 	}
}
void scanear_controle()
{
	while(b_start==0) //encontrar uma ocasião que trava na linha do scaneamento!!!!!
		{
			luz=0;
			printf("1K 1PZ ");
			printf("2K 2PZ ");
			if(b_frente) frente();
			if(b_esquerda) esquerda();
			if(b_direita) direita();
			if(b_traz) traz();
			if(b_acao) 
				{
					furar();				//ao acionar o botão acao, realiza a simulação do furo e retorna
					time(6);				//espera tempo curto
					automatico();     //vai para rotina que retorna o robo automaticamente para sua posição de origem, e demostra todo trajeto gravado por uma vez.
				}
		}
}
void frente()
{
	time(1);
	luz=0;
	while(1)
	{
		if(b_frente==1)
		{
			printf("1A0.5 1V0.5 1D+ 1MC 1TR0XX 1G ");		//setando acelereação, velocidade, sentido, modo continuo, espera trigger, executa. 
			printf("2A0.5 2V0.5 2D- 2MC 2TR0XX 2G ");	
			time(1);
			trg_d1=1;
			trg_d2=1;
			while(b_frente==1);
			printf("K ");
			trg_d1=0;
			trg_d2=0;
		}
		if(b_traz==1)
		{
			printf("1A0.5 1V0.5 1D- 1MC 1TR0XX 1G ");		//setando acelereação, velocidade, sentido, modo continuo, espera trigger, executa. 
			printf("2A0.5 2V0.5 2D+ 2MC 2TR0XX 2G ");		//inverte o sentido da rotação dos motores
			time(1);
			trg_d1=1;
			trg_d2=1;
			while(b_traz==1);
			printf("K ");
			trg_d1=0;
			trg_d2=0;
		}
		if(b_start==1)
		{
			luz=1;
			sirene=1;
			time(3);
			sirene=0;
			break;
		}
	}
	posicao_driver();
	while(b_start);			//trava nessa linha enquanto o start for acionado, assim q soltar o botão volta a scanear
	time(3);						//tempo para evitar "debounce" 
}
void direita()
{
	time(1);
	luz=0;
	while(1)
		{
			if(b_direita==1)
			{
				printf("2A0.5 2V0.5 2D- 2MC 2TR0XX 2G ");      //Driver com sentido a frente
				time(1);
				trg_d2=1;
				while(b_direita==1);
				printf("K ");
				trg_d2=0;
			}
			if(b_esquerda==1)
			{
				printf("2A0.5 2V0.5 2D+ 2MC 2TR0XX 2G ");  //sentido inverso no Driver
				time(1);
				trg_d2=1;
				while(b_traz==1);
				printf("K ");
				trg_d2=0;
			}
			if(b_start==1)
			{
				luz=1;
				sirene=1;
				time(3);
				sirene=0;
				break;
			}
		}
	posicao_driver();
	while(b_start);			//trava nessa linha enquanto o start for acionado, assim q soltar o botão volta a scanear
	time(3);					//tempo para evitar "debounce" no controle
}
void esquerda()
{
	time(1);
	luz=0;
	while(1)
		{
			if(b_esquerda==1)
			{
				printf("1A0.5 1V0.5 1D+ 1MC 1TR0XX 1G ");      //Driver com sentido a frente
				time(1);
				trg_d1=1;
				while(b_esquerda==1);
				printf("K ");
				trg_d1=0;
			}
			if(b_direita==1)
			{
				printf("1A0.5 1V0.5 1D- 1MC 1TR0XX 1G ");  //sentido inverso no Driver
				time(1);
				trg_d1=1;
				while(b_traz==1);
				printf("K ");
				trg_d1=0;
			}
			if(b_start==1)
			{
				luz=1;
				sirene=1;
				time(3);
				sirene=0;
				break;
			}
		}	
	posicao_driver();
	while(b_start);			//trava nessa linha enquanto o start for acionado, assim q soltar o botão volta a scanear
	time(3);					//tempo para evitar "debounce" no controle
}
void traz()
{
	time(1);
	luz=0;
	while(1)
		{
			if(b_traz==1)
			{
				printf("1A0.5 1V0.5 1D- 1MC 1TR0XX 1G ");		//setando acelereação, velocidade, sentido, modo continuo, espera trigger, executa. 
				printf("2A0.5 2V0.5 2D+ 2MC 2TR0XX 2G ");
				time(1);
				trg_d1=1;
				trg_d2=1;
				while(b_traz==1);
				printf("K ");
				trg_d1=0;
				trg_d2=0;
			}
			if(b_frente==1)
			{
				printf("1A0.5 1V0.5 1D+ 1MC 1TR0XX 1G ");		//setando acelereação, velocidade, sentido, modo continuo, espera trigger, executa. 
				printf("2A0.5 2V0.5 2D- 2MC 2TR0XX 2G ");		//inverte o sentido da rotação dos motores
				time(1);
				trg_d1=1;
				trg_d2=1;
				while(b_frente==1);
				printf("K ");
				trg_d1=0;
				trg_d2=0;
			}
			if(b_start==1)
			{
				luz=1;
				sirene=1;
				time(3);
				sirene=0;
				break;
			}
		}
	posicao_driver();
	while(b_start);			//trava nessa linha enquanto o start for acionado, assim q soltar o botão volta a scanear
	time(3);					//tempo para evitar "debounce" no controle
}
void automatico()
{
	time(5);
	luz=0;
	autonomo=1;
	while(1)			// 1º laço para retornar
	{
		if(!posicao)
		{
			time(10);
			break;
		}
		posicao--;
		m1[posicao]=m1[posicao]*-1;
		m2[posicao]=m2[posicao]*-1;
		motor_a();
	}
	while(posicao!=maximo)                  //laço para realizar os passo adiante
	{
		m1[posicao]=m1[posicao]*-1;
		m2[posicao]=m2[posicao]*-1;
		motor_a();
		posicao++;
	}
	while(posicao==maximo) 				//2º laço para retornar
	{
		furar();
		while(1)
		{
			if(!posicao)
			{
				time(10);
				break;
			}
			posicao--;
			m1[posicao]=m1[posicao]*-1;
			m2[posicao]=m2[posicao]*-1;
			motor_a();
		}
	}
	time(5);
	luz=1;
	main();
}
//rotinas de comando aos drivers PARKER
void motor_a()
{
 	printf("1K 1LD3 1MN 1PZ 1TR0XX 1A0.5 1V0.5 1O11 1D%ld 1G 1O00 1XT 1XR1 ",m1[posicao]);
 	printf("2K 2LD3 2MN 2PZ 2TR0XX 2A0.5 2V0.5 2O11 2D%ld 2G 2O00 2XT 2XR1 ",m2[posicao]);
	trg_d1=1;
	trg_d2=1;
	espera_1();
	espera_2();
}
void motor3_c()								//rotina para subir suporte da furadeira
{
 	printf("3ST0 ");
	time(1);

 	printf("3K 3LD3 3A10 3V40 3O11 3D-%0.0lu 3G 3O00 3XT ",distancia);
 	espera_3();

 	printf("3ST1 ");

}
void motor3_cf()								//rotina para retornar com a furadeira ligada
{
 	printf("3ST0 ");
	time(1);
 	printf("3K 3LD3 3A40 3V40 3O11 3D-%0.0lu 3G 3O00 3XT ",distancia);
 	time(1);
 	while(out_d3==1)
 	{
	 	printf("4S ");
 	}		
 	printf("3ST1 ");
}
void motor3_b()								//rotina que baixa o suporte da furadeira
{
 	printf("3ST0 ");
	time(1);
	
 	printf("3K 3LD3 3A200 3V40 3O11 3D+%0.0lu 3G 3O00 3XT ",distancia); 
 	espera_3();
 	printf("3ST1 ");
}
void motor3_bf()					//rotina para baixar com a furadeira ligada (avanço lento)
{
 	printf("3ST0 ");
	time(1);	
	
 	printf("3K 3LD3 3A200 3V5 3O11 3D+%0.0lu 3G 3O00 3XT ",distancia); 
 	printf("4ST0 ");
	time(1);	
 	printf("4K 4LD3 4A100 4V9 4O11 4H 4D+4064000 4G 4O00 4XT ");
 	espera_3();
 	
 	printf("3ST1 ");
 	printf("4ST1 ");
}
void espera_1()
{
	time(1);					//conta um tempo apenas para variar a saida do driver
	while(out_d1==1);  	//enquanto a saida mostrar q o driver está funcionando espera
	trg_d1=0;				//desabilita o trigger do driver
} 	
void espera_2()
{
	time(1);					//conta um tempo apenas para variar a saida do driver
	while(out_d2==1);		//enquanto a saida mostrar q o driver está funcionando espera nessa linha
	trg_d2=0;				//desabilita o trigger do driver
} 	
void espera_3()
{
   time(1);
   while(out_d3==1);
}
void furar()
{											//1mm = 9000ppr
	luz=1;								//desliga a luz
	sirene=1;							//liga a sirene
	distancia=fuso*100;				//profundidade para achegar perto do solo
	motor3_b();							
	distancia=fuso*50;				//rotina desce realizando o furo
	motor3_bf();		
	distancia=fuso*150;          //rotina para retornar a broca a sua posição inicial
	motor3_cf();						
	luz=0;
	sirene=0;
}
void time(int h) 					//contagem MENOR QUE 1/2 segundo
{
	int f;
	int g;
	while(h!=0)
	{
		h--;
		f=4;
		while(f!=0)
			{
			 for(g=0;g<=57350;g++);
			 	{
			 		f--;
				}
			}
	}
}
void posicao_driver()
{
//INICIA PROCESSO DE REQUECIÇÃO DE POSIÇÃO E ARMAZENAMENTO  							
 		printf("1TR0XX 1PR ");  //manda o driver esperar o trigger, para depois disso dar a posição real
 	  	time(1);				  		
   	trg_d1=1;
  	 	TI=0;							//habilita bit de interupção na transmissão
   	ES=1;							//seta bit que habilita interrupção serial
   	motor=0;						//bit motor=0 para escrever na posição referente a tal roda; em algum momento daki em diante gerará uma interrupção na Serial
   	enable=0;
   	indice=0;	
  				
 		while(indice<=10);		//enquanto o indice for menor/igual a 10 atribui o valor de dado na tabela M1 no endereço apontado pelo indic	
 		ES=0;							//desabilita interrupção da serial
 		temp[11]=0;					//coloca numero zero na ultima posição da tabela para construir assim uma string valida
 		TI=1;							//habilita o bit de interupção na transmissão			 					
 		trg_d1=0;

		m1[posicao] = atol (temp);
 		
   	printf("2TR0XX 2PR ");
  		time(1);
  		trg_d2=1;
  		TI=0;
   	ES=1;
   	motor=1;					//motor=1 para escrever em posições referente a tal roda!
   	enable=0;
   	indice=0;
   							
 		while(indice<=10);
 		ES=0;		
 		temp[11]=0;
 		TI=1;		   
   	trg_d2=0;	   
   	
   	m2[posicao] = atol (temp);
  	 	posicao++;		//incrementa o ponteiro tabela, para guardar o valor colhido em uma nova posição
   	maximo=posicao;
}
void ler_serial(void) interrupt 4
{
	ES=0;
	if(SBUF=='-' || SBUF=='+') enable=1;
	if(enable)
		{
		if(!motor)
			{
			temp[indice]=SBUF;
			indice++;
			}
		if(motor)
			{
			temp[indice]=SBUF;
			indice++;
			}
		}
   RI=0;
	ES=1;
}
