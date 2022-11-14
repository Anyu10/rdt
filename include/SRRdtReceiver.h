#pragma once

#include "RdtReceiver.h"
#include <deque>
#include <vector>
#include <array>

class SRRdtReceiver : public RdtReceiver {
    
    private:

    const int win_size_;

    int base_;
    int next_seq_;
    Packet ack_pkt_;
    std::deque<Packet> buffer_;
    std::deque<bool> ok_;

    public:

	SRRdtReceiver();
	virtual ~SRRdtReceiver();
	
	void receive(const Packet &packet);	//接收报文，将被NetworkService调用

};

