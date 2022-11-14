
#include "Global.h"
#include "SRRdtReceiver.h"


SRRdtReceiver::SRRdtReceiver() : win_size_{5}, base_{1}, next_seq_{1} {
	ack_pkt_.acknum = 0;
	ack_pkt_.checksum = 0;
	ack_pkt_.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		ack_pkt_.payload[i] = '.';
	}
	ack_pkt_.checksum = pUtils->calculateCheckSum(ack_pkt_);

	for (int i = 0; i < win_size_; ++i) {
		buffer_.push_back(Packet());
		ok_.push_back(false);
	}
}


SRRdtReceiver::~SRRdtReceiver() { }


void SRRdtReceiver::receive(const Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && packet.seqnum >= base_ && packet.seqnum < base_ + win_size_) {

		pUtils->printPacket("接收方正确收到发送方的报文", packet);

		Packet pkt = packet;
		Message msg;
		buffer_.at(pkt.seqnum - base_) = pkt;
		ok_.at(pkt.seqnum - base_) = true;

		ack_pkt_.acknum = packet.seqnum; 	//确认序号等于收到的报文序号
		ack_pkt_.checksum = pUtils->calculateCheckSum(ack_pkt_);
		pUtils->printPacket("接收方发送确认报文", ack_pkt_);
		pns->sendToNetworkLayer(SENDER, ack_pkt_);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		while (ok_.front()) {
			pkt = buffer_.front();
			buffer_.pop_front();
			buffer_.push_back(Packet());
			ok_.pop_front();
			ok_.push_back(false);

			//取出Message，向上递交给应用层
			memcpy(msg.data, pkt.payload, sizeof(pkt.payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			base_++;
		}
		
	} else if (packet.seqnum < base_) {	//! important, 不加可能会陷入死循环
	
		pUtils->printPacket("接收方收到发送方之前发送的报文", packet);

		ack_pkt_.acknum = packet.seqnum; 	//确认序号等于收到的报文序号
		ack_pkt_.checksum = pUtils->calculateCheckSum(ack_pkt_);
		pUtils->printPacket("接收方发送确认报文", ack_pkt_);
		pns->sendToNetworkLayer(SENDER, ack_pkt_);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

	} else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		} else {
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
		}
		// pUtils->printPacket("接收方重新发送上次的确认报文", ack_pkt_);
		// pns->sendToNetworkLayer(SENDER, ack_pkt_);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}