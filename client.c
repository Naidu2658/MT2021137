#include <sys/socket.h>
 #include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>  
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdbool.h>
#include<stdlib.h>
#include "constants.h"
#include "structs.h"   //linking user definded files

int main() 
{
	struct clientCred cred;
    struct clientDetails clientDet;
	int sd, valread; 
	struct sockaddr_in serv_addr; 
	
	sd= socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	
	connect(sd, (void *)&serv_addr, sizeof(serv_addr));
	printf("Connected to Server\n");
	
	int login_choice;
	int userid; 
	int password;
	char loginStatus[50];	
	
	printf("Login as:\n");
	printf("1.Agent\n2.Customer\n3.Admin\n");	

	scanf("%d",&login_choice);
	
	printf("Enter userid:\n");
	scanf("%d",&userid);
	
	printf("Enter Password:\n");
	scanf("%d",&password);
	
	cred.userid = userid;
	cred.password = password;
	
	clientDet.clientType  = login_choice;
	clientDet.cred = cred;
	
	write(sd , &clientDet , sizeof(clientDet) ); 
	read(sd, &loginStatus , 50);
	
	
	if(strcmp(loginStatus,"success") == 0)
	{
		
        printf("Login was successfull\n");
        int options;
        int delindex;
        int updateindex;
        int updatedSeats;
       	int UserId;
        int train_no;
        int no_of_seats;	
	int password;
	int accType;
	int trainNo;
	int num_of_seats;
	int newpass;
	int newaccType;
	int newtotalSeats;

	struct train train;
        struct bookingRequest breq;
        struct clientRequest cr;
        struct serverResponse response;


	struct account newuser;
	struct account modUser;
	struct train modTrain;

		switch(login_choice)
		{
		         case 1:
                         	while(1)
                         	{
                                	printf("\n");
	                        	printf("1.Search\n2.Book ticket\n3.Cancel Ticket\n4.View Previous Bookings\n5.Update Booking\n6.Log out");
                                  	printf("\nEnter choice:");
	                            	scanf("%d",&options);
	                                 while ((getchar()) != '\n');	
		
		
	                          	switch(options)
	                                   	{
				                   case SEARCH:
				
			                        	printf("Enter train number:\n");

                       			                         	                                              scanf("%d",&train_no);
				
			                          	train.trainNo = train_no;
			                               	breq.trainNo = train_no;
			
    			                        	cr.command = SEARCH;	                                                       cr.tr = train;
			                         	cr.bookReq = breq;
				                        write(sd , &cr , sizeof(cr));
			                            	char buff[10];
				                        read(sd,&response ,sizeof(struct serverResponse));
							if(response.commandStatus==1)
			                             	{
                      					printf("Train found\n");
		               		        	printf("Train number %d\n",response.tr.trainNo);
			                    		printf("Available seats %d\n",response.tr.totalSeats );
				                        }
			                         	else
			                                 	{
					                printf("Train not found\n");
			                                       	}
		                                		break;
				
		                 		case BOOK_TICKET:
				
				
				                    printf("Enter train number:\n");
			                         	scanf("%d",&train_no);
			                         	printf("Enter number of seats to be booked:\n");
		                            		scanf("%d",&no_of_seats);
				
		                            		train.trainNo = train_no;
			                          	breq.trainNo = train_no;
				                             breq.number_of_seats = no_of_seats;
				
			
			                         	cr.command = BOOK_TICKET; //book command
		                        		cr.tr = train;
		                           		cr.bookReq = breq;
			                         	cr.uid = userid;
				
			                         	write(sd , &cr , sizeof(cr));
			                         	read(sd,&response ,sizeof(struct serverResponse));
				
		                           		if(response.commandStatus==1)
		                                  {
					if(response.tr.totalSeats - response.tr.totalBooked >= no_of_seats)
					{
						if(response.bookingStatus == 1)
						{
							 printf("Booking successful\n");
						}
						else	{
							printf("Booking error\n");
						}
					}
					else
					{
						printf("seats are full\n");
					}
				}
				else
				{
					printf("Train not found\n");
				}
				
				
				
				break;
				
				case CANCEL_TICKET:
				
				cr.uid = userid;
				cr.command = CANCEL_TICKET;
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				
				printf("Bookings = %d \n",response.acc.totalBookings);
				int t = 0;
				int b=1;
				
				while(t<response.acc.totalBookings)
				{
					printf("===booking number %d===========\n",b);
					printf("Train no: %d\n",response.acc.booked[t].trainNo);
					printf("Number of seats: %d\n\n",response.acc.booked[t].number_of_seats);
					t++;
					b++;
				}
				
				printf("\nEnter the booking number to be deleted:");
				scanf("%d",&delindex);
				
				cr.deleteIndex = delindex;
				
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));

				if(response.commandStatus == 1) {	
					printf("Booking Deleted Successfully\n");
				}
				else
				{
					printf("Booking Deletion Failed\n");

				}
				
				break;
				
				case PREV_BOOKING:
				
				cr.uid = userid;
				cr.command = PREV_BOOKING;
				send(sd , &cr , sizeof(cr),0);
				read(sd,&response ,sizeof(struct serverResponse));
				
				printf("Total Bookings = %d \n",response.acc.totalBookings);
				 int tot = response.acc.totalBookings;
				 int bi=1;
				while(tot>0)
				{
					printf("===========Booking number %d===========\n",bi);	
					printf("Train no: %d\n",response.acc.booked[tot-1].trainNo);
					printf("Number of seats: %d\n\n",response.acc.booked[tot-1].number_of_seats);
					tot--;
					bi++;
				}
				
				break;
				
				case UPDATE_BOOKING:
				
				cr.uid = userid;
				cr.command = UPDATE_BOOKING;
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				
				printf("Bookings = %d \n",response.acc.totalBookings);
				int ti = 0;
				int booki=1;
				
				while(ti<response.acc.totalBookings)
				{
					printf("===========Booking number %d===========\n",booki);
					printf("Train no: %d\n",response.acc.booked[ti].trainNo);
					printf("Number of seats: %d\n\n",response.acc.booked[ti].number_of_seats);
					ti++;
					booki++;
				}
				
				printf("\nEnter the booking number to be updated:");
				scanf("%d",&updateindex);
				
				printf("\nEnter the new number of seats:");
				scanf("%d",&updatedSeats);
				
				
				cr.updateIndex = updateindex;
				cr.updatedSeats = updatedSeats;

				
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));

				if(response.commandStatus == 1) {	
					printf("Booking Updated Successfully\n");
				}
				else
				{
					printf("Booking Updation Failed\n");

				}
				
				break;
			 
			 case LOGOUT:
				
				cr.command = LOGOUT;
				send(sd , &cr , sizeof(cr),0);
			        exit(0);        
			    break;
			
			
		}
	
	
	}      
			case 2:
			
	while(1)
	{
	printf("----------------------------------------------------------------------------------------------------------------------------------\n");
		printf("1.Search\n2.Book ticket\n3.Cancel Ticket\n4.View Previous Bookings\n5.Update Booking\n6.Log out");
        	printf("\nEnter choice:");
		scanf("%d",&options);		
		
		 while ((getchar()) != '\n');

		
		
		switch(options)
		{
				case SEARCH:
				
				printf("Enter train number:\n");
				scanf("%d",&train_no);
				
				train.trainNo = train_no;
				breq.trainNo = train_no;
				
				cr.command = SEARCH; //search command
				cr.tr = train;
				cr.bookReq = breq;
				write(sd , &cr , sizeof(cr));
				char buff[10];
				read(sd,&response ,sizeof(struct serverResponse));
				if(response.commandStatus==1)
				{
					printf("Train found\n");
					printf("Train number %d\n",response.tr.trainNo);
					printf("Available seats %d\n",response.tr.totalSeats );
				}
				else
				{
					printf("Train not found\n");
				}
				break;
				
				case BOOK_TICKET:
				
				printf("Enter train number:\n");
				scanf("%d",&train_no);
				printf("Enter number of seats to be booked:\n");
				scanf("%d",&no_of_seats);
				printf("Enter the user id of customer:\n");
				scanf("%d",&UserId);
				
				train.trainNo = train_no;
				breq.trainNo = train_no;
				breq.number_of_seats = no_of_seats;
				
				
				cr.command = BOOK_TICKET; //book command
				cr.tr = train;
				cr.bookReq = breq;
				cr.uid = UserId;
				
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				
				if(response.commandStatus==1)
				{
					if(response.tr.totalSeats - response.tr.totalBooked >= no_of_seats)
					{
						if(response.bookingStatus == 1)
						{
							 printf("Booking successful!!!\n");
						}
						else
						{
							printf("Booking error\n");
						}
					}
					else
					{
						printf("No more available seats\n");
					}
					
				}
				else
				{
					printf("Train not found\n");
				}	
				
				break;
				
				case CANCEL_TICKET:
				
				printf("Enter the user id of customer:\n");
				scanf("%d",&UserId);
				cr.uid = UserId;
				cr.command = CANCEL_TICKET;
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				
				printf("Bookings = %d \n",response.acc.totalBookings);
				int t = 0;
				int b=1;
				
				while(t<response.acc.totalBookings)
				{
					printf("===========Booking number %d===========\n",b);
					printf("Train no: %d\n",response.acc.booked[t].trainNo);
					printf("Number of seats: %d\n\n",response.acc.booked[t].number_of_seats);
					t++;
					b++;
				}
				
				printf("\nEnter the booking number to be deleted:");
				scanf("%d",&delindex);
				
				cr.deleteIndex = delindex;
				
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));

				if(response.commandStatus == 1) {	
					printf("Booking Deleted Successfully\n");
				}
				else
				{
					printf("Booking Deletion Failed\n");

				}
				
				break;
				
				case PREV_BOOKING:
				
				printf("Enter the user id of customer:\n");
				scanf("%d",&UserId);
				cr.uid = UserId;
				cr.command = PREV_BOOKING;
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				
				printf("Total Bookings = %d \n",response.acc.totalBookings);
				 int tot = response.acc.totalBookings;
				 int bi=1;
				while(tot>0)
				{
					printf("===========Booking number %d===========\n",bi);	
					printf("Train no: %d\n",response.acc.booked[tot-1].trainNo);
					printf("Number of seats: %d\n\n",response.acc.booked[tot-1].number_of_seats);
					tot--;
					bi++;
				}
				
				break;
				
				case UPDATE_BOOKING:
				
				printf("Enter the user id of customer:\n");
				scanf("%d",&UserId);
				cr.uid = UserId;
				cr.command = UPDATE_BOOKING;
				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				
				printf("Bookings = %d \n",response.acc.totalBookings);
				int ti = 0;
				int booki=1;
				
				while(ti<response.acc.totalBookings)
				{
					printf("===========Booking number %d===========\n",booki);
					printf("Train no: %d\n",response.acc.booked[ti].trainNo);
					printf("Number of seats: %d\n\n",response.acc.booked[ti].number_of_seats);
					ti++;
					booki++;
				}
				
				printf("\nEnter the booking number to be updated:");
				scanf("%d",&updateindex);
				
				printf("\nEnter the new number of seats:");
				scanf("%d",&updatedSeats);
				
				cr.updateIndex = updateindex;
				cr.updatedSeats = updatedSeats;

				write(sd , &cr , sizeof(cr));
				read(sd,&response ,sizeof(struct serverResponse));
				if(response.commandStatus == 1) {	
					printf("Booking Updated Successfully\n");
				}
				else
				{
					printf("train was deleted\n");

				}
				break;
			 case LOGOUT:
				cr.command = LOGOUT;
				write(sd , &cr , sizeof(cr));
			        exit(1);
			    break;	
		}
	}
			break;	
          		case 3:
		
	while(1)
	{
	printf("----------------------------------------------------------------------------------------------------------------------------------\n");
		printf("1.Search User\n2.Add User\n3.Delete User\n4.Modify User\n5.Search Train\n6.Add Train\n7.Delete Train\n8.Modify Train\n9.Logout");
        	printf("\nEnter choice:");
		scanf("%d",&options);		
		while ((getchar()) != '\n');

		switch(options)
		{
			case SEARCH_USER:
			
				printf("Enter User ID:\n");
				scanf("%d",&UserId);
				
				cr.uid = UserId;
				
				cr.command = SEARCH_USER; //search command
				
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));

				if(response.commandStatus==1)
				{
					printf("User Found\n");
				}
				else
				{
					printf("User not found\n");
				}
				break;
			
			
			case SEARCH_TRAIN:
			
				printf("Enter train number:\n");
				scanf("%d",&train_no);
				
				
				train.trainNo = train_no;
				
			
				cr.command = SEARCH_TRAIN; //search command
				cr.tr = train;
				
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				if(response.commandStatus==1)
				{
					printf("Train found\n");
					printf("Train number %d\n",response.tr.trainNo);
					printf("Available seats %d\n",response.tr.totalSeats );
				}
				else
				{
					printf("Train not found\n");
				}
				break;
				
				case ADD_USER:
				
				printf("Enter User ID:\n");
				scanf("%d",&UserId); 
				printf("Enter Password:\n");
				scanf("%d",&password);
				printf("Enter account type(Agent (1)  Normal User(2) Admin(3) ):\n");
				scanf("%d",&accType);
				
				newuser.uid = UserId;
				newuser.pin = password;
				newuser.accType = accType;
				newuser.totalBookings=0;
				
				cr.uid = UserId;
				cr.newuser = newuser;
				cr.command = ADD_USER;
				
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				if(response.newuserStatus ==1)
				{
					if(response.commandStatus == 1)
					{
						printf("User added successfull\n");
					}
					else
					{
						printf("New user adding failed\n");
					}
				}
				else
				{
					printf("User already exists\n");
				}
				
				
				break;
				case DELETE_USER:
		
				printf("Enter User ID:\n");
				scanf("%d",&UserId); 
				
				cr.uid = UserId;
				cr.command = DELETE_USER;
		
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				
				if(response.newuserStatus ==0)
				{
					if(response.commandStatus == 1)
					{
						printf("User deleted successfully\n");
					}
					else
					{
						printf("User deletion failed\n");
					}
				}
				else
				{
					printf("User does not exists\n");
				}
		
				break;
				
				case ADD_TRAIN:
				
				printf("Enter Train number:\n");
				scanf("%d",&trainNo); 
				printf("Enter total number of seats:\n");
				scanf("%d",&num_of_seats);
				
				
				train.trainNo = trainNo;
				train.totalSeats = num_of_seats;
				train.totalBooked =0;
				
				cr.tr = train;
				cr.trainNo = trainNo;
				cr.command = ADD_TRAIN;
				
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				if(response.newtrainStatus ==1)
				{
					if(response.commandStatus == 1)
					{
						printf("Train added successfully\n");
					}
					else
					{
						printf("New Train adding failed\n");
					}
				}
				else
				{
					printf("Train already exists\n");
				}
								
				break;
				
				case DELETE_TRAIN:
					
				printf("Enter Train Number:\n");
				scanf("%d",&trainNo); 
				
				cr.trainNo = trainNo;
				cr.command = DELETE_TRAIN;
		
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				
				if(response.newtrainStatus ==0)
				{
					if(response.commandStatus == 1)
					{
						printf("Train deleted successfully!!!\n");
					}
					else
					{
						printf("Train deletion failed!!!\n");
					}
				}
				else
				{
					printf("Train does not exists!!!\n");
				}
	
				break;
				
				case MODIFY_USER:
					
				printf("Enter User ID:\n");
				scanf("%d",&UserId); 
				
				cr.uid = UserId;
				cr.command = MODIFY_USER;
		
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				
				if(response.newuserStatus ==0)
				{
					printf("User Id: %d\n",response.acc.uid);
					printf("Password: %d\n",response.acc.pin);
					printf("Account type: %d\n",response.acc.accType);
					
					modUser = response.acc;
					printf("Enter the new data:\n");
					printf("Enter new password \n");
					scanf("%d",&newpass);
					printf("Enter new account type\n");
					scanf("%d",&newaccType);
					
					modUser.pin = newpass;
					modUser.accType = newaccType;
					
					cr.newuser= modUser;
					send(sd , &cr , sizeof(cr),0);
					
					read(sd,&response ,sizeof(struct serverResponse));
					
					if(response.commandStatus == 1)
					{
						printf("User modified successfully\n");
					}
					else
					{
						printf("User modification failled\n");
				}
					
				}
				else
				{
					printf("User does not exists");
				}
				
				break;
				
				case MODIFY_TRAIN:
				
				printf("Enter Train number:\n");
				scanf("%d",&train_no); 
				
				cr.trainNo = train_no;
				cr.command = MODIFY_TRAIN;
		
				write(sd , &cr , sizeof(cr));
				
				read(sd,&response ,sizeof(struct serverResponse));
				
				if(response.newtrainStatus ==0)
				{
					printf("Train number: %d\n",response.tr.trainNo);
					printf("Total Seats: %d\n",response.tr.totalSeats);
					printf("Total booked: %d\n",response.tr.totalBooked);
					
					modTrain = response.tr;
					printf("Enter the new data:\n");
					printf("Enter new total seats\n");
					scanf("%d",&newtotalSeats);
					
					modTrain.totalSeats = newtotalSeats;
					
					cr.tr= modTrain;
					write(sd , &cr , sizeof(cr));
					
					read(sd,&response ,sizeof(struct serverResponse));
					
					if(response.commandStatus == 1)
					{
						printf("Train modified successfully\n");
					}
					else
					{
						printf("Train modification failled\n");

					}
					
				}
				else
				{
					printf("Train does not exists");
				}
				
				break;
				
				case LOGOUT_ADMIN:
				
				cr.command = LOGOUT_ADMIN;
				write(sd , &cr , sizeof(cr));
			     exit(1);
			    break;
		}
	}
			break;
                  default:
	             printf("invalid choice\n");
                     exit(0);		     
	}
	}
	else
	{
		printf("not able to login\n");
	}
return 0;	 
} 
