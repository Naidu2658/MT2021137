struct clientCred
{
	int userid;
	int password;
	
};

struct clientDetails
{
	struct clientCred cred;
	int clientType;
	
};

struct train{
	int  totalBooked;
	int  trainNo;
	int totalSeats;
};

struct bookingRequest{
	int trainNo;
	int number_of_seats;
	int uid;
};
struct bookingDetails{
	int trainNo;
	int number_of_seats;	
};
struct account{
	int uid;
	int pin;
	int accType;
	struct bookingDetails booked[MAX_BOOKING];
	int totalBookings;
};
struct clientRequest{
	int command;
	int uid;
	int trainNo;
	int deleteIndex;
	int updateIndex;
	int updatedSeats;
	struct train tr;
	struct bookingRequest bookReq;
	struct account newuser;
};

struct serverResponse{
	int commandStatus;
	int bookingStatus;
	int newuserStatus;
	int newtrainStatus;
	struct account acc;
	struct train tr;
};





