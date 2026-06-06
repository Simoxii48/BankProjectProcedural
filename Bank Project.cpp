#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <string.h>
#include <fstream>
#include <istream>
#include <cstdio>
#include <cctype>
#include <limits>
#include <conio.h>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <corecrt.h>
#pragma warning(disable:4996) // to disable warning of using localtime() function which is not thread safe, we can use localtime_s() instead but it is only supported in C++11 and later and I want to make sure that the code is compatible with older versions of C++ as well

#define NOMINMAX
#include <windows.h>

using namespace std;

const string clientsFileName = "clients.txt";
const string usersFileName = "users.txt";
const string transactionsLogFileName = "transactions.txt";
const string transferFileName = "transfers.txt";
const string actionLogsFileName = "actionsLog.txt";
constexpr const char* delim = "#//#";
const int FULL_PERMISSION = ~0;

enum enMenu
{
	ShowClientsList = 1,
	AddNewClient = 2,
	DeleteClient = 3,
	UpdateClient = 4,
	FindClient = 5,
	transactions = 6,
	ManageUsers = 7,
	Logout = 8
};

enum enMenuTransactions
{
	Deposit = 1,
	Withdraw = 2,
	Transfer = 3,
	TotalBalances = 4,
	TransactionsLog = 5,
	TransfersLog = 6,
	MainMenu = 7
};

enum enManageUsersMenu
{
	ListUsers = 1,
	AddNewUser = 2,
	DeleteUser = 3,
	UpdateUser = 4,
	FindUser = 5,
	RevokeUserPermissions = 6,
	LockUser = 7,
	UnlockUser = 8,
	ShowActionLogs = 9,
	ReturnMainMenu = 10
};

enum enClientPermissions
{
	ShowClientsListPer = 1 << 0,   //     1  mean 0000 0001 in binary
	AddNewClientPer = 1 << 1,     //      2  mean 0000 0010 in binary
	DeleteClientPer = 1 << 2,    //       4  mean 0000 0100 in binary
	UpdateClientPer = 1 << 3,   //        8  mean 0000 1000 in binary
	FindClientPer = 1 << 4,    //         16 mean 0001 0000 in binary
	TransactionsPer = 1 << 5, //          32 mean 0010 0000 in binary
	ManageUsersPer = 1 << 6, //           64 mean 0100 0000 in binary
};

enum enManageUsersPermissions
{
	ListUsersPer = 1 << 0,
	AddNewUserPer = 1 << 1,
	DeleteUserPer = 1 << 2,
	UpdateUserPer = 1 << 3,
	FindUserPer = 1 << 4,
};

enum enTransactionsPermissions
{
	DepositPer = 1 << 0,
	WithdrawPer = 1 << 1,
	TransferPer = 1 << 2,
	TotalBalancesPer = 1 << 3,
};

enum enpermissionsType
{
	ClientPermissions = 1,
	TransactionsPermissions = 2,
	ManageUsersPermissions = 3,
};

struct stClient
{
	string accNumber = "";
	string pinCode = "";
	string fullName = "";
	string phoneNumber = "";
	double accBalance = 0;
	bool isMarkedForDelete = false;
};

struct stUser
{
	string userName = "";
	string password = "";
	int permissions = 0;
	int transactionsPermissions = 0;
	int manageUsersPermissions = 0;
	bool isLocked = false;
};

// to combach here to fix trxID
struct stTransaction
{
	string trxID = "";
	string accNumber = "";
	string transactionType = "";
	double amount = 0.f;
	string transactionDateTime = "";
	string performedByUser = "";
};

struct stTransfer
{
	string TransferID = "";
	string fromAccNum = "";
	string toAccNum = "";
	double amount = 0;
	string performedByUser = "";
	string transferDateTime = "";
};

struct stActionLog
{
	string userName = "";
	string action = "";
	string actionDateTime = "";
};

struct stSystem
{
	vector<stClient> vAllClients{};
	vector<stUser> vAllUsers{};
	vector<stTransaction> vAllTransactions{};
	vector<stTransfer> vAllTransfers{};
	vector<stActionLog> vAllActionLogs{};
};

bool hasPermission(int userPermission, int required)
{
	return (userPermission & required) != 0;
}

// Functions declarations
vector<stClient> getAllClients();
int findUserIndex(const vector<stUser>& vAllUsers, string username);
void startBankApp(stSystem& system);
char readYesNo(string message);
bool isAdmin(const stUser& user);
void insertLogTransaction(vector<stTransaction>& vAllTransactions);
string getCurrentDateTime();
stActionLog createActionLog(const string& userName, const string& action);
void insertActionLog(const stActionLog& log);
vector<stActionLog>getAllActionLogs();
void insertTransfers(const vector<stTransfer>& vAllTransfers);

int validateINTTypeAndRange(int from, int to)
{
	int choice = 0;

	while (true)
	{
		if (!(cin >> choice))
		{
			cout << right << setw(5) << " " << "Invalid Datatype, Please enter a number : ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			continue;
		}
		
		if (choice < from || choice > to)
		{
			cout << right << setw(5) << " " << "Invalid Range, Please enter a number from " << from << " to " << to << " : ";
			continue;
		}
		else
			break;
	}

	return choice;
}

int randomNumber(int from, int to)
{
	return rand() % (to - from + 1) + from;
}

string generateSimpleGUID()
{
	string GUID = "";

	for (int i = 1;i <= 10;i++)
		GUID += char(randomNumber(48, 57));

	return GUID;
}

enMenu readUserChoice(const stUser& connectedUser)
{
	int choice = 0;

	cout << right << setw(5) << " " << "Your Choice : ";
	choice = validateINTTypeAndRange((int)enMenu::ShowClientsList, (int)enMenu::Logout);

	return (enMenu)choice;
}

enClientPermissions readRevokedClientPermission()
{
	int choice = 0;

	cout << endl << right << setw(5) << " " << "Which permission do you want to revoke : " << endl;
	cout << right << setw(5) << " " << "[1] Show Clients List Permission." << endl;
	cout << right << setw(5) << " " << "[2] Add New Client Permission." << endl;
	cout << right << setw(5) << " " << "[3] Delete Client Permission." << endl;
	cout << right << setw(5) << " " << "[4] Update Client Permission." << endl;
	cout << right << setw(5) << " " << "[5] Find Client Permission.\n" << endl;
	cout << right << setw(5) << " " << "Your Choice : ";

	choice = validateINTTypeAndRange(1, 5);

	return (enClientPermissions)(1 << (choice - 1));
}

enTransactionsPermissions readRevokedTransactionsPermission()
{
	int choice = 0;
	
	cout << endl << right << setw(5) << " " << "Which permission do you want to revoke : " << endl;
	cout << right << setw(5) << " " << "[1] Deposit Permission." << endl;
	cout << right << setw(5) << " " << "[2] Withdraw Permission." << endl;
	cout << right << setw(5) << " " << "[3] Transfer Permission." << endl;
	cout << right << setw(5) << " " << "[4] Total Balances Permission.\n" << endl;
	cout << right << setw(5) << " " << "Your Choice : ";

	choice = validateINTTypeAndRange(1, 4);

	return (enTransactionsPermissions)(1 << (choice - 1));
}

enManageUsersPermissions readRevokedManageUsersPermission()
{
	int choice = 0;

	cout << endl << right << setw(5) << " " << "Which permission do you want to revoke : " << endl;

	cout << right << setw(5) << " " << "[1] List Users Permission." << endl;
	cout << right << setw(5) << " " << "[2] Add User Permission." << endl;
	cout << right << setw(5) << " " << "[3] Delete User Permission." << endl;
	cout << right << setw(5) << " " << "[4] Update User Permission." << endl;
	cout << right << setw(5) << " " << "[5] Find User Permission.\n" << endl;
	cout << right << setw(5) << " " << "Your Choice : ";

	choice = validateINTTypeAndRange(1, 5);

	return (enManageUsersPermissions)(1 << (choice - 1));
}

bool revokeClientPermission(stUser& user, enClientPermissions permission)
{
	// To revoke a permission, we can use bitwise AND operator with the negation of the permission we want to revoke
	user.permissions &= ~permission;
	return true;
}

bool revokeTrxPermission(stUser& user, enTransactionsPermissions permission)
{
	user.transactionsPermissions &= ~permission;
	return true;
}

bool revokeManageUsersPermission(stUser& user, enManageUsersPermissions permission)
{
	user.manageUsersPermissions &= ~permission;
	return true;
}

bool revokePermission(stUser& user, enpermissionsType type)
{
	switch (type)
	{
	case enpermissionsType::ClientPermissions:
		revokeClientPermission(user, readRevokedClientPermission());
		break;
	case enpermissionsType::TransactionsPermissions:
		revokeTrxPermission(user, readRevokedTransactionsPermission());
		break;
	case enpermissionsType::ManageUsersPermissions:
		revokeManageUsersPermission(user, readRevokedManageUsersPermission());
		break;
	default:
		return false;
	}
	return true;
}

void lockUser(stUser& user)
{
	if (user.userName == "Admin")
	{
		cout << right << setw(5) << " " << "Admin user cannot be locked." << endl;
		return;
	}

	user.isLocked = true;
}

void unlockUser(stUser& user)
{
	if (user.userName == "Admin")
	{
		cout << right << setw(5) << " " << "Admin user cannot be unlocked because it cannot be locked." << endl;
		return;
	}

	user.isLocked = false;
}

enManageUsersMenu readManageUsersMenuChoice()
{
	cout << right << setw(5) << " " << "Your choice : ";
	int choice = validateINTTypeAndRange((int)enManageUsersMenu::ListUsers, (int)enManageUsersMenu::ReturnMainMenu);

	return (enManageUsersMenu)choice;
}

enMenuTransactions readTransactionsMenuChoice()
{
	cout << right << setw(5) << " " << "Your choice : ";

	int choice = validateINTTypeAndRange((int)enMenuTransactions::Deposit, (int)enMenuTransactions::MainMenu);

	return (enMenuTransactions)choice;
}

string readString(const string& message, bool isPassword = false)
{   
	string line = ""; 

	if (isPassword)
	{
		char c;
		cout << right << setw(5) << " " << message;
		while ((c = _getch()) != '\r') // Enter key is pressed
		{
			if (c == '\b') // Backspace key is pressed
			{
				if (!line.empty()) // Check if there is something to delete
				{
					line.pop_back(); // Remove the last character from the line
					cout << "\b \b"; // for deleting the lest input 
				}
			}
			else if(isprint(c))
			{
				line.push_back(c); // Append the character to the line
				cout << "*";
			}
		}

		return line;
	}
	
	cout << endl << right << setw(5) << " " << message; 
	getline(cin >> ws, line); 
	
	return line; 
}

string hashPassword(string pass,int key)
{
	transform(pass.begin(), pass.end(), pass.begin(), [key](unsigned char c) {return c + char(key);});
	return pass;
}

string unHashPass(string hashed, int key)
{
	transform(hashed.begin(), hashed.end(), hashed.begin(), [key](unsigned char c) {return c - char(key);});
	return hashed;
}

bool isAdmin(const stUser& user)
{
	return user.userName == "Admin";
}

void printConnectedUser(const stUser& user)
{
	cout << right << setw(5) << " " << "Connected User : " << user.userName << endl;
}

void printMainMenu(const vector<stUser>& vAllUsers, const int& userConnectedIdx)
{
	printConnectedUser(vAllUsers[userConnectedIdx]);
   
	cout << right << setw(5) << " " << "============================================================" << endl;
	cout << right << setw(22) << " " << "Main Menu Screen" << endl;
	cout << right << setw(5) << " " << "============================================================" << endl;
	cout << right << setw(5) << " " << "[1] Show Clients List." << endl;
	cout << right << setw(5) << " " << "[2] Add New Client." << endl;
	cout << right << setw(5) << " " << "[3] Delete Client." << endl;
	cout << right << setw(5) << " " << "[4] Update Client." << endl;
	cout << right << setw(5) << " " << "[5] Find Client." << endl;
	cout << right << setw(5) << " " << "[6] Transactions." << endl;
	cout << right << setw(5) << " " << "[7] Manage Users." << endl;
	cout << right << setw(5) << " " << "[8] Logout" << endl;
	cout << right << setw(5) << " " << "============================================================" << endl;
}

void pressKey()
{
	cout << endl << right << setw(5) << " "; 
	system("pause");
	
	/*<< "Press Any Key To Return To Main Menu ...";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	cin.get();*/
}

void clearScreen()
{
	system("cls");
}

void printAccessDeniedScreen()
{
	clearScreen();
	
	cout << right << setw(5) << " " << "============================================================" << endl;
	cout << right << setw(22) << " " << "Access Denied" << endl;
	cout << right << setw(5) << " " << "============================================================" << endl;
	cout << endl << right << setw(5) << " " << "You don't have permissions to access this feature." << endl;
	cout << right << setw(5) << " " << "Please contact the administrator for more information." << endl << endl;
	cout << right << setw(5) << " " << "============================================================" << endl;
	
	pressKey();
}

void printAllClientsHeader()
{
	cout << right << setw(5) << " " << "=====================================================================================" << endl;
	cout << right << setw(6) << " ";

	cout << left << setw(15) << "Account Number" << "|" << " ";
	cout << left << setw(9) << "Pin Code" << "|" << " ";
	cout << left << setw(20) << "Account Holder Name" << "|" << " ";
	cout << left << setw(13) << "Phone Number" << "|" << " ";
	cout << left << setw(15) << "Account Balance" << " " << endl;

	cout << right << setw(5) << " " << "=====================================================================================" << endl;
}

void printClientData(const stClient& c)
{
	cout << right << setw(6) << " ";

	cout << left << setw(15) << c.accNumber << "|" << " ";
	cout << left << setw(9) << c.pinCode << "|" << " ";
	cout << left << setw(20) << c.fullName << "|" << " ";
	cout << left << setw(13) << c.phoneNumber << "|" << " $";
	cout << left << setw(15) << fixed << setprecision(2) << c.accBalance << defaultfloat  <<endl;
}

void showAllClients(const stSystem& system, const int& userConnectedIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConnectedIdx].userName, "Show All Clients"));
	if (!(system.vAllUsers[userConnectedIdx].permissions & (int)enClientPermissions::ShowClientsListPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(system.vAllUsers[userConnectedIdx]);
	printAllClientsHeader();

	for (const stClient& c : system.vAllClients)
		printClientData(c);

	cout << right << setw(5) << " " << "=====================================================================================" << endl;
	cout << right << setw(5) << " " << "Total Numbers Of Client(s) is : " << (int)system.vAllClients.size() << endl << endl;

	pressKey();
	clearScreen();
}

int findClientIndex(const vector<stClient>& vAllClients,const string& accNum)
{
	if (vAllClients.empty())
		return -1;

	for (size_t i = 0;i < vAllClients.size();i++)
	{
		if (vAllClients[i].accNumber == accNum)
			return static_cast<int>(i);
	}

	return -1;
}

void printAddNewClientHeader()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Add New Client Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

string convertClientDataToStringRecord(const stClient& c)
{
	string record = "";

	record += c.accNumber + delim;
	record += c.pinCode + delim;
	record += c.fullName + delim;
	record += c.phoneNumber + delim;
	record += to_string(c.accBalance);

	return record;
}

void insertClients(const vector<stClient>& vAllClients)
{
	ofstream myFile(clientsFileName, std::ios::out);

	if (myFile.is_open())
	{
		string recordLine = "";

		for (const stClient& c : vAllClients)
		{
			if (!c.isMarkedForDelete)
			{
				recordLine = convertClientDataToStringRecord(c);
				myFile << recordLine << endl;
			}
		}
	}
}

stClient readClient(string& accNum)
{
	stClient client{};

	client.accNumber = accNum;
	client.pinCode = readString("Please enter your pin code : ");
	client.fullName = readString("Please enter your fullname : ");
	client.phoneNumber = readString("Please enter your phone number : ");
	
	cout << endl << right << setw(5) << " " << "Please enter account balance : ";
	
	while (true)
	{
		if (!(cin >> client.accBalance) || client.accBalance < 0)
		{
			cout << right << setw(5) << " " << "Invalid datatype, Please enter a positive number : ";
			
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			continue;
		}

		break;
	}

	return client;
}

void addNewClient(stSystem& system, const int& userConIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConIdx].userName, "Add New Client"));
	if (!hasPermission(system.vAllUsers[userConIdx].permissions, (int)enClientPermissions::AddNewClientPer))
	{
		printAccessDeniedScreen();
		return;
	}

	stClient client{};
	char confirm = 'n';

	do {
		clearScreen();
		printConnectedUser(system.vAllUsers[userConIdx]);
		printAddNewClientHeader();
		
		string accNum = readString("Please enter account number : ");
		int idx = findClientIndex(system.vAllClients, accNum);
		
		// Validation non existance of the client before inserting
		while (idx != -1)
		{
			accNum = readString("Account with [" + accNum + "] Already Exists, Please enter another account number : ");
			idx = findClientIndex(system.vAllClients, accNum);
		}

		client = readClient(accNum);
		system.vAllClients.push_back(client);
		insertClients(system.vAllClients);

		cout << endl << right << setw(5) << " " << "Client Added Successfully, do you want to add more clients [y/Y] : ";
		cin >> confirm;
	} while (tolower(confirm) == 'y');

	pressKey();
}

void printDeleteClientScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Delete Client Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void printClientDataCard(const stClient& c)
{
	cout << endl << right << setw(5) << " " << "Client Information : " << endl;
	cout << right << setw(5) << " " << "===============================" << endl << endl;

	cout << right << setw(6) << " " << "Account Number  : " << c.accNumber << endl;
	cout << right << setw(6) << " " << "Pin Code        : " << c.pinCode << endl;
	cout << right << setw(6) << " " << "Full Name       : " << c.fullName << endl;
	cout << right << setw(6) << " " << "Phone Number    : " << c.phoneNumber << endl;
	cout << right << setw(6) << " " << "Account Balance : $" << fixed << setprecision(2) << c.accBalance << defaultfloat << endl;
	cout << right << setw(5) << " " << "===============================" << endl << endl;

}

void deleteClient(stSystem& system, const int& userConIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConIdx].userName, "Delete Client"));
	if (!hasPermission(system.vAllUsers[userConIdx].permissions, (int)enClientPermissions::DeleteClientPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(system.vAllUsers[userConIdx]);
	printDeleteClientScreen();

	string accNum = readString("Please Enter account number : ");
	int idx = findClientIndex(system.vAllClients, accNum);

	if (idx == -1)
	{
		cout << endl << right << setw(5) << " " << "Client with account number (" << accNum << ") is not found !" << endl;
		pressKey();
		return;
	}

	char confirm = 'n';
	printClientDataCard(system.vAllClients[idx]);
	
	cout << right << setw(5) << " " << "Are you sure you want to delete this client [y/Y] : ";
	cin >> confirm;

	if (tolower(confirm) == 'y')
	{
		system.vAllClients.at(idx).isMarkedForDelete = true;
		insertClients(system.vAllClients);
		cout << endl << right << setw(5) << " " << "Client Deleted Successfully." << endl;
		
		// refresh
		system.vAllClients = getAllClients();
	}
	else
		cout << endl << right << setw(5) << " " << "Deletion Canceled." << endl;

	pressKey();
}

void printUpdateClientHeader()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Update Client Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void performUpdate(stClient& c)
{
	c.pinCode = readString("Please enter a new pin code : ");
	c.fullName = readString("Please enter a new full name : ");
	c.phoneNumber = readString("Please enter a new phone number : ");

	cout << right << setw(5) << " " << "Please enter a new balance : ";

	while (true)
	{
		if (!(cin >> c.accBalance) || c.accBalance < 0)
		{
			cout << right << setw(5) << " " << "Invalid datatype, Please enter a positive number : ";

			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			continue;
		}

		break;
	}
}

void updateClient(stSystem& system, const int& userConIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConIdx].userName, "Update Client"));
	if (!hasPermission(system.vAllUsers[userConIdx].permissions ,(int)enClientPermissions::UpdateClientPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(system.vAllUsers[userConIdx]);
	printUpdateClientHeader();

	string accNum = readString("Please enter account number : ");
	int idx = findClientIndex(system.vAllClients, accNum);

	if (idx == -1)
	{
		cout << endl << right << setw(5) << " " << "Client with account number (" << accNum << ") is not found !" << endl;
		pressKey();
		return;
	}

	char confirm = 'n';
	printClientDataCard(system.vAllClients[idx]);
	
	cout << right << setw(5) << " " << "Are you sure you want to update this client [y/Y] : ";
	cin >> confirm;

	if (tolower(confirm) == 'y')
	{
		performUpdate(system.vAllClients[idx]);
		insertClients(system.vAllClients);
		cout << endl << right << setw(5) << " " << "Client Updated Successfully." << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Update Canceled." << endl;

	pressKey();
}

void printFindClientHeader()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Find Client Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void findClientUI(const stSystem& system, const int& userConIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConIdx].userName, "Find Client"));
	if (!hasPermission(system.vAllUsers[userConIdx].permissions, (int)enClientPermissions::FindClientPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(system.vAllUsers[userConIdx]);
	printFindClientHeader();

	string accNum = readString("Please enter account number : ");
	int idx = findClientIndex(system.vAllClients, accNum);

	if (idx != -1)
		printClientDataCard(system.vAllClients[idx]);
	else
		cout << endl << right << setw(5) << " " << "Client with account number (" << accNum << ") is not found !" << endl;

	cout << endl << right << setw(5) << " " << "Press Any Key To Return To Main Menu ...";

	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printTrxMainMenu()
{
	cout << right << setw(5) << " " << "============================================================" << endl;
	cout << right << setw(23) << " " << "Transaction Menu Screen" << endl;
	cout << right << setw(5) << " " << "============================================================" << endl;
	cout << right << setw(6) << " " << "[1] Deposit." << endl;
	cout << right << setw(6) << " " << "[2] Withdraw." << endl;
	cout << right << setw(6) << " " << "[3] Transfer." << endl;
	cout << right << setw(6) << " " << "[4] Total Balances." << endl;
	cout << right << setw(6) << " " << "[5] Transactions Log." << endl;
	cout << right << setw(6) << " " << "[6] Transfers Log." << endl;
	cout << right << setw(6) << " " << "[7] Main Menu." << endl;
	cout << right << setw(5) << " " << "============================================================" << endl;
}

void printTrxScreens(string whatScreen)
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << whatScreen << endl;
	cout << right << setw(5) << " " << "===============================" << endl;
}

double readAmount(string message)
{
	double amount = 0;

	cout << right << setw(5) << " " << message;
	
	while (true)
	{
		if (!(cin >> amount) || amount < 0)
		{
			cout << right << setw(5) << " " << "Please enter a positive number : ";
			
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			continue;
		}

		break;
	}

	return amount;
}

void performDeposit(stClient& c,double amount)
{
	c.accBalance += amount;
}

stTransaction createTransactionRecord(const stClient& c, const string& transactionType, double amount, const stUser& performedBy)
{
	stTransaction trx{};
	
	trx.trxID = generateSimpleGUID();
	trx.accNumber = c.accNumber;
	trx.transactionType = transactionType;
	trx.amount = amount;
	trx.transactionDateTime = getCurrentDateTime();
	trx.performedByUser = performedBy.userName;

	return trx;
}

void depositTrx(vector<stClient>& vAllClients,const vector<stUser>& vAllUsers, vector<stTransaction>& vAllTransactions, const int userConIdx)
{
	insertActionLog(createActionLog(vAllUsers[userConIdx].userName, "Deposit Transaction"));
	if (!hasPermission(vAllUsers[userConIdx].transactionsPermissions, (int)enTransactionsPermissions::DepositPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[userConIdx]);
	printTrxScreens("Deposit Screen");


	int idx = -1;
	string accNum = "";

	do{
		accNum = readString("Please enter your account number : ");
		idx = findClientIndex(vAllClients, accNum);

		if(idx == -1)
			cout << endl << right << setw(5) << " " << "Client with [" << accNum << "] does not exists." << endl;

	} while (idx == -1);

	printClientDataCard(vAllClients[idx]);
	char confirm = 'n';

	double amountToDeposit = readAmount("Please enter the deposit amount : ");
	
	cout << endl << right << setw(5) << " " << "Are you sure you want to perform this transaction [y/Y] : ";
	cin >> confirm;

	if (tolower(confirm) == 'y')
	{
		performDeposit(vAllClients[idx], amountToDeposit);
		insertClients(vAllClients);
		stTransaction trxRecord = createTransactionRecord(vAllClients[idx], "Deposit", amountToDeposit, vAllUsers[userConIdx]);
		vAllTransactions.push_back(trxRecord);
		insertLogTransaction(vAllTransactions);
		cout << endl << right << setw(5) << " " << "Deposit Operation successfully !" << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Deposit Operation Canceled !" << endl;

	pressKey();
}

void performWithdraw(stClient& c, double withdrawalAmount)
{
	c.accBalance -= withdrawalAmount;
}

bool checkAvailableBalance(const stClient& c,double amount)
{
	return c.accBalance >= amount;
}

void withdrawTrx(vector<stClient>& vAllClients,const vector<stUser>& vAllUsers, vector<stTransaction>& vAllTransactions, const int userConIdx)
{
	insertActionLog(createActionLog(vAllUsers[userConIdx].userName, "Withdraw Transaction"));
	if (!hasPermission(vAllUsers[userConIdx].transactionsPermissions ,(int)enTransactionsPermissions::WithdrawPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[userConIdx]);
	printTrxScreens("Withdraw Screen");

	string accNum = "";
	int idx = -1;

	do {
		accNum = readString("Please enter your account number : ");
		idx = findClientIndex(vAllClients, accNum);

		if(idx ==-1)
			cout << endl << right << setw(5) << " " << "Client with [" << accNum << "] does not exists." << endl;

	} while (idx == -1);

	printClientDataCard(vAllClients[idx]);
	char confirm = 'n';

	double amountToWithdraw = readAmount("Please enter the withdraw amount : ");

	if (checkAvailableBalance(vAllClients[idx], amountToWithdraw))
	{
		cout << endl << right << setw(5) << " " << "Are you sure you want to perform this transaction [y/Y] : ";
		cin >> confirm;

		if (tolower(confirm) == 'y')
		{
			performWithdraw(vAllClients[idx], amountToWithdraw);
			insertClients(vAllClients);
			stTransaction trxRecord = createTransactionRecord(vAllClients[idx], "Withdraw", amountToWithdraw, vAllUsers[userConIdx]);
			vAllTransactions.push_back(trxRecord);
			insertLogTransaction(vAllTransactions);
			cout << endl << right << setw(5) << " " << "Withdrawal Operation Successfully !" << endl;
		}
		else
			cout << endl << right << setw(5) << " " << "Withdrawal Operation Canceled !" << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Amount requested for withdrawal exceeds the user balance, Total Amount available to withdraw is : $ " << fixed << setprecision(2) << vAllClients[idx].accBalance << endl << defaultfloat;

	pressKey();
}

void printTotalBalancesHeader()
{
	cout << right << setw(5) << " " << "==============================================================" << endl;
	cout << right << setw(6) << " ";

	cout << left << setw(15) << "Account Number" << "|" << " ";
	cout << left << setw(25) << "Client Name" << "|" << " ";
	cout << left << setw(16) << "Account Balance" << "|" << endl;

	cout << right << setw(5) << " " << "==============================================================" << endl;
}

double totalBalances(const vector<stClient>& vAllClients)
{
	double sum = 0;

	for (const stClient& c : vAllClients)
		sum += c.accBalance;

	return sum;
}

void printTransferScreenHeader()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Transfer Screen" << endl;
	cout << right << setw(5) << " " << "===============================" << endl;
}

stTransfer fillTransferInfo(const string& fromAcc, const string& toAcc, const double& amount,const string& byUser)
{
	stTransfer transfer{};

	transfer.TransferID = generateSimpleGUID();
	transfer.fromAccNum = fromAcc;
	transfer.toAccNum = toAcc;
	transfer.amount = amount;
	transfer.performedByUser = byUser;
	transfer.transferDateTime = getCurrentDateTime();

	return transfer;
}

void transferTrx(vector<stClient>& vAllClients,const vector<stUser>& vAllUsers, vector<stTransfer>& vAllTransfers, const int& connectedUser)
{
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Access Transfer Menu"));
	
	if (!hasPermission(vAllUsers[connectedUser].transactionsPermissions, (int)enTransactionsPermissions::TransferPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printTransferScreenHeader();

	// Read Account Sender and find it
	string accFrom = readString("Please enter Account Sender number : ");
	int accFromIdx = findClientIndex(vAllClients, accFrom);

	while (accFromIdx == -1)
	{
		cout << right << setw(5) << " " << "Account Number not found! Please check again." << endl;
		
		accFrom = readString("Please enter valid Account Sender number : ");
		accFromIdx = findClientIndex(vAllClients, accFrom);
	};

	// Read and validate transfer amount
	double amount = readAmount("Please enter Transferable Amount : ");
	while (!checkAvailableBalance(vAllClients[accFromIdx],amount))
	{
		cout << endl << right << setw(5) << " " << "Amount requested for Transfer exceeds the balance, Total Amount available to Transfer is : $ " << fixed << setprecision(2) << vAllClients[accFromIdx].accBalance << endl << defaultfloat;
		amount = readAmount("Please enter Another amount less than or equal your balance : ");
	}

	// Read Account Reciever and find it
	string accTo = readString("Please enter Account Reciever number : ");
	int accToIdx = findClientIndex(vAllClients, accTo);

	while (accToIdx == -1)
	{
		cout << right << setw(5) << " " << "Account Number not found! Please check again." << endl;

		accTo = readString("Please enter valid Account Reciever number : ");
		accToIdx = findClientIndex(vAllClients, accTo);
	};

	// Print both date to user
	printClientDataCard(vAllClients[accFromIdx]);
	cout << endl << endl;
	printClientDataCard(vAllClients[accToIdx]);

	char confirm = readYesNo("Are you sure you want to perfom this transfer [y/Y] : ");
	
	if (tolower(confirm) == 'y')
	{
		vAllClients[accFromIdx].accBalance -= amount;
		vAllClients[accToIdx].accBalance += amount;

		// fill the transfer Infos
		vAllTransfers.push_back(fillTransferInfo(accFrom, accTo, amount, vAllUsers[connectedUser].userName));

		// inserting new clients data
		insertClients(vAllClients);

		// Inserting new transfers data
		insertTransfers(vAllTransfers);

		cout << right << setw(5) << " " << "Transfer went successfully." << endl;
	}
	else
		cout << right << setw(5) << " " << "Transfer has been Canceled successfully." << endl;

	pressKey();
}

void printTotalBalances(const vector<stClient>& vAllClients,const vector<stUser>& vAllUsers, const int userConIdx)
{
	insertActionLog(createActionLog(vAllUsers[userConIdx].userName, "Total Balances"));
	if (!hasPermission(vAllUsers[userConIdx].transactionsPermissions, (int)enTransactionsPermissions::TotalBalancesPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[userConIdx]);
	printTotalBalancesHeader();

	for (const stClient& c : vAllClients)
	{
		cout << right << setw(5) << " " << "|";
		
		cout << left << setw(15) << c.accNumber << "|" << " ";
		cout << left << setw(25) << c.fullName << "|" << " $";
		cout << left << setw(15) << fixed << setprecision(2) << c.accBalance << "|" << defaultfloat << endl;
	}
	cout << right << setw(5) << " " << "==============================================================" << endl;
	cout << right << setw(6) << " " << "Total Balances : $" << fixed << setprecision(2) << totalBalances(vAllClients) << defaultfloat << endl;
	cout << right << setw(5) << " " << "==============================================================" << endl << endl;

	pressKey();
}

void printTransactionsLogHeader()
{
	cout << right << setw(5) << " " << "======================================================================================================================" << endl;
	cout << right << setw(6) << " ";
	cout << left << setw(15) << "Transaction ID" << "|" << " ";
	cout << left << setw(15) << "Account Number" << "|" << " ";
	cout << left << setw(20) << "Transaction Type" << "|" << " ";
	cout << left << setw(11) << "Amount" << "|" << " ";
	cout << left << setw(25) << "Performed By" << "|" << " ";
	cout << left << setw(20) << "Date & Time" << "|" << endl;
	cout << right << setw(5) << " " << "======================================================================================================================" << endl;
}

void printTransactionsLog(const vector<stTransaction>& vAllTransactions, const stUser& connectedUser)
{
	insertActionLog(createActionLog(connectedUser.userName, "View Transactions Log"));
	clearScreen();
	printConnectedUser(connectedUser);
	printTransactionsLogHeader();

	for (const stTransaction& t : vAllTransactions)
	{
		cout << right << setw(6) << " ";
		
		cout << left << setw(15) << t.trxID << "| ";
		cout << left << setw(15) << t.accNumber  << "| ";
		cout << left << setw(20) << t.transactionType << "| $";
		cout << left << setw(10)  << fixed  << setprecision(2)  << t.amount  << "|"  << defaultfloat << " ";
		cout << left << setw(25)  << t.performedByUser << "| ";
		cout << left << setw(20)  << t.transactionDateTime << "|" << endl;
	}

	cout << right << setw(5) << " " << "======================================================================================================================" << endl;
	
	pressKey();
}

void printTransfersLogHeader()
{
	cout << right << setw(5) << " " << "===============================================================================================================" << endl;
	cout << right << setw(6) << " ";
	cout << left << setw(12) << "Transfer ID" << "|" << " ";
	cout << left << setw(15) << "Account From" << "|" << " ";
	cout << left << setw(15) << "Account To" << "|" << " ";
	cout << left << setw(11) << "Amount" << "|" << " ";
	cout << left << setw(25) << "Performed By" << "|" << " ";
	cout << left << setw(21) << "Date & Time" << "|" << endl;
	cout << right << setw(5) << " " << "===============================================================================================================" << endl;
}

void printTransfersLog(const vector<stTransfer>& vAllTransfers, const stUser& connectedUser)
{
	insertActionLog(createActionLog(connectedUser.userName, "View Transfers Log"));
	clearScreen();
	printConnectedUser(connectedUser);
	printTransfersLogHeader();

	for (const stTransfer& t : vAllTransfers)
	{
		cout << right << setw(6) << " ";

		cout << left << setw(12) << t.TransferID << "|" << " ";
		cout << left << setw(15) << t.fromAccNum << "|" << " ";
		cout << left << setw(15) << t.toAccNum << "| $";
		cout << left << setw(10) << fixed << setprecision(2) << t.amount << "|" << defaultfloat << " ";
		cout << left << setw(25) << t.performedByUser << "|" << " ";
		cout << left << setw(21) << t.transferDateTime << "|" << endl;
	}

	cout << right << setw(5) << " " << "===============================================================================================================" << endl;

	pressKey();
}

void redirectTrxMenu(const enMenuTransactions& trx, stSystem& system, const int userConIdx)
{
	switch (trx)
	{
	case enMenuTransactions::Deposit:
		depositTrx(system.vAllClients, system.vAllUsers, system.vAllTransactions, userConIdx);
		break;
	case enMenuTransactions::Withdraw:
		withdrawTrx(system.vAllClients, system.vAllUsers, system.vAllTransactions, userConIdx);
		break;
	case enMenuTransactions::Transfer:
		transferTrx(system.vAllClients, system.vAllUsers, system.vAllTransfers, userConIdx);
		break;
	case enMenuTransactions::TotalBalances:
		printTotalBalances(system.vAllClients, system.vAllUsers, userConIdx);
		break;
	case enMenuTransactions::TransactionsLog:
		printTransactionsLog(system.vAllTransactions, system.vAllUsers[userConIdx]);
		break;
	case enMenuTransactions::TransfersLog:
		printTransfersLog(system.vAllTransfers, system.vAllUsers[userConIdx]);
		break;
	case enMenuTransactions::MainMenu:
		break;
	default:
		return;
	}
}

void transactionMenu(stSystem& system, const int& userConIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConIdx].userName, "Access Transactions Menu"));
	if (!hasPermission(system.vAllUsers[userConIdx].permissions ,(int)enClientPermissions::TransactionsPer))
	{
		printAccessDeniedScreen();
		return;
	}

	enMenuTransactions trxChoice{};

	do
	{
		clearScreen();
		printConnectedUser(system.vAllUsers[userConIdx]);
		printTrxMainMenu();
		trxChoice = readTransactionsMenuChoice();

		redirectTrxMenu(trxChoice, system, userConIdx);
	} while (trxChoice != enMenuTransactions::MainMenu);
}

void printManageUsersScreenMenu()
{
	cout << right << setw(5) << " " << "================================================================================\n";
	cout << right << setw(32) << " " << "Manage Users Screen Menu\n";
	cout << right << setw(5) << " " << "================================================================================\n";

	cout << right << setw(5) << " " << "[1] List Users.\n";
	cout << right << setw(5) << " " << "[2] Add New User.\n";
	cout << right << setw(5) << " " << "[3] Delete User.\n";
	cout << right << setw(5) << " " << "[4] Update User.\n";
	cout << right << setw(5) << " " << "[5] Find User.\n";
	cout << right << setw(5) << " " << "[6] Revoke Permissions Options (Admin Only)" << endl; // For now
	cout << right << setw(5) << " " << "[7] Lock User (Admin Only)" << endl;
	cout << right << setw(5) << " " << "[8] Unlock User (Admin Only)" << endl;
	cout << right << setw(5) << " " << "[9] View Action Logs " << endl;
	cout << right << setw(5) << " " << "[10] Main Menu.\n";
	cout << right << setw(5) << " " << "================================================================================\n";
}

void printListUsersHeader()
{
	cout << right << setw(5) << " " << "==================================================================================\n";
	cout << right << setw(6) << " ";
	cout << left << setw(15) << "Username" << "|" << " ";
	cout << left << setw(12) << "Permissions" << "|" << " ";
	cout << left << setw(16) << "Trx Permissions" << "|" << " ";
	cout << left << setw(19) << "Manage Users Perms" << "|" << " ";
	cout << left << setw(10) << "Is Locked" << "|" << endl;
	cout << right << setw(5) << " " << "==================================================================================\n";
}

void listUsers(const vector<stUser>& vAllUsers, const int& connectedUser)
{
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "List Users"));
	if (!hasPermission(vAllUsers[connectedUser].manageUsersPermissions, enManageUsersPermissions::ListUsersPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printListUsersHeader();

	for (const stUser& u : vAllUsers)
	{
		cout << right << setw(6) << " ";
		cout << left << setw(15) << u.userName << "|" << " ";
		cout << left << setw(12) << u.permissions << "|" << " ";
		cout << left << setw(16) << u.transactionsPermissions << "|" << " ";
		cout << left << setw(19) << u.manageUsersPermissions << "|" << " ";
		cout << left << setw(10) << (u.isLocked ? "Yes" : "No") << "|" << endl;
	}
	cout << right << setw(5) << " " << "==================================================================================\n";
	cout << right << setw(5) << " " << "Total Number Of Users is : " << (int)vAllUsers.size() << endl << endl;
	pressKey();
}

void addNewUserScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Add New User Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

bool grantAllPermissions(stUser& user)
{
	cout << endl;

	char confirmAll = readYesNo("Do you want to give full permissions [y/Y] : ");
	if (tolower(confirmAll) == 'y')
	{
		user.permissions = FULL_PERMISSION;
		user.transactionsPermissions = FULL_PERMISSION;
		user.manageUsersPermissions = FULL_PERMISSION;
	}

	return tolower(confirmAll) == 'y';
}

void grantClientPermissions(stUser& user)
{
	char confirmAll = 'n';

	cout << endl << right << setw(5) << " " << "Do you want to give access to : " << endl << endl;
	cout << right << setw(5) << " " << "Client Permissions : " << endl << endl;
	cout << right << setw(5) << " " << "=======================================" << endl << endl;

	confirmAll = readYesNo("[1] Show Clients List Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
		user.permissions |= (int)enClientPermissions::ShowClientsListPer;

	confirmAll = readYesNo("[2] Add New Client Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
		user.permissions |= (int)enClientPermissions::AddNewClientPer;

	confirmAll = readYesNo("[3] Delete Client Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
		user.permissions |= (int)enClientPermissions::DeleteClientPer;

	confirmAll = readYesNo("[4] Update Client Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
		user.permissions |= (int)enClientPermissions::UpdateClientPer;

	confirmAll = readYesNo("[5] Find Client Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
		user.permissions |= (int)enClientPermissions::FindClientPer;
}

void grantTrxPermissions(stUser& user)
{
	char confirmAll = 'n';

	cout << right << setw(5) << " " << "=======================================" << endl << endl;
	cout << right << setw(5) << " " << "Transactions Permissions : " << endl << endl;
	cout << right << setw(5) << " " << "=======================================" << endl << endl;

	confirmAll = readYesNo("[6] Transactions Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
	{
		user.permissions |= (int)enClientPermissions::TransactionsPer;

		confirmAll = readYesNo("Do you want to grant full Transactions Permissions [y/Y] : ");
		if (tolower(confirmAll) == 'y')
			user.transactionsPermissions = FULL_PERMISSION;

		if (user.transactionsPermissions != FULL_PERMISSION)
		{
			cout << endl << right << setw(5) << " " << "Do you want to give access to : " << endl << endl;

			confirmAll = readYesNo("[1] Deposit Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.transactionsPermissions |= (int)enTransactionsPermissions::DepositPer;

			confirmAll = readYesNo("[2] Withdraw Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.transactionsPermissions |= (int)enTransactionsPermissions::WithdrawPer;

			confirmAll = readYesNo("[3] Transfer Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.transactionsPermissions |= (int)enTransactionsPermissions::TransferPer;

			confirmAll = readYesNo("[3] Total Balances Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.transactionsPermissions |= (int)enTransactionsPermissions::TotalBalancesPer;
		}
	}
	else
		user.transactionsPermissions = 0;
}

void grantManageUsersPermissions(stUser& user)
{
	char confirmAll = 'n';

	cout << right << setw(5) << " " << "=======================================" << endl << endl;
	cout << right << setw(5) << " " << "Manage Users Permissions : " << endl << endl;
	cout << right << setw(5) << " " << "=======================================" << endl << endl;

	confirmAll = readYesNo("[7] Manage Users Permission [y/Y] : ");
	if (tolower(confirmAll) == 'y')
	{
		user.permissions |= (int)enClientPermissions::ManageUsersPer;

		confirmAll = readYesNo("Do you want to grant full Manage Users Permissions [y/Y] : ");
		if (tolower(confirmAll) == 'y')
			user.manageUsersPermissions = FULL_PERMISSION;

		if (user.manageUsersPermissions != FULL_PERMISSION)
		{
			cout << endl << right << setw(5) << " " << "Do you want to give access to : " << endl << endl;

			confirmAll = readYesNo("[1] Show Users Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.manageUsersPermissions |= (int)enManageUsersPermissions::ListUsersPer;

			confirmAll = readYesNo("[2] Add New User Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.manageUsersPermissions |= (int)enManageUsersPermissions::AddNewUserPer;

			confirmAll = readYesNo("[3] Delete User Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.manageUsersPermissions |= (int)enManageUsersPermissions::DeleteUserPer;

			confirmAll = readYesNo("[4] Update User Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.manageUsersPermissions |= (int)enManageUsersPermissions::UpdateUserPer;

			confirmAll = readYesNo("[5] Find User Permission [y/Y] : ");
			if (tolower(confirmAll) == 'y')
				user.manageUsersPermissions |= (int)enManageUsersPermissions::FindUserPer;
		}
	}
	else
		user.manageUsersPermissions = 0;
}

char readYesNo(string message)
{
	char confirm = 'n';
	
	cout << right << setw(5) << " " << message;
	cin >> confirm;
	
	return tolower(confirm);
}

stUser addNewUser(const vector<stUser>& vAllUsers,const int userConIdx)
{
	stUser user{};
	
	user.userName = readString("Please enter username : ");
	user.permissions = 0;
	int userIdx = findUserIndex(vAllUsers, user.userName); // to validate non existance of the user before inserting
	
	while (userIdx != -1)
	{
		clearScreen();
		addNewUserScreen();

		user.userName = readString("User with [" + user.userName + "] Already Exists, Please enter another username : ");
		userIdx = findUserIndex(vAllUsers, user.userName);
	}

	user.password = hashPassword(readString("Please enter password (Hidden) : ", true), 5);

	// Grant Full Permissions
	if(grantAllPermissions(user))
		return user;

	// Grant Client Permissions
	grantClientPermissions(user);

	// Grant Transactions Permissions
	grantTrxPermissions(user);

	// Grant Manage Users Permissions
	grantManageUsersPermissions(user);

	return user;
}

string convertUserDataToStringRecord(const stUser& u)
{
	string record = "";
	
	record += u.userName + delim;
	record += u.password + delim;
	record += to_string(u.permissions) + delim;
	record += to_string(u.transactionsPermissions) + delim;
	record += to_string(u.manageUsersPermissions) + delim;
	record += to_string(u.isLocked);
	
	return record;
}

void insertUsers(const vector<stUser>& vAllUsers)
{
	ifstream temp(usersFileName);
	ofstream myFile("temp.txt", ios::out);

	if (!myFile)
	{
		cout << right << setw(5) << " " << "Error Oppening File!\n";
		return;
	}

	if (temp.is_open())
	{
		for (const stUser& u : vAllUsers)
			myFile << convertUserDataToStringRecord(u) << endl;

		myFile.close();
	}

	temp.close();

	remove(usersFileName.c_str());
	
	if(!rename("temp.txt", usersFileName.c_str()))
		cout << right << setw(5) << " " << "Users file updated successfully." << endl;
	else
		cout << right << setw(5) << " " << "Error in updating users file." << endl;
}

void addUsers(vector<stUser>& vAllUsers,const int& connectedUser)
{
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Add New User"));
	if (!hasPermission(vAllUsers[connectedUser].manageUsersPermissions, enManageUsersPermissions::AddNewUserPer))
	{
		printAccessDeniedScreen();
		return;
	}

	char confirmAll = 'n';

	do {
		clearScreen();
		printConnectedUser(vAllUsers[connectedUser]);
		addNewUserScreen();

		stUser user = addNewUser(vAllUsers, connectedUser);
		vAllUsers.push_back(user);
		insertUsers(vAllUsers);
		
		cout << endl << right << setw(5) << " " << "User Added Successfully. Do you want to add more users [y/Y] : ";
		cin >> confirmAll;

	} while (tolower(confirmAll) == 'y');

	pressKey();
}

void printDeleteUserScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Delete User Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void printUser(const vector<stUser>& vAllUsers, const int& userIdx)
{
	cout << endl << right << setw(5) << " " << "User Information : " << endl;
	cout << right << setw(5) << " " << "===============================" << endl << endl;
	cout << right << setw(6) << " " << left << setw(28) << "Username : " << vAllUsers[userIdx].userName << endl;
	//cout << right << setw(6) << " " << left << setw(28) << "Password : " << vAllUsers[userIdx].password << endl;
	cout << right << setw(6) << " " << left << setw(28) << "Permissions : " << vAllUsers[userIdx].permissions << endl;
	cout << right << setw(6) << " " << left << setw(28) << "Transactions Permissions : " << vAllUsers[userIdx].transactionsPermissions << endl;
	cout << right << setw(6) << " " << left << setw(28) << "Manage User Permissions : " << vAllUsers[userIdx].manageUsersPermissions << endl;
	cout << right << setw(6) << " " << left << setw(28) << "Is Locked : " << (vAllUsers[userIdx].isLocked ? "Yes" : "No") << endl;
	cout << right << setw(5) << " " << "===============================" << endl << endl;
}

void deleteUser(vector<stUser>& vAllUsers,const int& connectedUser)
{
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Delete User"));
	if (!hasPermission(vAllUsers[connectedUser].manageUsersPermissions, enManageUsersPermissions::DeleteUserPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printDeleteUserScreen();
	
	string username = readString("Please enter username : ");
	int userIdx = findUserIndex(vAllUsers, username);
	
	if (userIdx == -1)
	{
		cout << endl << right << setw(5) << " " << "User with username (" << username << ") is not found !" << endl;
		pressKey();
		return;
	}

	if(vAllUsers[userIdx].userName == "Admin")
		{
		cout << endl << right << setw(5) << " " << "You cannot delete the default admin user !" << endl;
		pressKey();
		return;
	}

	printUser(vAllUsers, userIdx);
	
	char confirm = 'n';
	cout << endl << right << setw(5) << " " << "Are you sure you want to delete this user [y/Y] : ";
	cin >> confirm;
	
	if (tolower(confirm) == 'y')
	{
		vAllUsers.erase(vAllUsers.begin() + userIdx);
		insertUsers(vAllUsers);
		
		cout << endl << right << setw(5) << " " << "User Deleted Successfully." << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Deletion Canceled." << endl;
	
	pressKey();
}

void resetPermissions(stUser& user)
{
	user.permissions = 0;
	user.transactionsPermissions = 0;
	user.manageUsersPermissions = 0;
}

void printUpdateUserScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Update User Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

stUser updateTheFoundUser(vector<stUser>& vAllUsers, const int& userIdx)
{
	vAllUsers[userIdx].password = hashPassword(readString("Please enter a new password (Hidden) : ", true), 5);

	// Reset Permissions
	resetPermissions(vAllUsers[userIdx]);

	// Grant Full Permissions
	if (grantAllPermissions(vAllUsers[userIdx]))
		return vAllUsers[userIdx];

	// Grant Client Permissions
	grantClientPermissions(vAllUsers[userIdx]);

	// Grant Transactions Permissions
	grantTrxPermissions(vAllUsers[userIdx]);

	// Grant Manage Users Permissions
	grantManageUsersPermissions(vAllUsers[userIdx]);

	return vAllUsers[userIdx];
}

void printFindUserScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Find User Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void findUser(const vector<stUser>& vAllUsers, const int& connectedUser)
{
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Find User"));
	if (!hasPermission(vAllUsers[connectedUser].manageUsersPermissions, enManageUsersPermissions::FindUserPer))
	{
		printAccessDeniedScreen();
		return;
	}
	
	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printFindUserScreen();
	
	string username = readString("Please enter username : ");
	int userIdx = findUserIndex(vAllUsers, username);
	
	if (userIdx != -1)
		printUser(vAllUsers, userIdx);
	else
		cout << endl << right << setw(5) << " " << "User with username (" << username << ") is not found !" << endl;
	
	cout << right << setw(5) << " " << "Press any key to continue ...";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void updateUser(vector<stUser>& vAllUsers, const int& connectedUser)
{
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Update User"));
	if (!hasPermission(vAllUsers[connectedUser].manageUsersPermissions, enManageUsersPermissions::UpdateUserPer))
	{
		printAccessDeniedScreen();
		return;
	}

	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printUpdateUserScreen();
	
	string username = readString("Please enter username : ");
	int userIdx = findUserIndex(vAllUsers, username);
	
	if (userIdx == -1)
	{
		cout << endl << right << setw(5) << " " << "User with username (" << username << ") is not found !" << endl;
		pressKey();
		return;
	}
	
	printUser(vAllUsers, userIdx);
	char confirm = 'n';
	cout << endl << right << setw(5) << " " << "Are you sure you want to update this user [y/Y] : ";
	cin >> confirm;
	
	if (tolower(confirm) == 'y')
	{
		stUser updatedUser = updateTheFoundUser(vAllUsers, userIdx);
		vAllUsers[userIdx] = updatedUser;
		insertUsers(vAllUsers);
		
		cout << endl << right << setw(5) << " " << "User Updated Successfully." << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Update Canceled." << endl;
	
	pressKey();
}

void printRevokeUserPermissionsScreen()
{
	cout << right << setw(5) << " " << "========================================" << endl;
	cout << right << setw(10) << " " << "Revoke User Permissions Screen\n";
	cout << right << setw(5) << " " << "========================================" << endl;
}

enpermissionsType readPermissionTypeToRevoke()
{
	cout << endl << right << setw(5) << " " << "Which Permission type Do you want to revoke : \n";
	cout << right << setw(5) << " " << "[1] Client Permissions." << endl;
	cout << right << setw(5) << " " << "[2] Transactions Permissions." << endl;
	cout << right << setw(5) << " " << "[3] Manage Users Permissions.\n" << endl;
	cout << right << setw(5) << " " << "Your choice : ";
	
	int permissionType = validateINTTypeAndRange(1, 3);
	
	return (enpermissionsType)permissionType;
}

void revokeUserPermissions(vector<stUser>& vAllUsers, const int& connectedUser)
{
	clearScreen();
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Revoke User Permissions"));
	
	printConnectedUser(vAllUsers[connectedUser]);
	printRevokeUserPermissionsScreen();

	if (vAllUsers[connectedUser].userName != "Admin")
	{
		cout << right << setw(5) << " " << "Only Admin can revoke permissions." << endl;
		pressKey();
		return ;
	}

	string username = readString("Please enter username : ");
	int userIdx = findUserIndex(vAllUsers, username);
	
	if (userIdx == -1)
	{
		cout << endl << right << setw(5) << " " << "User with username (" << username << ") is not found !" << endl;
		pressKey();
		return;
	}

	printUser(vAllUsers, userIdx);

	char confirm = readYesNo("Revoke all permissions of the user, Do you want to continue [y/Y] : ");
	if (tolower(confirm) == 'y')
	{
		resetPermissions(vAllUsers[userIdx]);
		insertUsers(vAllUsers);

		cout << endl << right << setw(5) << " " << "User Permissions Revoked Successfully." << endl;
		pressKey();
		return;
	}
	
	enpermissionsType permissionType = readPermissionTypeToRevoke();
	if(revokePermission(vAllUsers[userIdx], permissionType))
		insertUsers(vAllUsers);
	else
		cout << endl << right << setw(5) << " " << "No permissions revoked." << endl;
	
	pressKey();
}

void printLockUserScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Lock User Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void lockUserAction(vector<stUser>& vAllUsers, const int& connectedUser)
{
	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printLockUserScreen();
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Lock User"));

	if (vAllUsers[connectedUser].userName != "Admin")
	{
		cout << right << setw(5) << " " << "Only Admin can lock users." << endl;
		pressKey();
		return;
	}

	string username = readString("Please enter username : ");
	int userIdx = findUserIndex(vAllUsers, username);
	
	if (userIdx == -1)
	{
		cout << endl << right << setw(5) << " " << "User with username (" << username << ") is not found !" << endl;
		pressKey();
		return;
	}

	if (vAllUsers[userIdx].isLocked)
	{
		cout << endl << right << setw(5) << " " << "User is already locked." << endl;
		pressKey();
		return;
	}

	printUser(vAllUsers, userIdx);
	char confirm = readYesNo("Are you sure you want to lock this user [y/Y] : ");
	
	if (tolower(confirm) == 'y')
	{
		lockUser(vAllUsers[userIdx]);;
		
		insertUsers(vAllUsers);
		cout << endl << right << setw(5) << " " << "User Locked Successfully." << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Locking Canceled." << endl;
	
	pressKey();
}

void printUnlockUserScreen()
{
	cout << right << setw(5) << " " << "===============================" << endl;
	cout << right << setw(10) << " " << "Unlock User Screen\n";
	cout << right << setw(5) << " " << "===============================" << endl;
}

void unlockUserAction(vector<stUser>& vAllUsers, const int& connectedUser)
{
	clearScreen();
	printConnectedUser(vAllUsers[connectedUser]);
	printUnlockUserScreen();
	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "Unlock User"));
  
	if (vAllUsers[connectedUser].userName != "Admin")
	{
		cout << right << setw(5) << " " << "Only Admin can unlock users." << endl;
		pressKey();
		return;
	}
	
	string username = readString("Please enter username : ");
	int userIdx = findUserIndex(vAllUsers, username);
	
	if (userIdx == -1)
	{
		cout << endl << right << setw(5) << " " << "User with username (" << username << ") is not found !" << endl;
		pressKey();
		return;
	}

	if (!vAllUsers[userIdx].isLocked)
	{
		cout << endl << right << setw(5) << " " << "User is not locked." << endl;
		pressKey();
		return;
	}

	printUser(vAllUsers, userIdx);
	char confirm = readYesNo("Are you sure you want to unlock this user [y/Y] : ");
	
	if (tolower(confirm) == 'y')
	{
		unlockUser(vAllUsers[userIdx]);
		
		insertUsers(vAllUsers);
		cout << endl << right << setw(5) << " " << "User Unlocked Successfully." << endl;
	}
	else
		cout << endl << right << setw(5) << " " << "Unlocking Canceled." << endl;
	
	pressKey();
}

void printActionLogsHeader()
{
	cout << right << setw(5) << " " << "============================================================================\n";
	cout << right << setw(6) << " ";
	cout << left << setw(25) << "Action Performed" << "|" << " ";
	cout << left << setw(25) << "Performed By" << "|" << " ";
	cout << left << setw(20) << "Date & Time" << "|" << endl;
	cout << right << setw(5) << " " << "============================================================================\n";
}

void printActionLogs(const stUser& user, const int& connectedUser, const vector<stUser>& vAllUsers, vector<stActionLog>& vAllActionLogs)
{
	// refresh action logs to get the new log of viewing action logs
	vAllActionLogs = getAllActionLogs();

	insertActionLog(createActionLog(vAllUsers[connectedUser].userName, "View Action Logs"));
	clearScreen();
	printConnectedUser(user);
	printActionLogsHeader();
	
	int record = 0;

	for (const stActionLog& log : vAllActionLogs)
	{
		++record;

		if (record != 20)
		{
			cout << right << setw(6) << " ";
			cout << left << setw(25) << log.action << "|" << " ";
			cout << left << setw(25) << log.userName << "|" << " ";
			cout << left << setw(20) << log.actionDateTime << "|" << endl;
		}
		else
		{
			cout << right << setw(5) << " " << "============================================================================\n";
			
			pressKey();
			clearScreen();
			printConnectedUser(user);
			printActionLogsHeader();

			record = 0;
			continue;
		}
	}

	cout << right << setw(5) << " " << "============================================================================\n";
	pressKey();
}

void redirectManageUsersMenu(const enManageUsersMenu& manageUsersChoice, stSystem& system, const int& connectedUser)
{
	switch (manageUsersChoice)
	{
	case enManageUsersMenu::ListUsers:
		listUsers(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::AddNewUser:
		addUsers(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::DeleteUser:
		deleteUser(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::UpdateUser:
		updateUser(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::FindUser:
		findUser(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::RevokeUserPermissions:
		revokeUserPermissions(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::LockUser:
		lockUserAction(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::UnlockUser:
		unlockUserAction(system.vAllUsers, connectedUser);
		break;
	case enManageUsersMenu::ShowActionLogs:
		printActionLogs(system.vAllUsers[connectedUser], connectedUser, system.vAllUsers, system.vAllActionLogs);
		break;
	case enManageUsersMenu::ReturnMainMenu:
		break;
	default:
		return;
	}
}

void manageUsers(stSystem& system, const int& userConIdx)
{
	insertActionLog(createActionLog(system.vAllUsers[userConIdx].userName, "Access Manage Users Menu"));
	if (!hasPermission(system.vAllUsers[userConIdx].permissions, (int)enClientPermissions::ManageUsersPer))
	{
		printAccessDeniedScreen();
		return;
	}

	enManageUsersMenu manageUsersChoice{};
   
	do
	{
		clearScreen();
		printConnectedUser(system.vAllUsers[userConIdx]);
		printManageUsersScreenMenu();

		manageUsersChoice = readManageUsersMenuChoice();
		redirectManageUsersMenu(manageUsersChoice, system, userConIdx);

	} while (manageUsersChoice != enManageUsersMenu::ReturnMainMenu);
}

void redirectChoice(enMenu userChoice,stSystem& system,const int& userConnectedIdx)
{
	switch (userChoice)
	{
	case enMenu::ShowClientsList:
		showAllClients(system, userConnectedIdx);
		break;
	case enMenu::AddNewClient:
		addNewClient(system, userConnectedIdx);
		break;
	case enMenu::DeleteClient:
		deleteClient(system, userConnectedIdx);
		break;
	case enMenu::UpdateClient:
		updateClient(system, userConnectedIdx);
		break;
	case enMenu::FindClient:
		findClientUI(system, userConnectedIdx);
		break;
	case enMenu::transactions:
		transactionMenu(system, userConnectedIdx);
		break;
	case enMenu::ManageUsers:
		manageUsers(system, userConnectedIdx);
		break;
	case enMenu::Logout:
	{
		insertActionLog(createActionLog(system.vAllUsers[userConnectedIdx].userName, "Logout"));
		startBankApp(system);
	}
		break;
	default:
		return;
	}
}

vector<string> splitStringRecordToVectorData(string record)
{
	vector<string> vData{};
	string word = "";
	size_t pos = 0;

	while ((pos = record.find(delim)) != string::npos)
	{
		word = record.substr(0, pos);
		
		if (!word.empty())
			vData.push_back(word);


		record.erase(0, pos + strlen(delim));
	}

	if (record != "")
		vData.push_back(record);

	return vData;
}

stClient convertStringRecordToClientData(string record)
{
	stClient client{};
	vector<string> vData = splitStringRecordToVectorData(record);

	if (vData.size() != 5)
		return stClient{};

	client.accNumber = vData[0];
	client.pinCode = vData[1];
	client.fullName = vData[2];
	client.phoneNumber = vData[3];
	
	try
	{
		client.accBalance = stod(vData[4]);
	}
	catch (...)
	{
		client.accBalance = 0;
	}

	return client;
}

vector<stClient> getAllClients()
{
	vector<stClient> vAllClients{};

	ifstream myFile(clientsFileName);

	if (myFile.is_open())
	{
		stClient client{};
		string record = "";

		while (getline(myFile, record))
		{
			client = convertStringRecordToClientData(record);
			
			if(!client.accNumber.empty())
				vAllClients.push_back(client);
		}
	}

	return vAllClients;
}

void loginScreen()
{
	cout << right << setw(5) << " " << "########################################" << endl;
	cout << right << setw(19) << " " << "Login Screen" << endl;
	cout << right << setw(5) << " " << "########################################" << endl;
}

int findUserIndex(const vector<stUser>& vAllUsers, string username)
{
	if (vAllUsers.empty())
		return -1;

	for (size_t i = 0;i < vAllUsers.size();i++)
	{
		if (vAllUsers[i].userName == username)
			return static_cast<int>(i);
	}

	return -1;
}

bool login(vector<stUser>& vAllUsers,int& userConnectedIdx)
{
	string userName = "";
	string password = "";
	int userIdx = -1;

	while (true)
	{
		clearScreen();
		loginScreen();

		userName = readString("Please enter your username : ");
		userIdx = findUserIndex(vAllUsers, userName);

		if (userIdx == -1)
		{
			cout << right << setw(5) << " " << "Invalid Username !";
			Sleep(3000);
			continue;
		}
		
		password = readString("Please enter your password (Hidden) : ", true);
		
		if (unHashPass(vAllUsers[userIdx].password, 5) != password)
		{
			cout << endl << right << setw(5) << " " << "Invalid Password !";
			Sleep(3000);
			continue;
		}

		if (vAllUsers[userIdx].isLocked)
		{
			cout << endl << right << setw(5) << " " << "Your Account is Locked, Please contact the admin !";
			Sleep(3000);
			continue;
		}
		insertActionLog(createActionLog(vAllUsers[userIdx].userName, "Login"));

		cout << endl << right << setw(5) << " " << "Login Successfully, Welcome " << vAllUsers[userIdx].userName << " !\n" << endl;
		cout << right << setw(5) << " " << "Redirecting To Main Menu ";
		userConnectedIdx = userIdx;

		for (int i = 1;i < 4;i++) { cout << "."; Sleep(1500); };
		
		break;
	}

	return true;
}

void startBankApp(stSystem& system)
{
	enMenu choice{};
	int userConnectedIdx = -1;

	clearScreen();
	loginScreen();
	bool isLogedIn = login(system.vAllUsers, userConnectedIdx);

	while (isLogedIn && choice != enMenu::Logout)
	{
		clearScreen();
		printMainMenu(system.vAllUsers, userConnectedIdx);
		
		choice = readUserChoice(system.vAllUsers[userConnectedIdx]);
		redirectChoice(choice, system, userConnectedIdx);
	}
}

stUser convertUserLineDataToStruct(string userLineRecord)
{
	stUser user{};
	vector<string> vUserRecord = splitStringRecordToVectorData(userLineRecord);

	if (vUserRecord.size() < 3 || vUserRecord.empty())
		return user;

	user.userName = vUserRecord[0];
	user.password = vUserRecord[1];
	user.permissions = stoi(vUserRecord[2]);
	user.transactionsPermissions = stoi(vUserRecord[3]);
	user.manageUsersPermissions = stoi(vUserRecord[4]);
	user.isLocked = stoi(vUserRecord[5]) == 1 ? true : false;

	return user;
}

vector<stUser> getAllUsers()
{
	ifstream myFlie(usersFileName);
	vector<stUser> vAllUsers{};

	if (myFlie.is_open())
	{
		stUser user{};
		string userLine = "";
		
		while (getline(myFlie, userLine))
		{
			user = convertUserLineDataToStruct(userLine);

			if(!user.userName.empty())
				vAllUsers.push_back(user);
		}

		myFlie.close();
	}

	return vAllUsers;
}

// From here

stTransaction convertTransactionLineToStruct(string transactionLineRecord)
{
	stTransaction transaction{};
	vector<string> vTransactionRecord = splitStringRecordToVectorData(transactionLineRecord);
	
	if (vTransactionRecord.size() != 6 || vTransactionRecord.empty())
		return transaction;
	
	transaction.trxID = vTransactionRecord[0];
	transaction.accNumber = vTransactionRecord[1];
	transaction.transactionType = vTransactionRecord[2];
	
	try
	{
		transaction.amount = stod(vTransactionRecord[3]);
	}
	catch (...)
	{
		transaction.amount = 0.f;
	}

	transaction.transactionDateTime = vTransactionRecord[4];
	transaction.performedByUser = vTransactionRecord[5];
	
	return transaction;
}

string convertTransactionDataToStringRecord(const stTransaction& transaction)
{
	string record = "";

	record += transaction.trxID + delim;
	record += transaction.accNumber + delim;
	record += transaction.transactionType + delim;
	record += to_string(transaction.amount) + delim;
	record += transaction.transactionDateTime + delim;
	record += transaction.performedByUser;
	
	return record;
}

vector<stTransaction> getAllTransactions()
{
	ifstream myFlie(transactionsLogFileName);
	vector<stTransaction> vAllTransactions{};

	if (myFlie.is_open())
	{
		stTransaction transaction{};
		string transactionLine = "";
		
		while (getline(myFlie, transactionLine))
		{
			transaction = convertTransactionLineToStruct(transactionLine);
			
			if(!transaction.accNumber.empty())
				vAllTransactions.push_back(transaction);
		}
		myFlie.close();
	}

	return vAllTransactions;
}

void insertLogTransaction(vector<stTransaction>& vAllTransactions)
{
	ofstream temp("temp.txt", ios::out);

	if(!temp)
	{
		cout << right << setw(5) << " " << "Error Oppening File!\n";
		return;
	}

	if (temp.is_open())
	{
		for(const stTransaction& t : vAllTransactions)
			temp << convertTransactionDataToStringRecord(t) << endl;
	}

	temp.close();

	remove(transactionsLogFileName.c_str());

	if(rename("temp.txt", transactionsLogFileName.c_str()))
		cout << right << setw(5) << " " << "Error in updating transactions log file." << endl;
	else
		cout << endl << right << setw(5) << " " << "Transactions log file updated successfully." << endl;
}

string getCurrentDateTime()
{
	time_t t = time(0);
	tm* now = localtime(&t);

	string dateTime = to_string(now->tm_mday) + "/" + to_string(1 + now->tm_mon) + "/" + to_string(1900 + now->tm_year) + " - " + to_string(now->tm_hour) + ":" + to_string(now->tm_min) + ":" + to_string(now->tm_sec);
	
	return dateTime;
}

stActionLog convertActionLogLineToStruct(const string& actionDescription)
{
	stActionLog actionLog{};
	vector<string> vActionLogRecord = splitStringRecordToVectorData(actionDescription);

	if (vActionLogRecord.size() != 3 || vActionLogRecord.empty())
		return actionLog;
	
	actionLog.action = vActionLogRecord[0];
	actionLog.userName = vActionLogRecord[1];
	actionLog.actionDateTime = vActionLogRecord[2];
	
	return actionLog;
}

string convertActionLogDataToStringRecord(const stActionLog& actionLog)
{
	string record = "";
	
	record += actionLog.action + delim;
	record += actionLog.userName + delim;
	record += actionLog.actionDateTime;
	
	return record;
}

vector<stActionLog> getAllActionLogs()
{
	ifstream myFlie(actionLogsFileName);
	vector<stActionLog> vAllActionLogs{};
	
	if (myFlie.is_open())
	{
		stActionLog actionLog{};
		string actionLogLine = "";
		
		while (getline(myFlie, actionLogLine))
		{
			actionLog = convertActionLogLineToStruct(actionLogLine);
			
			if(!actionLog.action.empty())
				vAllActionLogs.push_back(actionLog);
		}
		myFlie.close();
	}
	
	return vAllActionLogs;
}

stActionLog createActionLog(const string& userName, const string& action)
{
	stActionLog log{};

	log.userName = userName;
	log.action = action;
	log.actionDateTime = getCurrentDateTime();

	return log;
}

stTransfer convertTransferLineRecordToStruct(string& record)
{
	stTransfer transfer{};
	vector<string> vData = splitStringRecordToVectorData(record);

	if (vData.empty() || vData.size() > 6)
		return transfer;

	transfer.TransferID = vData[0];
	transfer.fromAccNum = vData[1];
	transfer.toAccNum = vData[2];
	try
	{
		transfer.amount = stod(vData[3]);
	}
	catch (...)
	{
		transfer.amount = 0;
	}
	transfer.performedByUser = vData[4];
	transfer.transferDateTime = vData[5];

	return transfer;
}

string convertTransferStructToLineRecord(const stTransfer& t)
{
	string line = "";

	line += t.TransferID + delim;
	line += t.fromAccNum + delim;
	line += t.toAccNum + delim;
	line += to_string(t.amount) + delim;
	line += t.performedByUser + delim;
	line += t.transferDateTime;

	return line;
}

void insertTransfers(const vector<stTransfer>& vAllTransfers)
{
	ofstream myfile("temp.txt", ios::out);

	if (!myfile)
	{
		cout << right << setw(5) << " " << "Error openning the file try again later ..." << endl;
		return;
	}

	if (myfile.is_open())
	{
		string lineRecord = "";

		for (const stTransfer& t : vAllTransfers)
		{
			lineRecord = convertTransferStructToLineRecord(t);
			myfile << lineRecord << endl;
		}

		myfile.close();
	}

	remove(transferFileName.c_str());
	if (!rename("temp.txt", transferFileName.c_str()))
		cout << right << setw(5) << " " << "Transfer File Updated Successfully." << endl;
	else
		cout << right << setw(5) << " " << "Error Updated Transfer File." << endl;
}

vector<stTransfer> getAllTransfers()
{
	ifstream myFile(transferFileName);
	vector<stTransfer> vAllTransfers{};

	if (myFile.is_open())
	{
		string lineRecord = "";
		stTransfer transfer{};

		while (getline(myFile, lineRecord))
		{
			transfer = convertTransferLineRecordToStruct(lineRecord);
			
			if(!transfer.fromAccNum.empty())
				vAllTransfers.push_back(transfer);
		}

		myFile.close();
	}

	return vAllTransfers;
}

stSystem loadSystemInfo()
{
	stSystem systemInfo{};
 
	systemInfo.vAllClients = getAllClients();
	systemInfo.vAllUsers = getAllUsers();
	systemInfo.vAllTransactions = getAllTransactions();
	systemInfo.vAllTransfers = getAllTransfers();
	systemInfo.vAllActionLogs = getAllActionLogs();
	
	return systemInfo;
}

void insertActionLog(const stActionLog& log)
{
	ofstream temp("temp.txt", ios::app);
	
	if(!temp)
	{
		cout << right << setw(5) << " " << "Error Oppening File!\n";
		return;
	}
	
	if (temp.is_open())
	{
		vector<stActionLog> vAllActionLogs = getAllActionLogs();
		
		for(const stActionLog& l : vAllActionLogs)
			temp << convertActionLogDataToStringRecord(l) << endl;
		temp << convertActionLogDataToStringRecord(log) << endl;
	}

	temp.close();
	remove(actionLogsFileName.c_str());
	
	if(rename("temp.txt", actionLogsFileName.c_str()))
		cout << endl << right << setw(5) << " " << "Error in updating action logs file." << endl;
	else
		cout << endl << right << setw(5) << " " << "Action logs file updated successfully." << endl;
}

int main()
{
	
	srand((unsigned)time(NULL));
	stSystem systemInfo = loadSystemInfo();

	startBankApp(systemInfo);

	return 0;
};