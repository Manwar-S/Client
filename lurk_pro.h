#ifndef LURK_PRO_H
#define LURK_PRO_H



#include<ncurses.h>
#include<stdio.h>
#include<stdlib.h>
// moved it here so all the struct togather, also easier to write funcs.
struct receive_info {
        int sockfd;
        WINDOW* top;
        WINDOW* bottom;
	WINDOW* right;
};

//1
struct Message{
	unsigned short Message_Len;
	char Rec_Name[32];
	char Sen_Name[32];
	char The_Message[1024];
};
//2
struct ChangeRoom{
	unsigned short change_room;
};
//3
struct Fight{
};
//4
struct PvPFight{
	char Fighter_Name[100];
};

//5
struct Loot{
	char Loot_Name[100];
};

//6
struct Start{
	
};
//7
struct Error{ 
	unsigned short Err_code;
	unsigned short Err_Message_len;
	char Err_Message[1024*1024];

};
//8
struct Accept{
	unsigned int Accepted;
};
//9
struct Room{ 
	unsigned short Room_num;
	char Room_name[32];
	unsigned short Room_des_len;
	char Room_des[1024];

};

//10
struct Character{ 
	char Char_Name[100];
	unsigned char  Flags;
	unsigned short Attack;
	unsigned short Defense;
	unsigned short Regen;
	short Health;
	unsigned short Gold;
	unsigned short Curr_Room_Num;
	unsigned short Char_Des_len;
	char Char_des[1024];
};

//11
struct Game{ 
	unsigned short Int_points;
	unsigned short Stat_limit;
	unsigned short Game_Des_len;
	char Game_des[1024];
};
//12
struct Leave{
	
};

//13
struct Connection{ 
	unsigned short Conn_Room_num;
	char Conn_Room_name[32];
	unsigned short Conn_Room_des_len;
	char Conn_Room_des[1024];

};

// read type 13
void readConnection(struct Connection *P, struct receive_info *skt);

// read type 11
void readGame(struct Game *P, struct receive_info *skt);

// read type 10
void readCharacter(struct Character *P, struct receive_info *skt);

// read type 9
void readRoom(struct Room *P, struct receive_info *skt);

// read type 8
void readAccept(struct Accept *P, struct receive_info *skt);

// read type 7
void readError(struct Error *P, struct receive_info *skt);

// read type 1
void readMessage(struct Message *P, struct receive_info *skt);


//___________________________________________________________________________

//write type 12
void writeLeave(struct Leave *P, struct receive_info *skt);

//write type 10
void writeCharacter(struct Character *P, struct receive_info *skt);

//write type 8
void writeAccept(struct Accept *P, struct receive_info *skt);

//write type 6
void writeStart(struct Start *P, struct receive_info *skt);

//write type 5
void writeLoot(struct Loot *P, struct receive_info *skt);

//write type 4
void writePvPFight(struct PvPFight *P, struct receive_info *skt);

//write type 3
void writeFight(struct Fight *P, struct receive_info *skt);

//write type 2
void writeChangeRoom(struct ChangeRoom *P, struct receive_info *skt);

//write type 1
void writeMessage(struct Message *P, struct receive_info *skt);
#endif
 
