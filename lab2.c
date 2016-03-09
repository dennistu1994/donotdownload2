#include "sim_abp.h"
double timeout = 5.0;
int H = 5;
int l = 1000;
int C = 1000000;
double tp = 0.01; //10ms propagation delay
SIM_ABP sim(timeout, H, l, C, 50000, tp, double bit_error_rate)


