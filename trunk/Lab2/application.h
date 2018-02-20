
int min_index_i = -10;
int max_index_i = 14;

// The base tone order for the key 0 
int BrotherJohnBase_ai[32] = {
	0,	2,	4,	0,	0,	2,	4,	0,	4,	5,	7,	4,	5,	7,	7,	9,	7,	5,	4,	0,	7,	9,	7,	5,	4,	0,	0,	-5,	0,	0,	-5,	0
	};

// The periods for the tones of the range -14 to 10
int BrotherJohnPeriod_ai[25] = {
	2025, 1911, 1804, 1703, 1607, 1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012, 956, 902, 851, 804, 758, 716, 676, 638, 602, 568, 536, 506
	};
	
// The Cycle length of the tones
double BrotherJohnCycle_ai[32] = {
	1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 2.0, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 0.5, 0.5, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 2.0
	};
	
struct asasdd {
	int BrotherJohnBase_ai;
	int key_i;
};
	