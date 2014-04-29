/*
 * FILE: rdt_sender.cc
 * DESCRIPTION: Reliable data transfer sender.
 * NOTE: This implementation assumes there is no packet loss, corruption, or 
 *       reordering.  You will need to enhance it to deal with all these 
 *       situations.  In this implementation, the packet format is laid out as 
 *       the following:
 *       
 *       |<-  1 byte  ->|<-             the rest            ->|
 *       | payload size |<-             payload             ->|
 *
 *       The first byte of each packet indicates the size of the payload
 *       (excluding this single-byte header)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rdt_struct.h"
#include "rdt_sender.h"
#include "rdt_help.h"




deque<struct myPacket> sendBuffer;
deque<struct myPacket> slot;
deque<struct timestamp> ts;



/* send the packet from buffer to slot */
void sendPacketFromBuffer()
{
	if(sendBuffer.empty())
	{
		//printf("sendPacketFromBuffer: Oops! error\n");
		return;
	}
		
	//slot is not full
	if(slot.size() < WINDOWSIZE)
	{
		//FILE *file;
		//file = fopen("sendOut.txt", "a");
		
		myPacket mpkt;
		mp2mp(&sendBuffer.front(), &mpkt);
		
		mpkt.ackNum = SEND_SEQNUM;
		mpkt.seqNum = SEND_SEQNUM;
		mpkt.flag = DATAFLAG;
		mpkt.isAck = false;
		checksum(&mpkt);
		slot.push_back(mpkt);
		sendBuffer.pop_front();
		
		//set timestamp
		/*timestamp temp;
		temp.ttl = 0.3;
		temp.seqNum = SEND_SEQNUM;
		ts.push_back(temp);*/
		
		//send
		//fprintf(file, "\nsendPacketFromBuffer: %d, %d, %d\t pakckets in slot: %d\n", mpkt.seqNum, mpkt.ackNum, mpkt.msg, slot.size());
		
		//convert to packet
		packet pkt;
		mp2p(&pkt, &mpkt);
		Sender_ToLowerLayer(&pkt);
		//SEND_ACKNUM = (SEND_ACKNUM + 1) % MAXACKNUM;
		SEND_SEQNUM = (SEND_SEQNUM + 1) % MAXSEQNUM;
		//SEND_SEQNUM ++;
		
		//fclose(file);
		
		
		//printBuffer(&sendBuffer);
		//printSlot(&slot);
	}
}

/* modify ts: timeout or ack */
void modifyTs(int seqNum, bool isTimeout)
{
	if(!ts.empty())
		ts.pop_front();
}

/* sender initialization, called once at the very beginning */
void Sender_Init()
{	
	SEND_ACKNUM = 0;
	SEND_SEQNUM = 0;
	
	
	//FILE *file;
	//file = fopen("sendOut.txt", "w");
	//fclose(file);
	
	
	fprintf(stdout, "At %.2fs: sender initializing ...\n", GetSimulationTime());
}

/* sender finalization, called once at the very end.
   you may find that you don't need it, in which case you can leave it blank.
   in certain cases, you might want to take this opportunity to release some 
   memory you allocated in Sender_init(). */
void Sender_Final()
{
    fprintf(stdout, "At %.2fs: sender finalizing ...\n", GetSimulationTime());
}

/* event handler, called when a message is passed from the upper layer at the 
   sender */
void Sender_FromUpperLayer(struct message *msg)
{
   
	//split the message and store them in the buffer and calculate the checksum
	
	msg2mp(msg, &sendBuffer);
	//printf("sender: buffer size:%d\n", sendBuffer.size());
	
	//init timestamp
	
	//printf("AAAAAAAAAAAAAAAAAAA\n");
	for(int i = 0; i < WINDOWSIZE; i ++)
	{
		sendPacketFromBuffer();
	}
	
	if(!Sender_isTimerSet())
		Sender_StartTimer(0.3);
	//printf("Sender_FromUpperLayer: %d\t%f\n", Sender_isTimerSet, ts[0].ttl);
	
	//SEND_ACKNUM = 0;
	//SEND_SEQNUM = 0;

}

/* event handler, called when a packet is passed from the lower layer at the 
   sender */
void Sender_FromLowerLayer(struct packet *pkt)
{
	myPacket mpkt;
	p2mp(pkt, &mpkt);
	
	//checksum fail
	if(!verifyChecksum(&mpkt))
		return;
	
	int flag = mpkt.flag;
	
	//FILE *file;
	//file = fopen("sendOut.txt", "a");
	//fprintf(file, "Sender_FromLowerLayer feedback: %d, %d, %d\tackNum: %d\n", mpkt.seqNum, mpkt.ackNum, mpkt.flag, SEND_ACKNUM);
	
	
	
	switch(flag)
	{
		//ACK	
		case ACKFLAG:
			if(mpkt.ackNum == SEND_ACKNUM)
			{
				//fprintf(file, "Sender_FromLowerLayer success: %d\tackNum: %d\n", slot[0].seqNum, SEND_ACKNUM);
				if(slot.empty())
					return;
				
				slot.pop_front();
				SEND_ACKNUM = (SEND_ACKNUM + 1) % MAXACKNUM;
				sendPacketFromBuffer();
				Sender_StartTimer(0.3);
			}
			else if(isIn(mpkt.ackNum, SEND_ACKNUM))
			{
				int i = 0;
				for(; i < slot.size(); i ++)
				{
					if(slot[i].seqNum == mpkt.ackNum)
					{
						break;
					}
				}
				i ++;	
				while(i --)
				{
					if(slot.empty())
						break;
					slot.pop_front();
					sendPacketFromBuffer();
					SEND_ACKNUM = (SEND_ACKNUM + 1) % MAXACKNUM;
					Sender_StartTimer(0.3);
				}
			}
			break;
		default:
			break;
	}
	
	//fclose(file);
}

/* event handler, called when the timer expires */
void Sender_Timeout()
{
	//ASSERT(SEND_SEQNUM > SEND_ACKNUM);
	if(SEND_SEQNUM == SEND_ACKNUM)
	{
		Sender_StopTimer();
		return;
	}
	
	//FILE *file;
	//file = fopen("sendOut.txt", "a");
	//fprintf(file, "Sender_Timeout: %d, %d\n",SEND_SEQNUM, SEND_ACKNUM);
	if(Sender_isTimerSet())
		Sender_StopTimer();
	//resend
	packet pkt;
	for(int i = 0; i < slot.size(); i ++)
	{
		packet pkt;
		mp2p(&pkt, &slot[i]);
		//fprintf(file, "resend: %d\n",slot[i].seqNum);
		Sender_ToLowerLayer(&pkt);
	}
	if(!Sender_isTimerSet())
		Sender_StartTimer(0.3);
	//fclose(file);
}
