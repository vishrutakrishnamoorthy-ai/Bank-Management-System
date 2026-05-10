// BANK MANAGEMENT SYSTEM - MINI PROJECT
// File: trans.c
// Uses: clients.dat (random access binary file)
// Output: accounts.txt (formatted text file)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        // Create new file with 100 blank records
        if ((cfPtr = fopen(filename, "wb")) == NULL)
        {
            printf("Error: Cannot create %s\n", filename);
            exit(-1);
        }
        
        // Write 100 blank records (100 accounts)
        struct clientData blank = {0, "", "", 0.0};
        for (int i = 0; i < 100; i++)
        {
            fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
        }
        fclose(cfPtr);
        
        // Reopen in rb+ mode
        if ((cfPtr = fopen(filename, "rb+")) == NULL)
        {
            printf("Error: Cannot open %s\n", filename);
            exit(-1);
        }
        printf("%s created with 100 accounts.\n", filename);
    }

    // Main menu loop
    while ((choice = enterChoice()) != 6)
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
            default:
                printf("Invalid choice! Please enter 1-6.\n");
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

    while (getchar() != '\n'); // clear input buffer

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
    }
    else
    {
        printf("Error deleting account!\n");
    }
}

// 4. Deposit money
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
    printf("Enter deposit amount: $");
    scanf("%lf", &amount);
    while (getchar() != '\n');

    if (amount <= 0)
    {
        printf("Invalid deposit amount!\n");
        return;
    }

    client.balance += amount;
    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);

    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        printf("*** NEW BALANCE: $%.2f ***\n", client.balance);
    }
    else
    {
        printf("Error processing deposit!\n");
    }
}

// 5. Withdraw money
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
        printf("Insufficient balance!\n");
        return;
    }

    client.balance -= amount;
    fseek(fPtr, -(long)sizeof(struct clientData), SEEK_CUR);

    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        printf("*** NEW BALANCE: $%.2f ***\n", client.balance);
    }
    else
    {
        printf("Error processing withdrawal!\n");
    }
}

// Find first empty record
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

// Display menu and get choice
unsigned int enterChoice(void)
{
    unsigned int choice;

    printf("\n=== MAIN MENU ===\n");
    printf("1. List accounts to accounts.txt\n");
    printf("2. Deposit money\n");
    printf("3. Withdraw money\n");
    printf("4. Add new account\n");
    printf("5. Delete account\n");
    printf("6. Exit\n");
    printf("Choice (1-6): ");

    scanf("%u", &choice);
    while (getchar() != '\n'); // clear input buffer
    return choice;
}