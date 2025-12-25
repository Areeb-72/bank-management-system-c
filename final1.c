#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

struct client
{
    char name[50];
    char cnic[50];
    char dob[50];

    double loan_amount;

    int balance;
    int pin;
    int is_employee; // 1 = employee, 0 = customer
    int emp_years;   // years of service (only if employee)
    int daily;       // daily transferred amount
    int last_day;    // day of last transfer (1-31)
};

int get_day()
{
    time_t t = time(NULL);         // Get the current time and store it in t 
    struct tm tm = *localtime(&t); // structure that stores time in human readable form , local time Converts the time t into local date & time
    return tm.tm_mday;             // member of struct tm, returns day of month (1-31)
}

void save_transaction(char sender[], char sender_cnic[],
                      char receiver[], char receiver_cnic[],
                      int amount)
{
    FILE *fp = fopen("transactions.txt", "a"); // append
    if (!fp)
    {
        printf("Error saving transaction!\n");
        return;
    }

    // Get current time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    fprintf(fp,
            "Date: %02d-%02d-%04d | Time: %02d:%02d:%02d | Sender: %s (%s) | Receiver: %s (%s) | Amount: %d Rs\n",
            tm.tm_mday,
            tm.tm_mon + 1,
            tm.tm_year + 1900,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            sender,
            sender_cnic,
            receiver,
            receiver_cnic,
            amount);

    fclose(fp);
}

// This function Save all users to file
void save_users(struct client user[], int count)
{
    int i;
    FILE *fp = fopen("users.txt", "w"); // overwrite whole file
    if (!fp)
    {
        printf("Error opening file!\n");
        return;
    }

    fprintf(fp, "# Name,CNIC,DOB,Balance,PIN,Loan,Employee,Years,Daily,Day\n");

    for (i = 0; i < count; i++)
    {
        fprintf(fp, "%s,%s,%s,%d,%d,%.2lf,%d,%d,%d,%d\n",
                user[i].name,
                user[i].cnic,
                user[i].dob,
                user[i].balance,
                user[i].pin,
                user[i].loan_amount,
                user[i].is_employee,
                user[i].emp_years,
                user[i].daily,
                user[i].last_day);
    }
    fclose(fp);
}

// Load all users from file
int load_users(struct client user[])
{
    FILE *fp = fopen("users.txt", "r");
    if (!fp)
        return -1; // file does not exist
    char header[200];
    fgets(header, sizeof(header), fp);

    int i = 0;
    while (fscanf(fp, "%[^,],%[^,],%[^,],%d,%d,%lf,%d,%d,%d,%d\n",
                  user[i].name,
                  user[i].cnic,
                  user[i].dob,
                  &user[i].balance,
                  &user[i].pin,
                  &user[i].loan_amount,
                  &user[i].is_employee,
                  &user[i].emp_years,
                  &user[i].daily,
                  &user[i].last_day) == 10)
    {
        i++;
    }
    fclose(fp);
    return i; // total count
}

int DAILY_LIMIT = 50000; // set daily limit

void transfer_money(struct client user[], int count, int sender_index)
{
    char rec_cnic[50], input_str[20];
    int amount, pin, valid, receiver_index = -1;

    int today = get_day();
    if (user[sender_index].last_day != today)
    {
        user[sender_index].daily = 0; // reset daily transferred
        user[sender_index].last_day = today;
    }

    // Receiver CNIC
    printf("Enter Receiver CNIC: ");
    scanf("%s", rec_cnic);

    // Find receiver
    for (int i = 0; i < count; i++)
    {
        if (strcmp(user[i].cnic, rec_cnic) == 0)
        {
            receiver_index = i;
            break;
        }
    }

    if (receiver_index == -1)
    {
        printf("Receiver not found!\n");
        return;
    }
    if (receiver_index == sender_index)
    {
        printf("You cannot transfer money to yourself!\n");
        return;
    }

    // Amount validation and daily limit check
    do
    {
        valid = 1;
        printf("Enter amount to transfer: ");
        scanf(" %s", input_str);

        for (int i = 0; input_str[i]; i++)
        {
            if (!isdigit(input_str[i]))
            {
                printf("Amount must be numeric!\n");
                valid = 0;
                break;
            }
        }

        if (valid)
        {
            amount = atoi(input_str);
            if (amount <= 0)
            {
                valid = 0;
                printf("Amount must be greater than 0!\n");
            }
            else if (amount > user[sender_index].balance)
            {
                valid = 0;
                printf("Insufficient balance!\n");
            }
            else if (user[sender_index].daily + amount > DAILY_LIMIT)
            {
                valid = 0;
                printf("Transfer exceeds daily limit of %d Rs!\n", DAILY_LIMIT);
            }
        }
    } while (!valid);

    // PIN confirmation
    do
    {
        valid = 1;
        printf("Enter your 5-digit PIN to confirm transfer: ");
        scanf(" %s", input_str);

        for (int i = 0; input_str[i] != '\0'; i++)
        {
            if (!isdigit(input_str[i]))
            {
                printf("PIN must be numeric only!\n");
                valid = 0;
                break;
            }
        }

        if (valid)
        {
            pin = atoi(input_str);
            if (pin != user[sender_index].pin)
            {
                printf("Incorrect PIN!\n");
                valid = 0;
            }
        }
    } while (!valid);

    // Perform transfer
    user[sender_index].balance -= amount;
    user[receiver_index].balance += amount;
    user[sender_index].daily += amount;

    save_users(user, count);
    save_transaction(user[sender_index].name, user[sender_index].cnic,
                     user[receiver_index].name, user[receiver_index].cnic,
                     amount);

    printf("\nTransfer Successful!\n");
    printf("%s sent %d Rs to %s\n",
           user[sender_index].name, amount, user[receiver_index].name);
}

void loan(struct client user[], int m, int n) // m is index of logged-in user
{
    double amount, interest, installment;
    int principal, tenure, valid;
    char input_str[20];

    // Principal input validation
    do
    {
        valid = 1;
        printf("\nHow much loan do you want?\nAnswer: ");
        scanf(" %s", input_str);

        for (int i = 0; input_str[i] != '\0'; i++)
        {
            if (!isdigit(input_str[i]))
            {
                printf("Principal must be numeric only!\n");
                valid = 0;
                break;
            }
        }

        if (valid)
        {
            principal = atoi(input_str);
            if (principal <= 0)
            {
                printf("Principal must be greater than 0!\n");
                valid = 0;
            }
        }
    } while (!valid);

    // Tenure input validation 
    do
    {
        valid = 1;
        printf("Enter loan tenure in months: ");
        scanf(" %s", input_str);

        for (int i = 0; input_str[i] != '\0'; i++)
        {
            if (!isdigit(input_str[i]))
            {
                printf("Tenure must be numeric only!\n");
                valid = 0;
                break;
            }
        }

        if (valid)
        {
            tenure = atoi(input_str);
            if (tenure <= 0)
            {
                printf("Tenure must be greater than 0!\n");
                valid = 0;
            }
        }
    } while (!valid);

    if (user[m].is_employee == 1)
    {
        if (user[m].emp_years >= 5)
        {
            interest = 0.25 * principal;
            amount = principal + interest;
            user[m].loan_amount = amount;
            installment = amount / tenure;
            printf("Loan approved! Monthly installment: %.2lf Rs\n", installment);
        }
        else
            printf("Loan Not Approved!!: Less than 5 years service.\n");
    }
    else
    {
        if (user[m].balance >= 0.2 * principal)
        {
            interest = 0.25 * principal;
            amount = principal + interest;
            user[m].loan_amount = amount;
            installment = amount / tenure;
            printf("Loan approved! Monthly installment: %.2lf Rs\n", installment);
        }
        else
            printf("Loan Not Approved!!\nBalance insufficient\n");
    }
    save_users(user, n);
}

void signin(struct client user[], int n)
{
    int j;
    char testname[50];
    char input_str[20];
    int test, valid;

    printf("\nUserName: ");
    scanf(" %[^\n]", testname);

    // PIN validation during signin 
    do
    {
        valid = 1;
        printf("Enter PIN: ");
        scanf(" %s", input_str);
        for (int i = 0; input_str[i] != '\0'; i++)
        {
            if (!isdigit(input_str[i]))
            {
                printf("PIN must contain only numbers!\n");
                valid = 0;
                break;
            }
        }
        if (valid)
        {
            test = atoi(input_str);
            if (test < 10000 || test > 99999)
            {
                printf("PIN must be exactly 5 digits (10000–99999)!\n");
                valid = 0;
            }
        }
    } while (!valid);

    for (j = 0; j < n; j++) // check each user
    {
        if (test == user[j].pin && strcmp(testname, user[j].name) == 0)
        {
            printf("Account login successfully!!\n");

            while (1)
            {
                int choice2, amount;
                // menu choice validation
                do
                {
                    valid = 1;
                    printf("\nOptions:\n1. Withdraw\n2. Deposit\n3. Check Balance\n4. Loan\n5. Account Summary\n6. Transfer Money\n7. Log out\nChoice: ");
                    scanf("%s", input_str);
                    for (int i = 0; input_str[i] != '\0'; i++)
                    {
                        if (!isdigit(input_str[i]))
                        {
                            printf("Enter numbers only!\n");
                            valid = 0;
                            break;
                        }
                    }
                    if (valid)
                        choice2 = atoi(input_str);
                } while (!valid);

                switch (choice2)
                {
                case 1: // Withdraw
                    do
                    {
                        valid = 1;
                        printf("Withdraw Amount: ");
                        scanf("%s", input_str);
                        for (int i = 0; input_str[i] != '\0'; i++)
                        {
                            if (!isdigit(input_str[i]))
                            {
                                printf("Enter numbers only!\n");
                                valid = 0;
                                break;
                            }
                        }
                        if (valid)
                            amount = atoi(input_str);
                        if (valid && amount > user[j].balance)
                        {
                            printf("Insufficient balance!\n");
                            valid = 0;
                        }
                    } while (!valid);
                    user[j].balance -= amount;
                    printf("Balance after withdraw: %d Rs\n", user[j].balance);
                    save_users(user, n);
                    break;

                case 2: // Deposit
                    do
                    {
                        valid = 1;
                        printf("Deposit Amount: ");
                        scanf("%s", input_str);
                        for (int i = 0; input_str[i] != '\0'; i++)
                        {
                            if (!isdigit(input_str[i]))
                            {
                                printf("Enter numbers only!\n");
                                valid = 0;
                                break;
                            }
                        }
                        if (valid)
                            amount = atoi(input_str);
                        if (valid && amount < 0)
                        {
                            printf("Deposit cannot be negative!\n");
                            valid = 0;
                        }
                    } while (!valid);
                    user[j].balance += amount;
                    printf("Balance after deposit: %d Rs\n", user[j].balance);
                    save_users(user, n);
                    break;

                case 3:
                    printf("Current Balance: %d Rs\n", user[j].balance);
                    break;

                case 4:
                    loan(user, j, n); 
                    break;

                case 5:
                    printf("\n----- Account Summary -----\n");
                    printf("Name: %s\nCNIC: %s\nDOB: %s\nBalance: %d Rs\n",
                           user[j].name, user[j].cnic, user[j].dob, user[j].balance);
                    if (user[j].loan_amount > 0)
                        printf("Active Loan: %.2lf Rs\n", user[j].loan_amount);
                    else
                        printf("Active Loan: None\n");

                    // --- Recent Transactions ---
                    {
                        FILE *fp = fopen("transactions.txt", "r");
                        if (fp != NULL)
                        {
                            char line[200];
                            char user_transactions[100][200]; // store up to 100 transactions
                            int count = 0;

                            while (fgets(line, sizeof(line), fp))
                            {
                                if (strstr(line, user[j].cnic)) // only this user's transactions
                                {
                                    strcpy(user_transactions[count], line);
                                    count++;
                                }
                            }
                            fclose(fp);

                            printf("\nRecent Transactions:\n");

                            // print only last 3 transactions
                            int start = count - 3;
                            if (start < 0)
                                start = 0;

                            for (int i = count - 1; i >= start; i--)
                            {
                                printf("%s", user_transactions[i]);
                            }
                            printf("----------------------------\n");

                            if (count == 0)
                                printf("No transactions found.\n");
                        }
                        
                        else
                        {
                            printf("Error opening transactions.txt\n");
                        }
                    }
                    break;

                case 6:
                    transfer_money(user, n, j);
                    break;

                case 7:
                    printf("Logged out successfully!\n");
                    return;

                default:
                    printf("Invalid Input\n");
                    break;
                }
            }
        }
    }
    printf("PIN or Username not found in Bank's Database\n");
}

int main()
{
    struct client user[100];
    int user_count = load_users(user);
    printf("Loaded users: %d\n", user_count);
    if (user_count < 0)
        user_count = 0;
    for (int i = 0; i < user_count; i++)
    {
        if (user[i].loan_amount < 0)
            user[i].loan_amount = 0;
    }
    FILE *fp = fopen("transactions.txt", "a");
    if (fp != NULL)
        fclose(fp);

    printf("\t\t======== WELCOME to Nexus Bank ========\n");
    while (1)
    {
        char ch_str[10]; 
        int choice, valid;

        do
        {
            valid = 1;
            printf("\n1. Sign Up\n2. Sign In\n3. Exit\nChoice: ");
            scanf(" %s", ch_str);

            for (int i = 0; ch_str[i] != '\0'; i++)
            {
                if (!isdigit(ch_str[i]))
                {
                    printf("Enter numbers only!\n");
                    valid = 0;
                    break;
                }
            }

            if (valid)
            {
                choice = atoi(ch_str);
                if (choice < 1 || choice > 3)
                {
                    printf("Invalid choice!\n");
                    valid = 0;
                }
            }
        } while (!valid);

        switch (choice)
        {
        case 1:
            if (user_count >= 100)
            {
                printf("Unable to create account, User limit reached!\n");
                break;
            }
            char name[50];
            do
            {
                valid = 1;
                printf("UserName: ");
                scanf(" %[^\n]", name);

                // check only letters and spaces
                for (int i = 0; name[i] != '\0'; i++)
                {
                    if (!isalpha(name[i]) && name[i] != ' ')
                    {
                        printf("Name can contain only letters and spaces!\n");
                        valid = 0;
                        break;
                    }
                }
            } while (!valid);
            strcpy(user[user_count].name, name);

            char admin_str[10];
            int admin_code;

            do
            {
                valid = 1;
                printf("Enter admin code (enter 0 if you are not employee): ");
                scanf(" %s", admin_str);

                for (int i = 0; admin_str[i] != '\0'; i++)
                {
                    if (!isdigit(admin_str[i]))
                    {
                        printf("Admin code must be numeric!\n");
                        valid = 0;
                        break;
                    }
                }

                if (valid)
                    admin_code = atoi(admin_str);

            } while (!valid);

            if (admin_code == 1234)
            {
                user[user_count].is_employee = 1;
                do
                {
                    printf("Enter years of employment: ");
                    scanf("%d", &user[user_count].emp_years);

                    if (user[user_count].emp_years <= 0)
                        printf("Invalid years!\n");

                } while (user[user_count].emp_years <= 0);
            }
            else
            {
                user[user_count].is_employee = 0;
                user[user_count].emp_years = 0;
            }

            do
            {
                valid = 1;
                printf("CNIC (13 digits):");
                scanf(" %s", user[user_count].cnic);
                if (strlen(user[user_count].cnic) != 13)
                    valid = 0;
                else
                {
                    for (int k = 0; k < 13; k++)
                    {
                        if (!isdigit(user[user_count].cnic[k]))
                        {
                            valid = 0;
                            break;
                        }
                    }
                }
                if (!valid)
                {
                    if (strlen(user[user_count].cnic) != 13)
                        printf("CNIC must be exactly 13 characters long!\n");
                    else
                        printf("CNIC must contain only digits!\n");
                }
            } while (!valid);

            do
            {
                valid = 1;
                printf("D.O.B (DD/MM/YYYY): ");
                scanf(" %s", user[user_count].dob);

                if (strlen(user[user_count].dob) != 10 ||
                    user[user_count].dob[2] != '/' ||
                    user[user_count].dob[5] != '/')
                {
                    printf("DOB format must be DD/MM/YYYY!\n");
                    valid = 0;
                }
            } while (!valid);

            char pin_str[10];
            do
            {
                valid = 1;
                printf("PIN (5 digits): ");
                scanf("%s", pin_str);

                for (int i = 0; pin_str[i] != '\0'; i++)
                {
                    if (!isdigit(pin_str[i]))
                    {
                        valid = 0;
                        printf("PIN must contain only numbers!\n");
                        break;
                    }
                }

                if (valid)
                {
                    user[user_count].pin = atoi(pin_str);

                    if (user[user_count].pin < 10000 || user[user_count].pin > 99999)
                    {
                        valid = 0;
                        printf("PIN must be exactly 5 digits (10000–99999)!\n");
                    }
                }
            } while (!valid);

            char dep_str[20]; 
            do
            {
                valid = 1;
                printf("Initial Deposit: ");
                scanf(" %s", dep_str);

                for (int i = 0; dep_str[i] != '\0'; i++)
                {
                    if (!isdigit(dep_str[i]))
                    {
                        printf("Deposit must be numeric only!\n");
                        valid = 0;
                        break;
                    }
                }

                if (valid)
                {
                    user[user_count].balance = atoi(dep_str);

                    if (user[user_count].balance < 0)
                    {
                        printf("Balance cannot be negative!\n");
                        valid = 0;
                    }
                }
            } while (!valid);
            user[user_count].loan_amount = 0;
            user[user_count].daily = 0;
            user[user_count].last_day = get_day();
            user_count++;
            save_users(user, user_count); 
            break;

        case 2:
            signin(user, user_count);
            break;

        case 3:
            printf("Thankyou for using!\n");
            return 0;

        default:
            printf("Invalid choice!\n");
            break;
        }
    }
}
