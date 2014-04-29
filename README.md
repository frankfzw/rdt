rdt
===
lab 1 of Distributed System

sending and receiveing side of a reliable data transport portocol.

Using sliding windows (Go-Back-N)



Design and implementation strategies:
	这次实验我采用的是GO-BACK-N的方式来实现sender和receiver之间的通讯。
	sender:
		用seqNum和ackNum(0~31 round robin)来保存下一个要发出去的包的sequence number和希望从receiver收到的acknowledge number。
		当message传送到rdt layer时，先将其拆分缓存，只要sliding window不满，就讲packet填充到slot中，并向lower layer发送packet。
		当收到receiver的包时，先计算checksum，如果出错则丢掉
		当收到一个期望的ackNum时（及receiver发送的ackNum == sender记录的ackNum），移动window，从buffer取出一个packet发送，并将ackNum ++
		当收到ackNum不是期望值但是在window的范围之内，就将sender的ackNum更新到该值，并且把这个ack对应的packet以及他之前的包从slot中删去，从buffer中取出包发送
		当收到ackNum不在window范围内，丢弃
		timeout：每个包0.3s一旦触发，就将slot中所有的packet重发一遍
		
	receiver:
		用seqNum和ackNum(0~31 round robin)来保存下一个要发出去的包的sequence number和希望从sender收到的acknowledge number
		用以个buffer来保存被拆分的包
		当收到一个packet时做checksum，如果出错则丢掉，不返回给sender信息
		当收到一个期望的ackNum时，如果改packet被标记为一个结尾，则和buffer中的缓存一起组装上传，否则放入缓存中，并将ackNum ++，发送ack给sender
		当收到ackNum不是期望值，发送一个ack给sender用来表示当前状态
		
	rdt_help:
		自己添加的c文件
		声明数据结构myPacket，方便操作，具体结构请参看rdt_help.h
		添加checksum函数和验证checksum的函数
		添加了message，packet和myPacket之间转换的函数
		添加了验证seqNum是否在window范围内的函数
		

