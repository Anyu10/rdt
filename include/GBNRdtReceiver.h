#pragma once

#include "RdtReceiver.h"

class GBNRdtReceiver : public RdtReceiver {
    
    private:
	
    int expected_seqnum_;	// 期待收到的下一个报文序号
    Packet last_ack_pkt_;

    public:

	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用

};