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
#define FREQ_TO_USEC(frequency)		(float)(1000*((1/(2*frequency)))
#define USEC_FREQUENCY				1000 //0x301			

typedef struct {
    Object 	super;
    int 	count;
    char 	c;
	char 	buf_c[100];
	int 	num_i;
	char 	text_c[100];
	int 	run_sum_i;
} App;

App app = { initObject(), 0, 'X', {0}, 0, {0} , 0};

typedef struct {
    Object 	super;
    int 	volume;
	char 	text_c[100];
	int 	call_count;
} Generate;

Generate generate = { initObject(), 5, {0}, 500};

typedef struct {
	Object 	super;
	int 	background_loop_range;
	int 	call_count;
} Background;

Background background = {initObject(), 1000, 500};

void reader(App*, int);
void receiver(App*, int);
void ShowBrotherJohnPeriods_v(App*, int);
void CreateTone_v(Generate *self, int volume_i);
void ChangeVolume_v(Generate *self, int ChangeVolume_i);
void BackgroundTask_v(Background *self, int unused_i);

Serial sci0 = initSerial(SCI_PORT0, &app, reader);
Serial sci1 = initSerial(SCI_PORT0, &app, ChangeVolume_v);

Can can0 = initCan(CAN_PORT0, &app, receiver);

int Volume_global_old_i = 0;
int Volume_global_i = 5;

void receiver(App *self, int unused) {
    CANMsg msg;
    CAN_RECEIVE(&can0, &msg);
    SCI_WRITE(&sci0, "Can msg received: ");
    SCI_WRITE(&sci0, msg.buff);
}

void reader(App *self, int c) {
	
	SCI_WRITE(&sci0, "Rcv: \'"); 
    SCI_WRITECHAR(&sci0, c);
    SCI_WRITE(&sci0, "\'\n");

	if(((48 <= c) && (c <= 57)) || ((c == 45) && (self->count == 0)))
	{		
		self->buf_c[self->count] = c;
		self->count++;
	}
	if (c == 'e')
	{
		self->num_i = atoi(self->buf_c);
		sprintf(self->text_c, "%d", self->num_i);
		SCI_WRITE(&sci0, "The entered Number is: ");		
		SCI_WRITE(&sci0, self->text_c);
		SCI_WRITE(&sci0, "\n"); 
		
		self->run_sum_i += atoi(self->buf_c);
		sprintf(self->text_c, "%d", self->run_sum_i);
		SCI_WRITE(&sci0, "The running sum is: ");		
		SCI_WRITE(&sci0, self->text_c);
		SCI_WRITE(&sci0, "\n");

		self->count = 0;
		memset(self->buf_c, 0, sizeof(self->buf_c));
		memset(self->text_c, 0, sizeof(self->text_c));
	}
	if(c == 'F')
	{
		self->run_sum_i = 0;
		sprintf(self->text_c, "%d", self->run_sum_i);
		SCI_WRITE(&sci0, "The running sum is: ");		
		SCI_WRITE(&sci0, self->text_c);
		SCI_WRITE(&sci0, "\n");
	}	
}

void ShowBrotherJohnPeriods_v(App *self, int key_i)
{		
	if(((48 <= key_i) && (key_i <= 53)) || ((key_i == 45) && (self->count == 0)))
	{		
		self->buf_c[self->count] = key_i;
		self->count++;
		self->num_i = atoi(self->buf_c);
		sprintf(self->text_c, "%d", self->num_i);
		SCI_WRITE(&sci1, "The entered Number is: ");		
		SCI_WRITE(&sci1, self->text_c);
		SCI_WRITE(&sci1, "\n"); 
	}
	if ((key_i == 'e') && (self->count != 0))
	{
		self->num_i = atoi(self->buf_c);
		SCI_WRITE(&sci1, "Transpose key: \'"); 
		SCI_WRITECHAR(&sci1, self->num_i);
		SCI_WRITE(&sci1, "\'\n");
		if((self->num_i<= 5) && (self->num_i >= -5))
		{
			SCI_WRITE(&sci1, "The periods for Brother John are: \n");		
			
			int BrotherJohnPeriodNew_ai[32];
			for(int x = 0; x <= 31; x++)
			{
				BrotherJohnPeriodNew_ai[x] = BrotherJohnBase_ai[x] + self->num_i;
			}
		
			for(int i = 0; i <= 31; i++)
			{
				self->num_i = BrotherJohnPeriodNew_ai[i];
				self->run_sum_i = BrotherJohnPeriod_ai[self->num_i+10];
				sprintf(self->buf_c, "%d", self->run_sum_i);
				SCI_WRITE(&sci1, self->buf_c);
				SCI_WRITE(&sci1, "\n");	
			}	
		}
		else
		{
			SCI_WRITE(&sci1, "Wrong input!! \n'"); 
		}
		
		self->count = 0;
		memset(self->text_c, 0, sizeof(self->text_c));
		memset(self->buf_c, 0, sizeof(self->buf_c));
	}
}

void CreateTone_v(Generate *self, int volume_i)
{
	self->volume = volume_i;
	Time period = USEC(500);
	
	if(DAC_READ() == 0)
	{
		DAC_WRITE(self->volume);
	}
	else
	{
		DAC_WRITE(0);
	}	
	
	if ( self->call_count > 0 ) {
		//AFTER(period, &generate, CreateTone_v, Volume_global_i);
		SYNC(&generate, CreateTone_v, Volume_global_i);
		self->call_count--;
	}
}

void ChangeVolume_v(Generate *self, int ChangeVolume_i)
{	
	if(ChangeVolume_i == 'w')
	{
		Volume_global_i++;
		
		if (Volume_global_i > 0x14)
		{
			Volume_global_i = 0x14;
		}		
	}
	else if(ChangeVolume_i == 's')
	{
		Volume_global_i--;
		
		if (Volume_global_i < 0x01)
		{
			Volume_global_i = 0x01;
		}		
	}
	else if(ChangeVolume_i == 'a')
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
	else if(ChangeVolume_i == 'l')
	{
		background.background_loop_range -= 500;
		
		if (background.background_loop_range < 1000)
		{
			background.background_loop_range = 1000;
		}
		sprintf(self->text_c, "%d", background.background_loop_range);
		SCI_WRITE(&sci1, "Loop range is: ");		
		SCI_WRITE(&sci1, self->text_c);
		SCI_WRITE(&sci1, "\n"); 	
	}
	else if(ChangeVolume_i == 'o')
	{
		background.background_loop_range += 500;
		
		if (background.background_loop_range > 8000)
		{
			background.background_loop_range = 8000;
		}	
		sprintf(self->text_c, "%d", background.background_loop_range);
		SCI_WRITE(&sci1, "Loop range is: ");		
		SCI_WRITE(&sci1, self->text_c);
		SCI_WRITE(&sci1, "\n"); 	
	}
}


void BackgroundTask_v(Background *self, int unused_i)
{
	Time period = USEC(1300);
	
	for (int i = 0; i<self->background_loop_range; i++)
	{
		//Do Nothing
	}
	
	if ( self->call_count > 0 )
	{
		//AFTER(period, &background, BackgroundTask_v, unused_i);
		SYNC( &background, BackgroundTask_v, unused_i);
		self->call_count--;
	}
}

void startApp(App *self, int arg) {
    CANMsg msg;

    CAN_INIT(&sci1);
    SCI_INIT(&sci1);
    SCI_WRITE(&sci1, "Hello, hello...\n");

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
//	ASYNC(&generate, CreateTone_v, Volume_global_i);
//	ASYNC(&background, BackgroundTask_v, 0);
	SYNC(&generate, CreateTone_v, Volume_global_i);
	//SYNC(&background, BackgroundTask_v, 0);
}

int main() {
    INSTALL(&sci1, sci_interrupt, SCI_IRQ0);
	INSTALL(&can0, can_interrupt, CAN_IRQ0);
    TINYTIMBER(&app, startApp, 0);
    return 0;
}
