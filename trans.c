// BANK MANAGEMENT SYSTEM - MINI PROJECT
// File: trans.c
// Uses: clients.dat (random access binary file)
// Output: accounts.txt (formatted text file), transactions.txt (transaction log)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// clientData structure definition (fixed size for random access)
struct clientData
{
    unsigned int acctNum;     // account number (1-100)
    char lastName[15];        // account last name
    char firstName[10];       // account first name
    double balance;           // account balance
};

// Function prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
int findEmptyRecord(FILE *fPtr);
void depositRecord(FILE *fPtr);
void withdrawRecord(FILE *fPtr);
void transferMoney(FILE *fPtr);
void saveTransaction(int accNum, char type[], double amount, double balance);
void showTransactionHistory(void);
void showAccountHistory(int accountNum, FILE *fPtr);

// Get current timestamp
void getTimestamp(char *timestamp)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", t);
}

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // clients.dat file pointer
    unsigned int choice; // user's choice
    char filename[] = "clients.dat";

    printf("=== BANK MANAGEMENT SYSTEM ===\n");
    printf("=============================\n");

    // Open clients.dat in read/write binary mode
    if ((cfPtr = fopen(filename, "rb+")) == NULL)
    {
        printf("Creating %s file...\n", filename);
        if ((cfPtr = fopen(filename, "wb")) == NULL)
        {
            printf("Error: Cannot create %s\n", filename);
            exit(-1);
        }
        
        struct clientData blank = {0, "", "", 0.0};
        for (int i = 0; i < 100; i++)
        {
            fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
        }
        fclose(cfPtr);
        
        if ((cfPtr = fopen(filename, "rb+")) == NULL)
        {
            printf("Error: Cannot open %s\n", filename);
            exit(-1);
        }
        printf("%s created with 100 accounts.\n", filename);
    }

    // Main menu loop
    while ((choice = enterChoice()) != 8)
    {
        switch(choice)
        {
            case 1:
                textFile(cfPtr);
                break;
            case 2:
                depositRecord(cfPtr);
                break;
            case 3:
                withdrawRecord(cfPtr);
                break;
            case 4:
                newRecord(cfPtr);
                break;
            case 5:
                deleteRecord(cfPtr);
                break;
            case 6:
                showTransactionHistory();
                break;
            case 7:
                transferMoney(cfPtr);
                break;
            default:
                printf("Invalid choice! Please enter 1-7.\n");
                break;
        }
        printf("\n");
    }

    fclose(cfPtr);
    printf("Thank you for using Bank Management System!\n");
    return 0;
}

// 1. Create formatted text file (accounts.txt)
void textFile(FILE *readPtr)
{
    FILE *writePtr;
    struct clientData client = {0, "", "", 0.0};

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("Error creating accounts.txt");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%12s\n", 
            "ACCT", "LAST NAME", "FIRST NAME", "BALANCE");
    fprintf(writePtr, "%-6s%-16s%-11s%12s\n", 
            "----", "----------", "----------", "-------");

    int count = 0;
    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6d%-16s%-11s%12.2f\n", 
                    client.acctNum, client.lastName, client.firstName, client.balance);
            count++;
        }
    }

    fclose(writePtr);
    printf("accounts.txt created! %d active accounts.\n", count);
}

// 2. Add new account
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int accountNum;

    accountNum = findEmptyRecord(fPtr);
    if (accountNum == -1)
    {
        printf("All accounts are full.\n");
        return;
    }

    printf("Next available account number: %d\n", accountNum);

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading slot #%d\n", accountNum);
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account #%d already exists!\n", client.acctNum);
        return;
    }

    printf("Enter lastname (max 14 chars): ");
    scanf("%14s", client.lastName);
    printf("Enter firstname (max 9 chars): ");
    scanf("%9s", client.firstName);
    printf("Enter initial balance: $");
    scanf("%lf", &client.balance);

    while (getchar() != '\n');

    if (client.balance < 0)
    {
        printf("Error: Balance cannot be negative.\n");
        return;
    }

    client.acctNum = accountNum;
    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);

    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        printf("*** Account %d created successfully! ***\n", accountNum);
        printf("Name: %s %s, Balance: $%.2f\n", 
               client.firstName, client.lastName, client.balance);
        saveTransaction(accountNum, "ACCOUNT CREATED", client.balance, client.balance);
    }
    else
    {
        printf("Error creating account!\n");
    }
}

// 3. Delete account
void deleteRecord(FILE *fPtr)
{
    struct clientData client, blank = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter account number to delete (1-100): ");
    scanf("%u", &accountNum);
    while (getchar() != '\n');

    if (accountNum < 1 || accountNum > 100)
    {
        printf("Error: Account must be 1-100\n");
        return;
    }

    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading account #%d\n", accountNum);
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account %d does not exist!\n", accountNum);
        return;
    }

    printf("Deleting Account #%d: %s %s (Balance: $%.2f)\n", 
           client.acctNum, client.firstName, client.lastName, client.balance);

    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);
    if (fwrite(&blank, sizeof(struct clientData), 1, fPtr) == 1)
    {
        printf("*** Account %d deleted successfully! ***\n", accountNum);
        saveTransaction(accountNum, "ACCOUNT DELETED", client.balance, 0.0);
    }
    else
    {
        printf("Error deleting account!\n");
    }
}

// 4. Find first empty record
int findEmptyRecord(FILE *fPtr)
{
    struct clientData client;

    rewind(fPtr);

    for (int i = 0; i < 100; i++)
    {
        if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
            return -1;

        if (client.acctNum == 0)
            return i + 1;
    }

    return -1;
}

// 5. Save transaction with timestamp
void saveTransaction(int accNum, char type[], double amount, double balance)
{
    FILE *tfPtr;
    char timestamp[20];

    getTimestamp(timestamp);
    
    if ((tfPtr = fopen("transactions.txt", "a")) == NULL)
    {
        printf("Unable to open transaction file!\n");
        return;
    }

    fprintf(tfPtr, "[%s] Account: %03d | %s | $%10.2f | Balance: $%10.2f\n",
            timestamp, accNum, type, amount, balance);

    fclose(tfPtr);
}

// 6. Show complete transaction history
void showTransactionHistory(void)
{
    FILE *tfPtr;
    char line[256];
    int count = 0;

    printf("\n=== TRANSACTION HISTORY ===\n");
    printf("============================\n");

    if ((tfPtr = fopen("transactions.txt", "r")) == NULL)
    {
        printf("No transactions recorded yet!\n");
        printf("============================\n");
        return;
    }

    while (fgets(line, sizeof(line), tfPtr) != NULL)
    {
        printf("%s", line);
        count++;
    }

    fclose(tfPtr);
    
    printf("============================\n");
    printf("Total transactions: %d\n", count);
}

// 7. Deposit money
void depositRecord(FILE *fPtr)
{
    struct clientData client = {0};
    unsigned int account;
    double amount;

    printf("Enter account number (1-100): ");
    scanf("%u", &account);
    while (getchar() != '\n');

    if (account < 1 || account > 100)
    {
        printf("Error: Account must be 1-100\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);

    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading account #%d\n", account);
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account #%d does not exist!\n", account);
        return;
    }

    printf("\n*** CURRENT BALANCE: $%.2f ***\n", client.balance);
    printf("Account: %s %s\n", client.firstName, client.lastName);

    printf("Enter deposit amount: $");
    scanf("%lf", &amount);
    while (getchar() != '\n');

    if (amount <= 0)
    {
        printf("Invalid deposit amount!\n");
        return;
    }

    double oldBalance = client.balance;
    client.balance += amount;

    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);

    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        saveTransaction(client.acctNum, "DEPOSIT", amount, client.balance);
        printf("*** DEPOSIT SUCCESSFUL! ***\n");
        printf("Deposited: $%.2f\n", amount);
        printf("New Balance: $%.2f\n", client.balance);
    }
    else
    {
        printf("Error processing deposit!\n");
    }
}

// 8. Withdraw money
void withdrawRecord(FILE *fPtr)
{
    struct clientData client = {0};
    unsigned int account;
    double amount;

    printf("Enter account number (1-100): ");
    scanf("%u", &account);
    while (getchar() != '\n');

    if (account < 1 || account > 100)
    {
        printf("Error: Account must be 1-100\n");
        return;
    }

    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);

    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        printf("Error reading account #%d\n", account);
        return;
    }

    if (client.acctNum == 0)
    {
        printf("Account #%d does not exist!\n", account);
        return;
    }

    printf("\n*** CURRENT BALANCE: $%.2f ***\n", client.balance);
    printf("Account: %s %s\n", client.firstName, client.lastName);

    printf("Enter withdrawal amount: $");
    scanf("%lf", &amount);
    while (getchar() != '\n');

    if (amount <= 0)
    {
        printf("Invalid withdrawal amount!\n");
        return;
    }

    if (amount > client.balance)
    {
        printf("Insufficient balance! Available: $%.2f\n", client.balance);
        return;
    }

    double oldBalance = client.balance;
    client.balance -= amount;

    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);

    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        saveTransaction(client.acctNum, "WITHDRAWAL", amount, client.balance);
        printf("*** WITHDRAWAL SUCCESSFUL! ***\n");
        printf("Withdrawn: $%.2f\n", amount);
        printf("New Balance: $%.2f\n", client.balance);
    }
    else
    {
        printf("Error processing withdrawal!\n");
    }
}

void transferMoney(FILE *fPtr)
{
    struct clientData sender = {0};
    struct clientData receiver = {0};

    unsigned int senderAcc;
    unsigned int receiverAcc;

    double amount;

    printf("\n=== MONEY TRANSFER ===\n");

    printf("Enter sender account number: ");
    scanf("%u", &senderAcc);

    printf("Enter receiver account number: ");
    scanf("%u", &receiverAcc);

    while (getchar() != '\n');

    if (senderAcc == receiverAcc)
    {
        printf("Cannot transfer to same account!\n");
        return;
    }

    if (senderAcc < 1 || senderAcc > 100 ||
        receiverAcc < 1 || receiverAcc > 100)
    {
        printf("Invalid account number!\n");
        return;
    }

    printf("Enter transfer amount: $");
    scanf("%lf", &amount);

    while (getchar() != '\n');

    if (amount <= 0)
    {
        printf("Invalid transfer amount!\n");
        return;
    }

    // Read sender
    fseek(fPtr,
          (senderAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    if (fread(&sender,
              sizeof(struct clientData),
              1,
              fPtr) != 1)
    {
        printf("Error reading sender account!\n");
        return;
    }

    if (sender.acctNum == 0)
    {
        printf("Sender account does not exist!\n");
        return;
    }

    // Read receiver
    fseek(fPtr,
          (receiverAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    if (fread(&receiver,
              sizeof(struct clientData),
              1,
              fPtr) != 1)
    {
        printf("Error reading receiver account!\n");
        return;
    }

    if (receiver.acctNum == 0)
    {
        printf("Receiver account does not exist!\n");
        return;
    }

    if (amount > sender.balance)
    {
        printf("Insufficient balance!\n");
        return;
    }

    // Transfer
    sender.balance -= amount;
    receiver.balance += amount;

    // Save sender
    fseek(fPtr,
          (senderAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&sender,
           sizeof(struct clientData),
           1,
           fPtr);

    // Save receiver
    fseek(fPtr,
          (receiverAcc - 1) * sizeof(struct clientData),
          SEEK_SET);

    fwrite(&receiver,
           sizeof(struct clientData),
           1,
           fPtr);

    // Log transactions
    saveTransaction(sender.acctNum,
                    "TRANSFER SENT",
                    amount,
                    sender.balance);

    saveTransaction(receiver.acctNum,
                    "TRANSFER RECEIVED",
                    amount,
                    receiver.balance);

    printf("\n*** TRANSFER SUCCESSFUL! ***\n");

    printf("Transferred $%.2f from Account %u to Account %u\n",
           amount,
           senderAcc,
           receiverAcc);

    printf("Sender Balance: $%.2f\n",
           sender.balance);

    printf("Receiver Balance: $%.2f\n",
           receiver.balance);
}

// 9. Display menu and get choice
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\n=== MAIN MENU ===\n");
    printf("1. List accounts to accounts.txt\n");
    printf("2. Deposit money\n");
    printf("3. Withdraw money\n");
    printf("4. Add new account\n");
    printf("5. Delete account\n");
    printf("6. View transaction history\n");
    printf("7. Transfer money\n");
    printf("8. Exit\n");
    printf("Choice (1-7): ");

    scanf("%u", &choice);
    while (getchar() != '\n');
    return choice;
}