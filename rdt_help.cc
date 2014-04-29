#include "rdt_help.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void checksum(struct myPacket *pkt)
{
	char buf[PAYLOADSIZE];
	memcpy(buf, pkt->payload, pkt->length); 
	
	//checksum
	unsigned long sum = 0;
	int length = (int)pkt->length;
	//fill the blank by 0
	for(int i = length; i < PAYLOADSIZE; i ++)
	{
		buf[i] = 0;
	
	}

	//calculate
	int temp;
	temp = (((pkt->length) << 8) & 0xff00) + ((pkt->msg) & 0xff);
	sum += (unsigned long)temp;
	temp = (((pkt->seqNum) << 8) & 0xff00) + ((pkt->ackNum) & 0xff);
	sum += (unsigned long)temp;
	temp = (((pkt->flag) << 8) & 0xff00) + ((pkt->offset & 0xff000000) >> 24);
	sum += (unsigned long)temp;
	temp = ((pkt->offset & 0x00ff0000) >> 8) + ((pkt->offset & 0x0000ff00) >> 8);
	sum += (unsigned long)temp;
	temp = ((pkt->offset & 0xff) << 8) + 0x00;
	sum += (unsigned long)temp;
	
	for(int i = 0; i < PAYLOADSIZE; i += 2)
	{
		temp =  ((buf[i]<<8) & 0xff00) + (buf[i+1] & 0xff);
	//	printf("this is temp 0x%x\n", temp);
		sum += (unsigned long)temp;
	}

	while(sum>>16)
		sum = (sum & 0xffff) + (sum>>16);

	sum = ~sum;
	pkt->checksum = sum;
	
}

bool verifyChecksum(struct myPacket *pkt)
{
	unsigned long sum = 0;
	
	int temp;
	temp = (((pkt->length) << 8) & 0xff00) + ((pkt->msg) & 0xff);
	sum += (unsigned long)temp;
	temp = (((pkt->seqNum) << 8) & 0xff00) + ((pkt->ackNum) & 0xff);
	sum += (unsigned long)temp;
	temp = (((pkt->flag) << 8) & 0xff00) + ((pkt->offset & 0xff000000) >> 24);
	sum += (unsigned long)temp;
	temp = ((pkt->offset & 0x00ff0000) >> 8) + ((pkt->offset & 0x0000ff00) >> 8);
	sum += (unsigned long)temp;
	temp = ((pkt->offset & 0xff) << 8) + 0x00;
	sum += (unsigned long)temp;
	
	for(int i = 0; i < PAYLOADSIZE; i += 2)
	{
		temp =  ((pkt->payload[i]<<8) & 0xff00) + (pkt->payload[i + 1] & 0xff);
		//printf("verify temp: 0x%x\n", temp);
		sum += (unsigned long)temp;
	}
	sum += pkt->checksum;
	while(sum>>16)
		sum = (sum & 0xffff) + (sum>>16);
	//printf("verify checksum: 0x%x\n", sum);
	
	return !(sum ^ 0xffff);
}

void msg2mp(struct message *msg, deque<struct myPacket> *pkts)
{
	int length = msg->size;
	int cursor = 0;
	while(cursor < length)
	{
		myPacket pkt;
		pkt.isAck = false;
		pkt.flag = DATAFLAG;
		//pkt.ttl = -1;
		int size = 0;
		if((length - cursor) > PAYLOADSIZE)
		{
			size = PAYLOADSIZE;
			pkt.msg = 0;
		}
		else
		{
			pkt.msg = 1;
			size = length - cursor;
		}
		pkt.length = size;
		pkt.offset = cursor;
		memset(pkt.payload, 0, PAYLOADSIZE);
		memcpy(pkt.payload, msg->data+cursor, size);
		cursor += PAYLOADSIZE;
		//checksum(&pkt);
		pkts->push_back(pkt);
	}
}

void mp2p(struct packet *pkt, struct myPacket *mpkt)
{
	memset(pkt->data, 0, RDT_PKTSIZE);
	//set header
	pkt->data[0] = mpkt->length;
	pkt->data[1] = mpkt->msg;
	pkt->data[2] = mpkt->seqNum;
	pkt->data[3] = mpkt->ackNum;
	pkt->data[4] = mpkt->flag;
	pkt->data[5] = (mpkt->offset & 0xff000000) >> 24;
	pkt->data[6] = (mpkt->offset & 0x00ff0000) >> 16;
	pkt->data[7] = (mpkt->offset & 0x0000ff00) >> 8;
	pkt->data[8] = (mpkt->offset & 0x000000ff);
	pkt->data[9] = (mpkt->checksum & 0xff00) >> 8;
	pkt->data[10] = (mpkt->checksum & 0x00ff);

	//copy data
	memcpy((pkt->data + 16), mpkt->payload, PAYLOADSIZE);
}

void p2mp(struct packet *pkt, struct myPacket *mpkt)
{
	memset(mpkt->payload, 0, PAYLOADSIZE);
	
	mpkt->length = (unsigned char)pkt->data[0];
	mpkt->msg = (unsigned char)pkt->data[1];
	mpkt->seqNum = (unsigned char)pkt->data[2];
	mpkt->ackNum = (unsigned char)pkt->data[3];
	mpkt->flag = (unsigned char)pkt->data[4];
	
	//get offset
	int one = (unsigned char)(pkt->data[5]);
	int two = (unsigned char)(pkt->data[6]);
	int three = (unsigned char)(pkt->data[7]);
	int four = (unsigned char)(pkt->data[8]);
	mpkt->offset = (unsigned int)((one << 24) + (two << 16) + (three << 8) + four);
	
	//get checksum
	int left = (unsigned char)(pkt->data[9]);
	int right = (unsigned char)(pkt->data[10]);
	mpkt->checksum = (word)((left << 8) + right);
	
	memcpy(mpkt->payload, (pkt->data + 16), PAYLOADSIZE);
}

void mp2mp(struct myPacket *sour, struct myPacket *dest)
{
	dest->length = sour->length;
	dest->msg = sour->msg;
	dest->seqNum = sour->seqNum;
	dest->ackNum = sour->ackNum;
	dest->flag = sour->flag;
	dest->offset = sour->offset;
	dest->checksum = sour->checksum;
	memcpy(dest->payload, sour->payload, PAYLOADSIZE);
	dest->isAck = sour->isAck;
}

void mp2msg(struct message *msg, deque<struct myPacket> *mpkts)
{
	int size = 0;
	for(int i = 0; i < mpkts->size(); i ++)
	{
		size += (*mpkts)[i].length;
	}
	msg->size = size;
	msg->data = (char*) malloc(size);
	
	//copy data
	for(int i = 0; i < mpkts->size(); i ++)
	{
		memcpy((msg->data + (*mpkts)[i].offset), (*mpkts)[i].payload, (*mpkts)[i].length);
	}
}

bool isIn(int num, int left)
{
	int max = left + WINDOWSIZE;
	int min = 0;
	if(max > MAXSEQNUM)
	{
		min = max - MAXSEQNUM;
		max = MAXSEQNUM;
	}
	return ((num > left) && (num < max)) || ((num >= 0) && (num < min));
}

void printTs(deque<struct timestamp> *ts)
{
	FILE *file;
	file = fopen("sendOut.txt", "a");
	fprintf(file, "timestamps:");
	for(int i = 0; i < (*ts).size(); i ++)
		fprintf(file, "\t%d,%f",(*ts)[i].seqNum, (*ts)[i].ttl);
	fprintf(file, "\n");
	fclose(file); 
}

void printBuffer(deque<struct myPacket> *buffer)
{
	FILE *file;
	file = fopen("sendOut.txt", "a");
	fprintf(file, "buffer:");
	for(int i = 0; i < (*buffer).size(); i ++)
		fprintf(file, "\t%d",(*buffer)[i].seqNum);
	fprintf(file, "\n");
	fclose(file); 
}

void printSlot(deque<struct myPacket> *slot)
{
	FILE *file;
	file = fopen("sendOut.txt", "a");
	fprintf(file, "slot:");
	for(int i = 0; i < (*slot).size(); i ++)
		fprintf(file, "\t%d",(*slot)[i].seqNum);
	fprintf(file, "\n");
	fclose(file); 
}


