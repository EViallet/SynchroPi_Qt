#ifndef CONSTANTS_H
#define CONSTANTS_H

#define UUID "00001111-0000-1000-8000-00805f9b34fb"
#define SEP_PACKET "="
#define SEP_ID "&"
#define SEP_DBG "-"
#define SEP_INT "#"
#define SEP_BOOL "$"
#define SEP_STR "*"
#define SEP_MAC "^"

#define PING_CHAR "!"
#define PING_TIMEOUT 2000
#define PING_DELAY 2000

#define SEP_TASK "+"
#define SEP_ENDTASK "/"
#define SEP_TASK_ITEM "#"

#define TARGET_ALL -1

#define SENDER_BT 0
#define SENDER_TCP 1
#define SENDER_TASKM 2

#define SUBNET "150.150.150.0"
#define DEFAULT_PORT 54000
#define ADDRESS_PATH "/home/pi/Desktop/address.txt"

#define LOG_FILENAME "/home/pi/log"

#define MAX_PIS 4

extern bool DEBUG_MODE;

#endif // CONSTANTS_H
