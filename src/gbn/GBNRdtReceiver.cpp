#include "Global.h"
#include "GBNRdtReceiver.h"


GBNRdtReceiver::GBNRdtReceiver() : expected_seqnum_(1) {

	last_ack_pkt_.acknum = 0; //初始状态下，上次发送的确认包的确认序号为0, 使得当第一个接受的数据包出错时该确认报文的确认号为0
	last_ack_pkt_.checksum = 0;
	last_ack_pkt_.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE; i++){
		last_ack_pkt_.payload[i] = '.';
	}
	last_ack_pkt_.checksum = pUtils->calculateCheckSum(last_ack_pkt_);
}


GBNRdtReceiver::~GBNRdtReceiver() { }


void GBNRdtReceiver::receive(const Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && expected_seqnum_ == packet.seqnum) {
		pUtils->printPacket("===接收方正确收到发送方的报文", packet);

		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		//! 取出Message，向上递交给应用层

		last_ack_pkt_.acknum = packet.seqnum; //确认序号等于收到的报文序号
		last_ack_pkt_.checksum = pUtils->calculateCheckSum(last_ack_pkt_);

		pUtils->printPacket("接收方发送确认报文", last_ack_pkt_);
		pns->sendToNetworkLayer(SENDER, last_ack_pkt_);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		//! send ack packet back

		expected_seqnum_= packet.seqnum + 1; // 接收序号

	} else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文, 数据校验错误", packet);
		} else {
			pUtils->printPacket("接收方没有正确收到发送方的报文, 报文序号不对", packet);
		}
		
		pUtils->printPacket("接收方重新发送上次的确认报文", last_ack_pkt_);
		pns->sendToNetworkLayer(SENDER, last_ack_pkt_);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文

	}
}