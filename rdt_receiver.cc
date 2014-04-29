/*
 * FILE: rdt_receiver.cc
 * DESCRIPTION: Reliable data transfer receiver.
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
#include "rdt_receiver.h"
#include "rdt_help.h"



deque<struct myPacket> receiveBuffer;
myPacket feedback;

void uploadMsg(struct myPacket *mpkt)
{
	receiveBuffer.push_back(*mpkt);
	message msg;
	mp2msg(&msg, &receiveBuffer);
	Receiver_ToUpperLayer(&msg);
	receiveBuffer.clear();
	/*if(msg->data != NULL)
		free(msg->data);
    	if (msg!=NULL)
    		free(msg);*/
}


/* receiver initialization, called once at the very beginning */
void Receiver_Init()
{
	RECEIVE_SEQNUM = 0;
	RECEIVE_ACKNUM = 0;
	
	feedback.length = 0;
	feedback.msg = 1;
	feedback.offset = 0;
	feedback.flag = NAKFLAG;
	memset(feedback.payload, 0, PAYLOADSIZE);
	
	//FILE *file;
	//file = fopen("receiver.txt", "w");
	//fclose(file);
	
    fprintf(stdout, "At %.2fs: receiver initializing ...\n", GetSimulationTime());
}

/* receiver finalization, called once at the very end.
   you may find that you don't need it, in which case you can leave it blank.
   in certain cases, you might want to use this opportunity to release some 
   memory you allocated in Receiver_init(). */
void Receiver_Final()
{
    fprintf(stdout, "At %.2fs: receiver finalizing ...\n", GetSimulationTime());
}

/* event handler, called when a packet is passed from the lower layer at the 
   receiver */
void Receiver_FromLowerLayer(struct packet *pkt)
{

  
    //FILE *file;
    //file = fopen("receiver.txt", "a");
    
    myPacket mpkt;
    p2mp(pkt, &mpkt);
    
    
	//init feedback
	
    //verify checksum
    if(verifyChecksum(&mpkt))
    {
    	feedback.seqNum = RECEIVE_SEQNUM;
		RECEIVE_SEQNUM = (RECEIVE_SEQNUM + 1) % MAXSEQNUM;
    	if(mpkt.seqNum == RECEIVE_ACKNUM)
    	{	
    		//feedback.flag = ACKFLAG;
    		feedback.flag = ACKFLAG;
    		feedback.ackNum = RECEIVE_ACKNUM;
    		if(mpkt.msg == 1)
    		{
    			uploadMsg(&mpkt);
    			//fprintf(file, "receive success: %d\t ackNum%d\n", mpkt.seqNum, RECEIVE_ACKNUM);
    		}
    		else
    		{
    			receiveBuffer.push_back(mpkt);
    			//fprintf(file, "receive buffered: %d\t ackNum%d\n", mpkt.seqNum, RECEIVE_ACKNUM);
    		}
    		RECEIVE_ACKNUM = (RECEIVE_ACKNUM + 1) % MAXACKNUM;
    		//RECEIVE_ACKNUM ++;
    		
    	}
    	checksum(&feedback);
		packet temp;
		mp2p(&temp, &feedback);
		Receiver_ToLowerLayer(&temp);
		//fprintf(file, "feedback: %d, %d, %d\n", feedback.seqNum, feedback.ackNum, feedback.flag);
    }
    else
    {
    	//fprintf(file, "receive corrupt: %d, %d, %d\n", mpkt.seqNum, mpkt.ackNum, mpkt.msg);
    	//feedback.flag = NAKFLAG;
    }
    
    //fclose(file);
    //printf("receive checksum :%d\n", verifyChecksum(&feedback));    
    
}
