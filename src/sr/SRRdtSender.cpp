#include "Global.h"
#include "SRRdtSender.h"


SRRdtSender::SRRdtSender() : base_{1}, next_seq_{1}, win_size_{5} {
	for (int i = 0; i < win_size_; ++i) {
		ok_.push_back(false);
	}
}


SRRdtSender::~SRRdtSender() { }


bool SRRdtSender::getWaitingState() {
	return win_size_ == que_.size();
}


bool SRRdtSender::send(const Message &message) {
	if (getWaitingState()) { //发送方处于等待确认状态
		return false;
	}
	
	Packet pkt;

	pkt.acknum = -1; 	//忽略该字段
	pkt.seqnum = next_seq_++;
	pkt.checksum = 0;
	memcpy(pkt.payload, message.data, sizeof(message.data));
	pkt.checksum = pUtils->calculateCheckSum(pkt);
	pUtils->printPacket("发送方发送报文, 并启动对应的定时器", pkt);
	pns->sendToNetworkLayer(RECEIVER, pkt);		//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pns->startTimer(SENDER, Configuration::TIME_OUT, pkt.seqnum);	//启动发送方定时器

	que_.push_back(pkt);

	return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号落在窗口内
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= base_ && ackPkt.acknum < base_ + win_size_) {
		if (!ok_.at(ackPkt.acknum - base_)) {
			ok_.at(ackPkt.acknum - base_) = true;
			pUtils->printPacket("---发送方正确收到确认，并关闭响应的计时器", ackPkt);
			pns->stopTimer(SENDER, ackPkt.acknum);		//关闭定时器
		}
		int slide = 0;
		while (ok_.front()) {
			ok_.pop_front();
			ok_.push_back(false);
			que_.pop_front();
			base_++;
			slide++;
		}	//& 维护que_和ok_两个队列
		printf("------窗口移动了%d位\n", slide);

	}
	else {
		if (checkSum != ackPkt.checksum) {
			pUtils->printPacket("发送方没有正确收到确认, 收到的包损坏", ackPkt);
		} else {
			pUtils->printPacket("发送方没有正确收到确认, 收到的顺序超出窗口范围", ackPkt);
		}
	}
}

void SRRdtSender::timeoutHandler(int seqNum) {
	//唯一一个定时器,无需考虑seqNum
	// if (seqNum >= base_ && seqNum < base_ + win_size_) {
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", que_.at(seqNum - base_));
	pns->stopTimer(SENDER, seqNum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, que_.at(seqNum - base_));			//重新发送数据包
	// } else {
	// 	pns->stopTimer(SENDER, seqNum);
	// }
}
