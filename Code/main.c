#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct user {
    char username[50];
    unsigned long passwordHash;
};

struct budgetEntry{
    char date[30]; // DD-MM-YYYY e.g: 27-11-2025
    float amount; 
    char type[30];
};


void homeMenu(struct user users[], int *usercount);
void dashboardMenu(char *username);

void loadCredentials(struct user users[], int *usercount);
void saveCredentialToFile(struct user u);
void getUserRecordFile(char *username, char *filepath);

void registration(struct user users[], int *usercount);
void login(struct user users[], int usercount);

// security algorithms
unsigned long hashString(const char *str);
float encryptAmount(float amount);
float decryptAmount(float encrypted);

void deleteRecord(char *username);
void editRecord(char *username);
void viewRecord(char *username);
void addRecord(char *username);
void sentAlert(char *username);
void sentMonthlyReport(char *username);



void exitMenu();
void invalidChoice();

int main() {

    struct user users[100];
    int usercount = 0;

    loadCredentials(users, &usercount);


    homeMenu(users,&usercount);

    return 0;
}

void homeMenu(struct user users[], int *usercount) {
    int choice;

    do {
        printf("\n\tHOME MENU\t\n");
        printf("0. Exit\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 0:exitMenu();break;
            case 1:login(users, *usercount);break;
            case 2:registration(users, usercount);break;
            default:invalidChoice(); break;
        }
    } while(choice != 0);
    
}

// Load credentials from file into struct array
void loadCredentials(struct user users[], int *usercount) {

    FILE *fptr = fopen("credentials/cred.txt", "r");
    if (fptr == NULL) {
        fptr = fopen("credentials/cred.txt", "w");
        fclose(fptr);
        return;
    }

    while (fscanf(fptr, "%s %lu", 
                  users[*usercount].username, 
                  &users[*usercount].passwordHash) != EOF) 
    {
        (*usercount)++;
    }

    fclose(fptr);
}

void saveCredentialToFile(struct user u) {

    FILE *fp = fopen("credentials/cred.txt", "a");
    if (fp == NULL) {
        printf("Error opening credentials file!\n");
        return;
    }

    fprintf(fp, "%s %lu\n", u.username, u.passwordHash);

    fclose(fp);
}

void registration(struct user users[], int *usercount) {

    printf("\n\tREGISTRATION\t\n");

    int duplicate;
    do {
        duplicate = 0;
        printf("Enter username: ");
        scanf("%s", users[*usercount].username);

        // Check duplicate
        for(int i = 0; i < *usercount; i++) {
            if(strcmp(users[i].username, users[*usercount].username) == 0) {
                printf("Username already exists! Try another.\n");
                duplicate = 1;
                break;
            }
        }
    } while(duplicate == 1);

    // Password input + validation
    char password[50];
    int length;
    do {
        printf("Enter password (min 4 alpha numeric characters): ");
        scanf("%s", password);
        length = strlen(password);

        if (length < 4) {
            printf("Password too short! Try again.\n");
        }
    } while(length < 4);

    // Hash password
    unsigned long hashed = hashString(password);
    users[*usercount].passwordHash = hashed;

    // Save to file
    saveCredentialToFile(users[*usercount]);

    printf("Registration successful!\n");
    (*usercount)++;
}

void login(struct user users[], int usercount) {
    int found = 0;
    int wantToRegister =0;
    do{
        printf("\n\tLOGIN\t\n");

        char uname[50], pword[50];

        printf("Enter username: ");
        scanf("%s", uname);

        printf("Enter password: ");
        scanf("%s", pword);

        unsigned long hashed = hashString(pword);

        for (int i = 0; i < usercount; i++) {

            if(strcmp(users[i].username, uname) == 0 &&
                users[i].passwordHash == hashed) 
            {
                printf("Login successful!\n");
                found = 1;
                dashboardMenu(uname); 
                break;
            }
        }

        if (found==0) {
            printf("Invalid username or password.\n");
            printf("Do you want to register? (1 - Yes, 0 - No): ");
            scanf("%d", &wantToRegister);
            if (wantToRegister==1){
                registration(users, &usercount);
            }
        }
        
    } while (found==0);
}

unsigned long hashString(const char *str) {
    unsigned long hash = 5381;   
    int c;
    while (*str != '\0') {
        c= *str;
        hash = ((hash << 5) + hash) + c;  // hash * 33 + 97
        str++;                     
    }
    return hash;
}

void dashboardMenu(char *username) {
    int choice;

    do {
        printf("\n\tDASHBOARD (%s)\t\n", username);
        printf("1. View Records\n");
        printf("2. Add Record\n");
        printf("3. Edit Record\n");
        printf("4. Delete Record\n");
        printf("5. Send Monthly Report\n");
        printf("6. Send Alert\n");
        printf("0. Exit\n");

        printf("Enter choice(0-6): ");

        scanf("%d", &choice);


        switch(choice) {
            case 0: exitMenu();break;
            case 1: viewRecord(username); break;
            case 2: addRecord(username); break;
            case 3: editRecord(username); break;
            case 4: deleteRecord(username); break;
            case 5: sentMonthlyReport(username); break;
            case 6: sentAlert(username); break;
            default: invalidChoice();break;
          
        }

    } while(choice != 0);
}


void exitMenu() {
    printf("\n\tExited\n");
    exit(1);
    
}

void invalidChoice() {
    printf("Invalid choice! Try again.\n");
}

void getUserRecordFile(char *username, char *filepath) {
    sprintf(filepath, "records/%s.txt", username);
}

void addRecord(char *username) {
    struct budgetEntry p;
    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "a");
    if (!fp) {
        printf("Error opening records file!\n");
        return;
    }


    printf("Enter date (dd-mm-yyyy): ");
    scanf("%s", p.date);

    printf("Enter amount: ");
    scanf("%f", &p.amount);

    printf("Enter type (income or expense): ");
    getchar();  

    fgets(p.type, sizeof(p.type), stdin);
    p.type[strcspn(p.type, "\n")] = 0;

    float encryptedAmount = encryptAmount(p.amount);
    fprintf(fp, "%s %.2f %s\n", p.date, encryptedAmount, p.type);


    //fprintf(fp, "%s %.2f %s\n", p.date, p.amount, p.type);

    printf("Record added!\n");
    fclose(fp);
}

void viewRecord(char *username) {
    struct budgetEntry p;
    int i=1;
    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("\nNo records found!\n");
        return;
    }

    printf("\n\tYour Records \t\n");
    while (fscanf(fp, "%s %f %[^\n]",
                  &p.date, &p.amount, p.type) != EOF) 
    {
        float amount = decryptAmount(p.amount);


        printf("S.no: %d | Date: %s | Amount: %.2f | type: %s\n",
               i, p.date, amount, p.type); 
        i++;
    }


    fclose(fp);
}

void editRecord(char *username) {
    struct budgetEntry arr[500];
    int count = 0, recordSno, found = 0;

    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records to edit!\n");
        return;
    }

    // Read + decrypt amounts
    while (fscanf(fp, "%s %f %[^\n]",
                  arr[count].date,
                  &arr[count].amount,
                  arr[count].type) != EOF) 
    {
        arr[count].amount = decryptAmount(arr[count].amount);
        count++;
    }
    fclose(fp);

    viewRecord(username);

    printf("Enter S.no to edit that specific record: ");
    scanf("%d", &recordSno);

    if (recordSno < 1 || recordSno > count) {
        printf("Record not found!\n");
        return;
    }

    int i = recordSno - 1;

    printf("Enter new date: ");
    scanf("%s", arr[i].date);

    printf("Enter new amount: ");
    scanf("%f", &arr[i].amount);

    printf("Enter new type (income or expense): ");
    getchar();
    fgets(arr[i].type, sizeof(arr[i].type), stdin);
    arr[i].type[strcspn(arr[i].type, "\n")] = 0;

    // Write back (encrypting amounts)
    fp = fopen(filepath, "w");
    for (int j = 0; j < count; j++) {

        float encrypted = encryptAmount(arr[j].amount);

        fprintf(fp, "%s %.2f %s\n",
                arr[j].date, encrypted, arr[j].type);
    }

    fclose(fp);
    printf("Record updated!\n");
}

void deleteRecord(char *username) {
    struct budgetEntry arr[500];
    int count = 0, recordSno, found = 0;

    char filepath[100];
    getUserRecordFile(username, filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records to delete!\n");
        return;
    }

    // Read + decrypt amounts
    while (fscanf(fp, "%s %f %[^\n]",
                  arr[count].date,
                  &arr[count].amount,
                  arr[count].type) != EOF) 
    {
        arr[count].amount = decryptAmount(arr[count].amount);
        count++;
    }
    fclose(fp);

    viewRecord(username);

    printf("Enter S.no to delete that specific record: ");
    scanf("%d", &recordSno);

    if (recordSno < 1 || recordSno > count) {
        printf("Record not found!\n");
        return;
    }

    fp = fopen(filepath, "w");

    for (int i = 0; i < count; i++) {

        if (i == recordSno - 1) {
            found = 1;
            continue;  
        }

        float encrypted = encryptAmount(arr[i].amount);

        fprintf(fp, "%s %.2f %s\n",
                arr[i].date, encrypted, arr[i].type);
    }

    fclose(fp);

    if (found)
        printf("Record deleted!\n");
    else
        printf("Record not found!\n");
}

void sentMonthlyReport(char *username) {
    char monthNum[3];
    printf("Enter month (e.g., 11 for November): ");
    scanf("%s", monthNum);

    struct budgetEntry p;
    float totalIncome = 0, totalExpense = 0;

    char filepath[100];
    sprintf(filepath, "records/%s.txt", username);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records found!\n");
        return;
    }

    while (fscanf(fp, "%s %f %s", p.date, &p.amount, p.type) != EOF) {
        p.amount = decryptAmount(p.amount);

        if (strncmp(p.date + 3, monthNum, 2) == 0) {  // check month part
            if (strcmp(p.type, "income") == 0)
                totalIncome += p.amount;
            else if (strcmp(p.type, "expense") == 0)
                totalExpense += p.amount;
        }
    }

    fclose(fp);

    char month[30];

    if (strcmp(monthNum, "1") == 0) strcpy(month, "January");
    else if (strcmp(monthNum, "2") == 0) strcpy(month, "February");
    else if (strcmp(monthNum, "3") == 0) strcpy(month, "March");
    else if (strcmp(monthNum, "4") == 0) strcpy(month, "April");
    else if (strcmp(monthNum, "5") == 0) strcpy(month, "May");
    else if (strcmp(monthNum, "6") == 0) strcpy(month, "June");
    else if (strcmp(monthNum, "7") == 0) strcpy(month, "July");
    else if (strcmp(monthNum, "8") == 0) strcpy(month, "August");
    else if (strcmp(monthNum, "9") == 0) strcpy(month, "September");
    else if (strcmp(monthNum, "10") == 0) strcpy(month, "October");
    else if (strcmp(monthNum, "11") == 0) strcpy(month, "November");
    else if (strcmp(monthNum, "12") == 0) strcpy(month, "December");
    else strcpy(month, "Invalid");


    printf("\n\t Monthly Report for %s \t\n", month);
    printf("Total Income  : %.2f\n", totalIncome);
    printf("Total Expense : %.2f\n", totalExpense);
    printf("Net Balance   : %.2f\n", totalIncome - totalExpense);
}

void sentAlert(char *username) {
    float limit;
    printf("Enter expense limit for alert: ");
    scanf("%f", &limit);

    struct budgetEntry p;
    float totalExpense = 0;

    char filepath[100];
    sprintf(filepath, "records/%s.txt", username);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("No records found!\n");
        return;
    }

    while (fscanf(fp, "%s %f %s", p.date, &p.amount, p.type) != EOF) {
        if (strcmp(p.type, "expense") == 0)
            totalExpense += p.amount;
    }

    fclose(fp);

    if (totalExpense > limit)
        printf("\nALERT: Your total expense of %.2f exceeded the limit %.2f\n", totalExpense, limit);
    else
        printf("Total expense of %.2f is within the limit %.2f\n", totalExpense, limit);
}

float encryptAmount(float amount) {
    return (amount *4) + 147.59;   
}

float decryptAmount(float encrypted) {
    return (encrypted-147.59) / 4;  
}

