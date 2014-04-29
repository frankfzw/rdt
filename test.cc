#include "rdt_struct.h"
#include "rdt_help.h"
#include <stdio.h>
#include <deque>


using namespace std;

int main()
{
	printf("I'm the test\n");
	
	//test checksum
	myPacket mpkt;
	for(int i = 0; i < PAYLOADSIZE; i ++)
	{
		mpkt.payload[i] = 0;
	}
	mpkt.payload[0] = 0x01;
	mpkt.payload[1] = 0x0;
	mpkt.payload[2] = 0xf2;
	mpkt.payload[3] = 0x3;
	mpkt.payload[4] = 0xf4;
	mpkt.payload[5] = 0xf5;
	mpkt.payload[6] = 0xf6;
	mpkt.payload[7] = 0xf7;
	mpkt.length = PAYLOADSIZE;
	printf("this is payload %x\n", mpkt.payload[3]);
	checksum(&mpkt);
	printf("the checksum is 0x%x\n", mpkt.checksum);
	bool result = verifyChecksum(&mpkt);
	printf("verify checksum is %d\n", result);

	message msg;
	msg.size = 256;
	msg.data = new char[256];
	
	for(int i = 0; i < msg.size; i ++)
	{
		msg.data[i] = i;
		//printf("msg: %d\n", msg.data[i]);
	}

	deque<struct myPacket> pkts;
	msg2mp(&msg, &pkts);
	printf("size %d\n", pkts.size());

	for(int i = 0; i < pkts.size(); i ++)
	{
		checksum(&pkts[i]);
		printf("payload msg: %x, %x, %x, %x\n", pkts[i].length, pkts[i].offset, pkts[i].msg, pkts[i].checksum);
	//	for(int j = 0; j < PAYLOADSIZE; j ++)
	//	{
	//		printf("payload %d %d\n", i, pkts[i].payload[j]);
	//	}
		packet p;
		mp2p(&p, &pkts[i]);
		printf("packet length: %d\n", p.data[0]);
		//printf("packet data: %x\t%x\t\t%x\t%x\n", (unsigned int)p.data[5], (unsigned int)p.data[6], (unsigned int)p.data[7], (unsigned int)p.data[8]);
		myPacket temp;
		p2mp(&p, &temp);
		printf("temp msg: %d, %d, %d, %x\n", temp.length, temp.offset, temp.msg, temp.checksum);
		result = verifyChecksum(&temp);
		printf("verify checksum is %d\n", result);
	}

	
	return 0;
}
