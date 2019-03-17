
#include"lurk_pro.h"
#include<ncurses.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<pthread.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h> 
#include<signal.h>
#include<unistd.h>
unsigned int type;
struct Game G;
struct Connection C;
struct Character Ch;
struct Room R;
struct Accept A;
struct Error E;
struct Message M;
struct Loot L;
struct PvPFight P;
struct Fight F;
struct ChangeRoom Cr;

void exit_graceful(int signal){
	endwin();
	exit(0);
}

void* receive_print(void* arg){
	struct receive_info* prfi = (struct receive_info*)arg;
	ssize_t readsize;
	for(;;){
		read(prfi->sockfd, &type,1);
                if(type == 11)
		{
			readGame(&G, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Inital_Points: %d \n",G.Int_points);
			wprintw(prfi->top, "Stat_Limit: %d \n",G.Stat_limit);
			wprintw(prfi->top, "Game_Description: %s \n",G.Game_des);
			wrefresh(prfi->top);
			memset(G.Game_des, 0, sizeof(G.Game_des));
		}
		else if(type == 13)
		{
			readConnection(&C, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Room_Number: %d \n",C.Conn_Room_num);
			wprintw(prfi->top, "Room_Name: %s \n",C.Conn_Room_name);
			wprintw(prfi->top, "Room_Description: %s \n",C.Conn_Room_des);
			wrefresh(prfi->top);
			memset(C.Conn_Room_name, 0, sizeof(C.Conn_Room_name));
			memset(C.Conn_Room_des, 0, sizeof(C.Conn_Room_des));



		}
		else if(type == 10)
                {
			int alive, join, monster, started;
                        readCharacter(&Ch, prfi);
			wrefresh(prfi->top);
			alive  = 128 & Ch.Flags;
			monster = 32 & Ch.Flags;
			started = 16 & Ch.Flags;
			join = 64 & Ch.Flags;
			wprintw(prfi->top, "----------\n");
			if(monster)
                                wprintw(prfi->top, "--Monster--\n");
                        else
                                wprintw(prfi->top, "--Human--\n");
			wprintw(prfi->top, "----------\n");

			if(alive)
                                wprintw(prfi->top, "Name: %s (ALIVE!)\n",Ch.Char_Name);
                        else
                                wprintw(prfi->top, "Name: %s (DEAD!)\n",Ch.Char_Name);
			wprintw(prfi->top, "HP: %d | ATT: %d | DEF: %d | REG: %d\n",Ch.Health,Ch.Attack,Ch.Defense,Ch.Regen);
			wprintw(prfi->top, "Gold: %d | ",Ch.Gold);
			if(!monster)
				wprintw(prfi->top, "In_Room: %d ",Ch.Curr_Room_Num);
			wprintw(prfi->top, "\nDescription: %s\n",Ch.Char_des);
			if(started &&  !monster)
				wprintw(prfi->top, "Started: YES! | ");
			else if(!started &&  !monster)
                                wprintw(prfi->top, "Started: Not Yet! | ");
			if(join && !monster)
                                wprintw(prfi->top, "Join Battle: Yes!\n");
			else if(!join && !monster)
				wprintw(prfi->top, "Join Battle: No\n");
			wrefresh(prfi->top);
			memset(Ch.Char_Name, 0, sizeof(Ch.Char_Name));
			memset(Ch.Char_des, 0, sizeof(Ch.Char_des));

                }
		else if(type == 9)
		{
			readRoom(&R, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Room_Number: %d | ",R.Room_num);
			wprintw(prfi->top, "Room_Name: %s \n",R.Room_name);
			wprintw(prfi->top, "Room_Description: %s \n",R.Room_des);
			memset(R.Room_name, 0, sizeof(R.Room_name));
			memset(R.Room_des, 0, sizeof(R.Room_des));


		}
		else if(type == 8)
		{
			readAccept(&A,prfi);
			wprintw(prfi->top, "Accepted\n",A.Accepted);
		}
		else if(type == 7)
		{
			readError(&E, prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Error_Code: %d \n",E.Err_code);
			wprintw(prfi->top, "Error_Message: %s \n",E.Err_Message);
			wrefresh(prfi->top);
			memset(E.Err_Message, 0, sizeof(E.Err_Message));

		}
		else if(type == 1)
		{
			readMessage(&M,prfi);
			wrefresh(prfi->top);
			wprintw(prfi->top, "Recipient_Name: %s | ",M.Rec_Name);
			wprintw(prfi->top, "Sender_Name: %s \n",M.Sen_Name);
			wprintw(prfi->top, "Message: %s \n",M.The_Message);
			wrefresh(prfi->top);
			memset(M.Rec_Name, 0, sizeof(M.Rec_Name));
			memset(M.Sen_Name, 0, sizeof(M.Sen_Name));
			memset(M.The_Message, 0, sizeof(M.The_Message));


		} 
		wprintw(prfi->top, "\n");
		wmove(prfi->bottom, LINES/4 - 1, 0);
		wrefresh(prfi->top);
		wrefresh(prfi->bottom);
		
	}
	return 0; 
}

int main(int argc, char ** argv){
	// Usage Information
	if(argc < 3){
		printf("Usage:  %s hostname port\n", argv[0]);
		return 1;
	}

	// Handle Signals
	struct sigaction sa;
	sa.sa_handler = exit_graceful;
	sigaction(SIGINT, &sa, 0);

	// Prepare the network connection, but don't call connect yet
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		goto err;
	short port = atoi(argv[2]);
	struct sockaddr_in connect_addr;
	connect_addr.sin_port = htons(port);
	connect_addr.sin_family = AF_INET;
	struct hostent* entry = gethostbyname(argv[1]);
	if(!entry)
		goto err;
	struct in_addr **addr_list = (struct in_addr**)entry->h_addr_list;
	struct in_addr* c_addr = addr_list[0];
	char* ip_string = inet_ntoa(*c_addr);
	connect_addr.sin_addr = *c_addr;
	
	// Set up curses.  This is probably easier in Python.
	initscr();
	start_color();
	use_default_colors();
	init_pair(1, COLOR_GREEN, -1);
	init_pair(2, COLOR_RED, -1);
	refresh();
	WINDOW* top = newwin(LINES*3/4, COLS, 0, 0);
	WINDOW* bottom = newwin(LINES/4 - 1, COLS, LINES*3/4 + 1, 0);
	refresh();
	wmove(stdscr, LINES*3/4, 0);
	whline(stdscr, ACS_HLINE , COLS);
	wmove(bottom, LINES/4 - 2, 0);

	scrollok(bottom, 1);
	scrollok(top, 1);
	wrefresh(top);
	wrefresh(bottom);
	refresh();

	// The UI is up, let's reassure the user that whatever name they typed resolved to something
	wattron(top, COLOR_PAIR(1));
	wprintw(top, "Connecting to host %s (%s)\n", entry->h_name, ip_string);
	wrefresh(top);
	
	// Actually connect.  It might connect right away, or sit here and hang - depends on how the
	// host is feeling today
	if(connect(sockfd, (struct sockaddr*)&connect_addr, sizeof(struct sockaddr_in)))
		goto err;
	
	// Let the user know we're connected, so they can start doing whatever they do.
	wprintw(top, "Connected\n");
	wrefresh(top);
	wattroff(top, COLOR_PAIR(1));

	// Start the receive thread
	struct receive_info rfi;
	rfi.sockfd = sockfd;
	rfi.top = top;
	rfi.bottom = bottom;
	pthread_t t;
	pthread_create(&t, 0, receive_print, &rfi);
	
	// Get user input.  Ctrl + C is the way out now.
	char input[1024*1024];
	wmove(bottom, LINES/4 - 2, 0);
	size_t length;


	for(;;){
		wprintw(bottom, "Type /commands for help \n");
		wprintw(bottom,"----------------------------------------------------------------------\n");
		wscrl(bottom, 0);
		wrefresh(bottom);
		wgetnstr(bottom, input, 1024*1024-1);


		if(!strcmp(input,"/commands"))
                {	
			wprintw(bottom, "\n");
                        wprintw(bottom, "Type /char to create a character.\n");
			wprintw(bottom, "Type /start to start the game.\n");
			wprintw(bottom, "Type /fight to fight monsters.\n");
			wprintw(bottom, "Type /pvpFight to fight other players.\n");
			wprintw(bottom, "Type /loot to loot from dead players.\n");
			wprintw(bottom, "Type /change to change room.\n");
			wprintw(bottom, "Type /send to sned a message to other player.\n");
			wprintw(bottom, "Type /leave to exit the game.\n");
                }


		else if(!strcmp(input,"/leave"))
                {
                        type = 12;
                        write(sockfd,&type,1);
			endwin();
			exit(0);

                }

		else if(!strcmp(input,"/char"))
                {
                        type = 10;
                        write(sockfd,&type,1);
                        writeCharacter(&Ch,&rfi);
                }
		else if(!strcmp(input,"/start"))
 		{
			type = 6;
                        write(sockfd,&type,1);
		}
		else if(!strcmp(input,"/loot"))
                {
                        type = 5;
                        write(sockfd,&type,1);
			writeLoot(&L,&rfi);

                }
		else if(!strcmp(input,"/pvpFight"))
                {
                        type = 4;
                        write(sockfd,&type,1);
                        writePvPFight(&P,&rfi);

                }
		else if(!strcmp(input,"/fight"))
                {
                        type = 3;
                        write(sockfd,&type,1);

                }
		else if(!strcmp(input,"/change"))
                {
                        type = 2;
                        write(sockfd,&type,1);
			writeChangeRoom(&Cr,&rfi);


                }
		else if(!strcmp(input,"/send"))
                {
                        type = 1;
                        write(sockfd,&type,1);
			writeMessage(&M,&rfi);

                }

		wprintw(bottom, "\n");
		wrefresh(top);
		wrefresh(bottom);
	}

err:
	endwin();
	perror(argv[0]);
	return 1;
}
