#include "Global.h"
#include "TCPRdtSender.h"
#include "DataStructure.h"


TCPRdtSender::TCPRdtSender() : base_{1}, next_seq_{1}, win_size_{5}, cumulation_{0} { }


TCPRdtSender::~TCPRdtSender() { }


bool TCPRdtSender::getWaitingState() {
	return que_.size() == win_size_;
}


bool TCPRdtSender::send(const Message &message) {
	if (getWaitingState()) { //发送方处于等待确认状态
		return false;
	}

	Packet pkt;
	pkt.seqnum = next_seq_;
	pkt.acknum = -1;
	pkt.checksum = 0;
	memcpy(pkt.payload, message.data, sizeof(message.data));
	pkt.checksum = pUtils->calculateCheckSum(pkt);
	
	pns->sendToNetworkLayer(RECEIVER, pkt);				//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pUtils->printPacket("发送方发送报文", pkt);

	que_.push_back(pkt);
	if (next_seq_ == base_) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, pkt.seqnum);	//启动发送方定时器
		printf("发送方启动了一个定时器\n");
	}
	next_seq_++;

	return true;
}

void TCPRdtSender::receive(const Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);

	//如果校验和正确，并且确认序号>发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum && ackPkt.acknum >= base_) {
		int gap = ackPkt.acknum + 1 - base_;
		for (int i = 0; i < gap; ++i) {
			que_.pop_front();
		}

		pns->stopTimer(SENDER, base_);
		base_ = ackPkt.acknum + 1;
		
		pUtils->printPacket("发送方正确收到确认, 关闭之前的定时器", ackPkt);

		if (base_ != next_seq_) {
			pns->startTimer(SENDER, Configuration::TIME_OUT, base_);
			printf("发送方重启了一个计时器\n");
		}

	
	} else {
		if (checkSum != ackPkt.checksum) {
			pUtils->printPacket("发送方没有正确收到确认, 包损坏", que_.front());
		} else if (ackPkt.acknum == base_ - 1) {
			cumulation_++;
			pUtils->printPacket("发送方没有正确收到确认, 但收到的acknum小于当前的base", que_.front());
		} else {
			pUtils->printPacket("发送方没有正确收到确认, 但收到的acknum小于当前的base", que_.front());
			return;
		}
		if (cumulation_ == 3) {
			pns->stopTimer(SENDER, base_);
			// for (Packet pkt : que_) {
			// 	pns->sendToNetworkLayer(RECEIVER, pkt);
			// 	pUtils->printPacket("发送方收到三个冗余确认, 重发报文中", pkt);
			// }
			if (que_.size() != 0) {
				pns->sendToNetworkLayer(RECEIVER, que_.at(0));
				pUtils->printPacket("&&&发送方收到三个冗余确认, 重发报文中", que_.at(0));
				cumulation_ = 0;
				pns->startTimer(SENDER, Configuration::TIME_OUT, base_);
			}
			// pns->startTimer(SENDER, Configuration::TIME_OUT, base_);
		}
	}

}

void TCPRdtSender::timeoutHandler(int seqNum) {
	pns->stopTimer(SENDER, seqNum);					//首先关闭定时器
	
	// for (int i = base_; i < next_seq_; ++i) {
	// 	pns->sendToNetworkLayer(RECEIVER, que_.at(i - base_));
	// 	pUtils->printPacket("发送方定时器时间到, 重发报文中", que_.at(i - base_));
	// }
	if (que_.size() != 0) {
		pns->sendToNetworkLayer(RECEIVER, que_.at(0));
		pUtils->printPacket("发送方定时器时间到, 重发报文中", que_.at(0));
		pns->startTimer(SENDER, Configuration::TIME_OUT, base_);
	}
}