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
#define SEP_DELAY "%"
#define SEP_TASKID "~"


#define SEP_TASK "+"
#define SEP_ENDTASK "-"
#define SEP_TASK_ITEM "#"

#define TARGET_ALL -1

#define CMD_CANCEL "CANCEL"

#define SUBNET "150.150.150.0"
#define DEFAULT_PORT 54000
#define ADDRESS_PATH "/home/pi/Desktop/address.txt"

#define LOG_FILENAME "/home/pi/log"

#define MAX_PIS 5
#define TIME_BETWEEN_DISCOVERIES 5000L
#define TIME_BETWEEN_PINGS 3000L
#define PING_TIMEOUT 1000L
#define PING_CHAR "+"

#endif // CONSTANTS_H
