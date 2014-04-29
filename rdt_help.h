#ifndef _RDT_SENDER_HELP_H_
#define _RDT_SENDER_HELP_H_

#include "rdt_struct.h"
#include <deque>

using namespace std;

typedef unsigned short word;
#define PAYLOADSIZE (RDT_PKTSIZE - 16)
#define WINDOWSIZE 10
#define ACKFLAG 1
#define DATAFLAG 0
#define NAKFLAG 2
#define INVALID 3
#define MAXSEQNUM 32
#define MAXACKNUM 32
/*
my packet format
|<- 1 byte ->|<- 1 byte ->|<- 1 byte ->|<- 1 byte ->|<- 1 byte ->|<- 2 byte ->|<- rest ->|
| length     | seqNum     | ackNum     | flag       | offset     | checksum   | payload  |
*/

struct myPacket {
	unsigned char length;
	unsigned char msg;
	unsigned char seqNum;
	unsigned char ackNum;
	unsigned char flag;
	unsigned int offset;
	word checksum;
	char payload[PAYLOADSIZE];
	
	//don't transmit these variables
	bool isAck;
};

struct timestamp {
	double ttl;
	unsigned char seqNum;
};
/* calculate checksum */
void checksum(struct myPacket *pkt);

/* verify checksum */
bool verifyChecksum(struct myPacket *pkt);

/* split the message */
void msg2mp(struct message *msg, deque<struct myPacket> *pkts);

/* convert myPacket to packet */
void mp2p(struct packet *pkt, struct myPacket *mpkt);

/* convert packet to myPacket */
void p2mp(struct packet *pkt, struct myPacket *mpkt);

/* copy myPacket */
void mp2mp(struct myPacket *sour, struct myPacket *dest);

/* convert myPackets to message */
void mp2msg(struct message *msg, deque<struct myPacket> *mpkts);

/* verify if the num is in slot */
bool isIn(int num, int left);

void printTs(deque<struct timestamp> *ts);
void printBuffer(deque<struct myPacket> *buffer);
void printSlot(deque<struct myPacket> *slot);


static int SEND_SEQNUM;
static int SEND_ACKNUM;

static int RECEIVE_SEQNUM;
static int RECEIVE_ACKNUM;



#endif
