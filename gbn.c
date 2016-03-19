#include "sim_gbn_new.h"
int main(int argc, char* argv[])
{
    int H = 432;
    int l = 12000;
    int C = 5000000;
    double tp = atof(argv[1]); //10ms propagation delay
    double timeout = atof(argv[2]) * tp;
    double bit_error_rate = atof(argv[3]);
    SIM_GBN sim(timeout, H, l, C, 10000, tp, bit_error_rate, 4);
    sim.start();
}
