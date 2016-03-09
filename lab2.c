#include "sim_abp.h"
double timeout = 5.0;
int H = 432;
int l = 12000;
int C = 5000000;
double tp = 0.01; //10ms propagation delay
double bit_error_rate = 0.0;
SIM_ABP sim(timeout, H, l, C, 10000, tp, bit_error_rate);
sim.start();


