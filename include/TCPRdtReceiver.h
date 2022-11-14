#pragma once

#include "RdtReceiver.h"
#include <deque>
#include <vector>
#include <array>

class TCPRdtReceiver : public RdtReceiver {
    
    private:

    int expected_seqnum_;
    Packet ack_pkt_;


    public:

	TCPRdtReceiver();
	virtual ~TCPRdtReceiver();
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用

};

