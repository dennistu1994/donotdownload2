#pragma once
class Sender {
    public:
        int SN=0;
        int NEXT_EXPECTED_ACK=1;
        int BAD_ACK=0;
        int IN_FLIGHT = 0;
        bool BUSY = false;
};
