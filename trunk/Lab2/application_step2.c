#include "TinyTimber.h"
#include "sciTinyTimber.h"
#include "canTinyTimber.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "application.h"

#define DAC_adress					0x4000741C
#define DAC_WRITE(dac_value) 		*(char*)DAC_adress = dac_value
#define DAC_READ()					*(char*)DAC_adress

typedef struct {
    Object 	super;
    int 	key;
    int 	tempo;
	Time    beat;
	int 	volume;
	char 	text_c[100];
	int 	BrotherJohnPeriodNew_ai[32];
	int		counter;
	char	buf_c[100];
	int		count;
	int		num_i;
} MelodyPlayer;

MelodyPlayer melodyplayer = {initObject(), 0, 120, MSEC(500), USEC(1300), {0}, {0}, 0};

typedef struct {
    Object 	super;
    int 	volume;
	Time	period;
	int 	alive;
	Time	deadline;
	int		counter;
	double	buffer[500];
	char 	text_c[100];
} Generate;

Generate generate = { initObject(), 5, USEC(500), USEC(100), 0, 1, {0}, {0}};

void receiver(Generate*, int);
void CreateTone_v(Generate *self, int volume_i);
void ChangeParameters_v(MelodyPlayer *self, int ChangeVolume_i);
void Kill_v(Generate *self, int unsused);
void MelodyPlayer_v(MelodyPlayer *self, int unused);

Serial sci0 = initSerial(SCI_PORT0, &melodyplayer, ChangeParameters_v);

Can can0 = initCan(CAN_PORT0, &generate, receiver);

int Volume_global_old_i = 0;
int Volume_global_i = 10;

void receiver(Generate *self, int unused) {
    CANMsg msg;
    CAN_RECEIVE(&can0, &msg);
    SCI_WRITE(&sci0, "Can msg received: ");
    SCI_WRITE(&sci0, msg.buff);
}


void CreateTone_v(Generate *self, int volume_i)
{
	self->volume = volume_i;
	
	if(self->alive == 1)
	{
		if(DAC_READ() == 0)
		{
			DAC_WRITE(self->volume);
		}
		else
		{
			DAC_WRITE(0);
		}		
	}	
	else
	{
		DAC_WRITE(0);
	}
	
	//Recall the task	
	//AFTER(self->period, self, CreateTone_v, Volume_global_i);
	SEND(self->period, self->period, self, CreateTone_v, Volume_global_i);
}

void Kill_v(Generate *self, int unsused)
{
	self->alive = 0;
}

void ChangeParameters_v(MelodyPlayer *self, int input_i)
{	
	//Change Volume
	if(input_i == 'w')
	{
		Volume_global_i++;
		
		if (Volume_global_i > 0x14)
		{
			Volume_global_i = 0x14;
		}		
	}
	else if(input_i == 's')
	{
		Volume_global_i--;
		
		if (Volume_global_i < 0x01)
		{
			Volume_global_i = 0x01;
		}		
	}
	else if(input_i == 'a')
	{
		if(Volume_global_i != 0x00)
		{
			Volume_global_old_i = Volume_global_i;
			Volume_global_i = 0x00;			
		}
		else
		{
			Volume_global_i = Volume_global_old_i;
		}
	}
	
	else if(((48 <= input_i) && (input_i <= 57)) || ((input_i == 45) && (self->count == 0)))
	{		
		self->buf_c[self->count] = input_i;
		self->count++;
		self->num_i = atoi(self->buf_c);
		sprintf(self->text_c, "%d", self->num_i);
		SCI_WRITE(&sci0, "The entered Number is: ");		
		SCI_WRITE(&sci0, self->text_c);
		SCI_WRITE(&sci0, "\n"); 
	}
	
	//Change tempo
	else if(input_i == 't') 
	{
		if((self->num_i<= 240) && (self->num_i >= 60))
		{
			self->tempo = self->num_i;
			
			sprintf(self->text_c, "%d", self->tempo);
			SCI_WRITE(&sci0, "Tempo is: ");		
			SCI_WRITE(&sci0, self->text_c);
			SCI_WRITE(&sci0, "\n"); 	
		}
		else
		{
			SCI_WRITE(&sci0, "Ivalid tempo input\n");	
		}	
		
		self->count = 0;
		memset(self->text_c, 0, sizeof(self->text_c));
		memset(self->buf_c, 0, sizeof(self->buf_c));
	}
	
	//Change key
	else if(input_i == 'k')
	{		
		if((self->num_i<= 5) && (self->num_i >= -5))
		{		
			self->key = self->num_i ;	
			for(int x = 0; x <= 31; x++)
			{
				self->BrotherJohnPeriodNew_ai[x] = BrotherJohnBase_ai[x] + self->key;
			}
			
			generate.period = USEC(BrotherJohnPeriod_ai[self->BrotherJohnPeriodNew_ai[self->counter] + 10]);
			sprintf(self->text_c, "%d", self->key);
			SCI_WRITE(&sci0, "Key is: ");		
			SCI_WRITE(&sci0, self->text_c);
			SCI_WRITE(&sci0, "\n"); 		
		}
		else
		{
			SCI_WRITE(&sci0, "Ivalid key input\n");	
		}
		
		self->count = 0;
		memset(self->text_c, 0, sizeof(self->text_c));
		memset(self->buf_c, 0, sizeof(self->buf_c));
	}	
}


void MelodyPlayer_v(MelodyPlayer *self, int unused)
{	
	generate.alive = 1;
	generate.period = USEC(BrotherJohnPeriod_ai[self->BrotherJohnPeriodNew_ai[self->counter] + 10]);
	self->beat = MSEC(60000*BrotherJohnCycle_ai[self->counter]/self->tempo);
	self->counter++;
	if(self->counter > 31)
	{
		self->counter = 0;
	}
	AFTER(self->beat - MSEC(50), &generate, Kill_v, 0);
	SEND(self->beat, self->beat, self, MelodyPlayer_v, 0);
}

void startApp(MelodyPlayer *self, int arg) {
    CANMsg msg;

    CAN_INIT(&sci0);
    SCI_INIT(&sci0);
    SCI_WRITE(&sci0, "Hello, hello...\n");

    msg.msgId = 1;
    msg.nodeId = 1;
    msg.length = 6;
    msg.buff[0] = 'H';
    msg.buff[1] = 'e';
    msg.buff[2] = 'l';
    msg.buff[3] = 'l';
    msg.buff[4] = 'o';
    msg.buff[5] = 0;
    CAN_SEND(&can0, &msg);
	
	for(int x = 0; x <= 31; x++)
	{
		self->BrotherJohnPeriodNew_ai[x] = BrotherJohnBase_ai[x];
	}
	
	ASYNC(&generate, CreateTone_v, Volume_global_i);
	ASYNC(&melodyplayer, MelodyPlayer_v, 0);
}

int main() {
    INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
	INSTALL(&can0, can_interrupt, CAN_IRQ0);
    TINYTIMBER(&melodyplayer, startApp, 0);
    return 0;
}
