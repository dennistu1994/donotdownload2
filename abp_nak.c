#include "sim_abp_nak.h"
#include <fstream>

int main(int argc, char* argv[]) {
    ofstream myfile;
    myfile.open ("ABP_NAK.csv");
    int H = 432;
    int l = 12000;
    int C = 5000000;
    double tps[2] = {0.005, 0.25};
    double tos[5] = {2.5, 5, 7.5, 10, 12.5};
    double bers[3] = {0, 1e-5, 1e-4};
    for(int a = 0; a<5;a++){
      for(int b = 0; b<2;b++){
        for(int c = 0; c<3;c++){
          SIM_ABP_NAK sim(tos[a] * tps[b], H, l, C, 10000, tps[b], bers[c]);
          myfile << sim.start();
          if(b != 1 || c !=2){
            myfile<<", ";
          }
        }
      }
      myfile<<'\n';
    }
    myfile.close();
}
