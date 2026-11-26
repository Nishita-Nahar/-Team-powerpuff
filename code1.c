#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_USERS 500
#define TOTAL_SLOTS 240
#define DEST_COUNT 20
#define AREA_COUNT 6

#define USER_FILE "users.dat"
#define PARK_FILE "parking.dat"
#define HISTORY_FILE "history.csv"
#define RECEIPT_FILE "receipt.txt"
#define DASHBOARD_FILE "dashboard.html"

#define ADMIN_PIN "1234"

#define RESET "\033[0m"
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define CYAN "\033[1;36m"

/* ============================================================
   STRUCTURES
   ============================================================ */

typedef struct
{
    char loginID[20];

    char sapID[10];
    char driverID[20];
    char priorityID[20];

    char name[80];
    char password[80];

    char category[20];

    char mobile[15];

    char vehicleNumber[30];
    char vehicleType[20];

} User;


typedef struct
{
    int slotID;
    int occupied;

    int areaID;
    int destinationID;

    char slotType[20];

    /*
       GENERAL
       VIP
       EMERGENCY
       AMBULANCE
    */

    char reservationType[20];

    char loginID[20];
    char vehicleNumber[30];

    time_t entryTime;

} ParkingSlot;


typedef struct
{
    int id;
    char name[80];

} Destination;


typedef struct
{
    int id;
    char code[8];
    char name[80];

} ParkingArea;


/* ============================================================
   GLOBAL DATA
   ============================================================ */

User users[MAX_USERS];

ParkingSlot slots[TOTAL_SLOTS];

int totalUsers = 0;


/* ============================================================
   DESTINATIONS
   ============================================================ */

Destination destinations[DEST_COUNT] =
{
    {1,  "Block 1"},
    {2,  "Block 2 (AB-2)"},
    {3,  "Block 3 (AB-3)"},
    {4,  "Block 4 (AB-4)"},
    {5,  "Block 5 (AB-5)"},
    {6,  "Block 6 (AB-6)"},
    {7,  "Block 8"},
    {8,  "Block 9"},
    {9,  "Block 10"},
    {10, "Block 11"},
    {11, "Main Block / Quadrangle"},
    {12, "Aditya Block"},
    {13, "R&D Block"},
    {14, "School of Design"},
    {15, "Central Library"},
    {16, "Main Gate"},
    {17, "Hostel Area"},
    {18, "Sports Area"},
    {19, "MAC (Multi Activity Centre)"},
    {20, "Cafeteria / Food Court"}
};


/* ============================================================
   PARKING AREAS
   ============================================================ */

ParkingArea areas[AREA_COUNT] =
{
    {1, "P1", "North Parking"},
    {2, "P2", "Academic Parking"},
    {3, "P3", "Central Parking"},
    {4, "P4", "South Parking"},
    {5, "P5", "Hostel Parking"},
    {6, "P6", "Main Gate / Visitor Parking"}
};


/* ============================================================
   PROTOTYPE DISTANCES IN METRES
   ============================================================ */

int distanceMatrix[AREA_COUNT][DEST_COUNT] =
{
    {
        70,100,140,190,230,
        280,320,360,400,440,
        180,220,300,350,240,
        80,500,430,380,260
    },

    {
        160,70,80,110,140,
        170,210,250,290,330,
        120,160,220,260,150,
        190,400,330,280,170
    },

    {
        260,180,120,80,70,
        90,130,170,210,250,
        80,100,150,190,70,
        280,310,240,180,90
    },

    {
        360,290,240,180,140,
        100,80,90,120,160,
        180,150,100,80,170,
        380,240,120,70,150
    },

    {
        500,450,400,350,300,
        260,220,190,170,150,
        320,280,240,210,300,
        520,60,140,200,260
    },

    {
        90,160,220,280,340,
        400,450,490,520,550,
        250,300,360,400,280,
        40,520,450,390,300
    }
};


/* ============================================================
   BASIC FUNCTIONS
   ============================================================ */

void clearInput(void)
{
    int c;

    while (
        (c = getchar()) != '\n' &&
        c != EOF
    )
    {
    }
}


void formatTime(
    time_t value,
    char *output,
    int size
)
{
    struct tm *info;

    info = localtime(&value);

    if (info != NULL)
    {
        strftime(
            output,
            size,
            "%d %b %Y, %I:%M %p",
            info
        );
    }

    else
    {
        strcpy(
            output,
            "Unknown"
        );
    }
}


int validSAP(
    const char *sap
)
{
    int i;

    if (
        strlen(sap) != 9
    )
    {
        return 0;
    }

    for (
        i = 0;
        i < 9;
        i++
    )
    {
        if (
            !isdigit(
                (unsigned char)sap[i]
            )
        )
        {
            return 0;
        }
    }

    return 1;
}


int validMobile(
    const char *mobile
)
{
    int i;

    if (
        strlen(mobile) != 10
    )
    {
        return 0;
    }

    for (
        i = 0;
        i < 10;
        i++
    )
    {
        if (
            !isdigit(
                (unsigned char)mobile[i]
            )
        )
        {
            return 0;
        }
    }

    return 1;
}


/* ============================================================
   LOOKUPS
   ============================================================ */

const char *areaCode(
    int id
)
{
    if (
        id >= 1 &&
        id <= AREA_COUNT
    )
    {
        return areas[id - 1].code;
    }

    return "PX";
}


const char *areaName(
    int id
)
{
    if (
        id >= 1 &&
        id <= AREA_COUNT
    )
    {
        return areas[id - 1].name;
    }

    return "Unknown";
}


const char *destName(
    int id
)
{
    if (
        id >= 1 &&
        id <= DEST_COUNT
    )
    {
        return destinations[id - 1].name;
    }

    return "Priority / Direct Access";
}


int findUser(
    const char *id
)
{
    int i;

    for (
        i = 0;
        i < totalUsers;
        i++
    )
    {
        if (
            strcmp(
                users[i].loginID,
                id
            ) == 0
        )
        {
            return i;
        }
    }

    return -1;
}


int findVehicle(
    const char *vehicle
)
{
    int i;

    for (
        i = 0;
        i < totalUsers;
        i++
    )
    {
        if (
            strcmp(
                users[i].vehicleNumber,
                vehicle
            ) == 0
        )
        {
            return i;
        }
    }

    return -1;
}


/* ============================================================
   USER DATABASE
   ============================================================ */

void saveUsers(void)
{
    FILE *fp;

    fp =
        fopen(
            USER_FILE,
            "wb"
        );

    if (
        fp == NULL
    )
    {
        return;
    }

    fwrite(
        &totalUsers,
        sizeof(int),
        1,
        fp
    );

    fwrite(
        users,
        sizeof(User),
        totalUsers,
        fp
    );

    fclose(fp);
}


void loadUsers(void)
{
    FILE *fp;

    fp =
        fopen(
            USER_FILE,
            "rb"
        );

    if (
        fp == NULL
    )
    {
        totalUsers = 0;

        return;
    }

    if (
        fread(
            &totalUsers,
            sizeof(int),
            1,
            fp
        ) != 1 ||

        totalUsers < 0 ||

        totalUsers > MAX_USERS
    )
    {
        totalUsers = 0;

        fclose(fp);

        return;
    }

    if (
        fread(
            users,
            sizeof(User),
            totalUsers,
            fp
        ) !=
        (size_t)totalUsers
    )
    {
        totalUsers = 0;
    }

    fclose(fp);
}


/* ============================================================
   PARKING INITIALIZATION

   TOTAL = 240

   GENERAL   = 230
   VIP       = 4
   EMERGENCY = 3
   AMBULANCE = 3
   ============================================================ */

void initializeParking(void)
{
    int i;

    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        slots[i].slotID =
            i + 1;

        slots[i].occupied =
            0;

        slots[i].destinationID =
            0;

        slots[i].entryTime =
            0;

        strcpy(
            slots[i].loginID,
            "NONE"
        );

        strcpy(
            slots[i].vehicleNumber,
            "NONE"
        );

        strcpy(
            slots[i].reservationType,
            "GENERAL"
        );


        /* VEHICLE TYPES */

        if (
            i < 120
        )
        {
            strcpy(
                slots[i].slotType,
                "2-Wheeler"
            );
        }

        else if (
            i < 200
        )
        {
            strcpy(
                slots[i].slotType,
                "Car"
            );
        }

        else if (
            i < 225
        )
        {
            strcpy(
                slots[i].slotType,
                "Van"
            );
        }

        else
        {
            strcpy(
                slots[i].slotType,
                "Bus"
            );
        }


        slots[i].areaID =
            (
                i %
                AREA_COUNT
            ) +
            1;
    }


    /* ========================================================
       VIP PARKING = 4
       ======================================================== */

    strcpy(
        slots[122].reservationType,
        "VIP"
    );

    slots[122].areaID =
        3;


    strcpy(
        slots[128].reservationType,
        "VIP"
    );

    slots[128].areaID =
        3;


    strcpy(
        slots[125].reservationType,
        "VIP"
    );

    slots[125].areaID =
        6;


    strcpy(
        slots[131].reservationType,
        "VIP"
    );

    slots[131].areaID =
        6;


    /* ========================================================
       EMERGENCY PARKING = 3
       ======================================================== */

    strcpy(
        slots[134].reservationType,
        "EMERGENCY"
    );

    slots[134].areaID =
        3;


    strcpy(
        slots[137].reservationType,
        "EMERGENCY"
    );

    slots[137].areaID =
        6;


    strcpy(
        slots[143].reservationType,
        "EMERGENCY"
    );

    slots[143].areaID =
        6;


    /* ========================================================
       AMBULANCE PARKING = 3
       ======================================================== */

    strcpy(
        slots[203].reservationType,
        "AMBULANCE"
    );

    slots[203].areaID =
        6;


    strcpy(
        slots[209].reservationType,
        "AMBULANCE"
    );

    slots[209].areaID =
        6;


    strcpy(
        slots[215].reservationType,
        "AMBULANCE"
    );

    slots[215].areaID =
        6;
}


void saveParking(void)
{
    FILE *fp;

    fp =
        fopen(
            PARK_FILE,
            "wb"
        );

    if (
        fp == NULL
    )
    {
        return;
    }

    fwrite(
        slots,
        sizeof(ParkingSlot),
        TOTAL_SLOTS,
        fp
    );

    fclose(fp);
}


void loadParking(void)
{
    FILE *fp;

    fp =
        fopen(
            PARK_FILE,
            "rb"
        );

    if (
        fp == NULL
    )
    {
        initializeParking();

        saveParking();

        return;
    }

    if (
        fread(
            slots,
            sizeof(ParkingSlot),
            TOTAL_SLOTS,
            fp
        ) !=
        TOTAL_SLOTS
    )
    {
        fclose(fp);

        initializeParking();

        saveParking();

        return;
    }

    fclose(fp);
}


/* ============================================================
   AUTOMATIC FEMALE VOICE - MACOS
   ============================================================ */

void speak(
    const char *message
)
{
    char command[1200];

    snprintf(
        command,
        sizeof(command),

        "/usr/bin/say "
        "-v Samantha "
        "-r 170 "
        "\"%s\"",

        message
    );

    if (
        system(command) != 0
    )
    {
        snprintf(
            command,
            sizeof(command),

            "/usr/bin/say "
            "-r 170 "
            "\"%s\"",

            message
        );

        system(command);
    }
}


/* ============================================================
   SLOT CODE
   ============================================================ */

void makeSlotCode(
    int index,
    char *output,
    int size
)
{
    const char *type;

    int i;

    int number =
        0;


    if (
        strcmp(
            slots[index].slotType,
            "2-Wheeler"
        ) == 0
    )
    {
        type =
            "2W";
    }

    else if (
        strcmp(
            slots[index].slotType,
            "Car"
        ) == 0
    )
    {
        type =
            "C";
    }

    else if (
        strcmp(
            slots[index].slotType,
            "Van"
        ) == 0
    )
    {
        type =
            "V";
    }

    else
    {
        type =
            "B";
    }


    for (
        i = 0;
        i <= index;
        i++
    )
    {
        if (
            slots[i].areaID ==
                slots[index].areaID &&

            strcmp(
                slots[i].slotType,
                slots[index].slotType
            ) == 0
        )
        {
            number++;
        }
    }


    snprintf(
        output,
        size,

        "%s-%s-%03d",

        areaCode(
            slots[index].areaID
        ),

        type,

        number
    );
}


/* ============================================================
   GENERATE DRIVER / PRIORITY ID
   ============================================================ */

void generateID(
    const char *prefix,
    char *output,
    int size
)
{
    int number =
        1;

    do
    {
        snprintf(
            output,
            size,

            "%s%03d",

            prefix,

            number++
        );

    }
    while (
        findUser(
            output
        ) != -1
    );
}


/* ============================================================
   STUDENT / FACULTY REGISTRATION
   ============================================================ */

void registerStudentFaculty(void)
{
    User u;

    char password[80];

    char confirm[80];

    int choice;


    memset(
        &u,
        0,
        sizeof(u)
    );


    printf(
        "\n" CYAN
        "============================================================\n"
        "               STUDENT / FACULTY REGISTRATION\n"
        "============================================================\n"
        RESET
    );


    printf(
        "Enter 9-Digit SAP ID: "
    );

    scanf(
        "%9s",
        u.sapID
    );


    if (
        !validSAP(
            u.sapID
        )
    )
    {
        printf(
            RED
            "Invalid SAP ID. Exactly 9 digits required.\n"
            RESET
        );

        return;
    }


    if (
        findUser(
            u.sapID
        ) != -1
    )
    {
        printf(
            RED
            "SAP ID already registered.\n"
            RESET
        );

        return;
    }


    strcpy(
        u.loginID,
        u.sapID
    );


    strcpy(
        u.driverID,
        "NONE"
    );


    strcpy(
        u.priorityID,
        "NONE"
    );


    strcpy(
        u.mobile,
        "NONE"
    );


    printf(
        "Full Name: "
    );


    clearInput();


    fgets(
        u.name,
        sizeof(u.name),
        stdin
    );


    u.name[
        strcspn(
            u.name,
            "\n"
        )
    ] = '\0';


    printf(
        "\n1. Student\n"
        "2. Faculty\n"
        "Category: "
    );


    scanf(
        "%d",
        &choice
    );


    if (
        choice == 1
    )
    {
        strcpy(
            u.category,
            "Student"
        );
    }

    else if (
        choice == 2
    )
    {
        strcpy(
            u.category,
            "Faculty"
        );
    }

    else
    {
        printf(
            RED
            "Invalid category.\n"
            RESET
        );

        return;
    }


    printf(
        "Create Password: "
    );

    scanf(
        "%79s",
        password
    );


    printf(
        "Confirm Password: "
    );

    scanf(
        "%79s",
        confirm
    );


    if (
        strcmp(
            password,
            confirm
        ) != 0 ||

        strlen(password) <
        4
    )
    {
        printf(
            RED
            "Passwords do not match or are too short.\n"
            RESET
        );

        return;
    }


    strcpy(
        u.password,
        password
    );


    printf(
        "Vehicle Number: "
    );

    scanf(
        "%29s",
        u.vehicleNumber
    );


    if (
        findVehicle(
            u.vehicleNumber
        ) != -1
    )
    {
        printf(
            RED
            "Vehicle already registered.\n"
            RESET
        );

        return;
    }


    printf(
        "\n1. 2-Wheeler\n"
        "2. Car\n"
        "3. Van\n"
        "4. Bus\n"
        "Vehicle Type: "
    );


    scanf(
        "%d",
        &choice
    );


    if (
        choice == 1
    )
    {
        strcpy(
            u.vehicleType,
            "2-Wheeler"
        );
    }

    else if (
        choice == 2
    )
    {
        strcpy(
            u.vehicleType,
            "Car"
        );
    }

    else if (
        choice == 3
    )
    {
        strcpy(
            u.vehicleType,
            "Van"
        );
    }

    else if (
        choice == 4
    )
    {
        strcpy(
            u.vehicleType,
            "Bus"
        );
    }

    else
    {
        return;
    }


    users[totalUsers++] =
        u;


    saveUsers();


    printf(
        GREEN
        "\nREGISTRATION SUCCESSFUL\n"
        RESET
    );


    printf(
        "SAP ID   : %s\n"
        "Name     : %s\n"
        "Category : %s\n"
        "Vehicle  : %s\n",

        u.sapID,

        u.name,

        u.category,

        u.vehicleNumber
    );
}


/* ============================================================
   DRIVER REGISTRATION
   ============================================================ */

void registerDriver(void)
{
    User u;

    char password[80];

    char confirm[80];

    int choice;


    memset(
        &u,
        0,
        sizeof(u)
    );


    strcpy(
        u.sapID,
        "NONE"
    );


    strcpy(
        u.priorityID,
        "NONE"
    );


    strcpy(
        u.category,
        "Driver"
    );


    printf(
        "\n" CYAN
        "============================================================\n"
        "                     DRIVER REGISTRATION\n"
        "============================================================\n"
        RESET
    );


    printf(
        "Full Name: "
    );


    clearInput();


    fgets(
        u.name,
        sizeof(u.name),
        stdin
    );


    u.name[
        strcspn(
            u.name,
            "\n"
        )
    ] = '\0';


    printf(
        "10-Digit Mobile: "
    );


    scanf(
        "%14s",
        u.mobile
    );


    if (
        !validMobile(
            u.mobile
        )
    )
    {
        printf(
            RED
            "Invalid mobile number.\n"
            RESET
        );

        return;
    }


    generateID(
        "DRV",
        u.driverID,
        sizeof(u.driverID)
    );


    strcpy(
        u.loginID,
        u.driverID
    );


    printf(
        GREEN
        "Generated Driver ID: %s\n"
        RESET,

        u.driverID
    );


    printf(
        "Create Password: "
    );


    scanf(
        "%79s",
        password
    );


    printf(
        "Confirm Password: "
    );


    scanf(
        "%79s",
        confirm
    );


    if (
        strcmp(
            password,
            confirm
        ) != 0 ||

        strlen(password) <
        4
    )
    {
        printf(
            RED
            "Invalid password.\n"
            RESET
        );

        return;
    }


    strcpy(
        u.password,
        password
    );


    printf(
        "Vehicle Number: "
    );


    scanf(
        "%29s",
        u.vehicleNumber
    );


    if (
        findVehicle(
            u.vehicleNumber
        ) != -1
    )
    {
        printf(
            RED
            "Vehicle already registered.\n"
            RESET
        );

        return;
    }


    printf(
        "\n1. Car\n"
        "2. Van\n"
        "3. Bus\n"
        "Vehicle Type: "
    );


    scanf(
        "%d",
        &choice
    );


    if (
        choice == 1
    )
    {
        strcpy(
            u.vehicleType,
            "Car"
        );
    }

    else if (
        choice == 2
    )
    {
        strcpy(
            u.vehicleType,
            "Van"
        );
    }

    else if (
        choice == 3
    )
    {
        strcpy(
            u.vehicleType,
            "Bus"
        );
    }

    else
    {
        return;
    }


    users[totalUsers++] =
        u;


    saveUsers();


    printf(
        GREEN
        "\nDRIVER REGISTRATION SUCCESSFUL\n"
        RESET
    );


    printf(
        "Driver ID : %s\n"
        "Name      : %s\n"
        "Vehicle   : %s\n",

        u.driverID,

        u.name,

        u.vehicleNumber
    );
}


/* ============================================================
   VIP / EMERGENCY / AMBULANCE REGISTRATION
   ============================================================ */

void registerPriority(void)
{
    User u;

    char password[80];

    char confirm[80];

    char prefix[8];

    int choice;


    memset(
        &u,
        0,
        sizeof(u)
    );


    strcpy(
        u.sapID,
        "NONE"
    );


    strcpy(
        u.driverID,
        "NONE"
    );


    printf(
        "\n" CYAN
        "============================================================\n"
        "              PRIORITY VEHICLE REGISTRATION\n"
        "============================================================\n"
        RESET
    );


    printf(
        "1. VIP\n"
        "2. Emergency\n"
        "3. Ambulance\n"
        "4. Return\n\n"
        "Category: "
    );


    scanf(
        "%d",
        &choice
    );


    if (
        choice == 1
    )
    {
        strcpy(
            u.category,
            "VIP"
        );

        strcpy(
            prefix,
            "VIP"
        );

        strcpy(
            u.vehicleType,
            "Car"
        );
    }

    else if (
        choice == 2
    )
    {
        strcpy(
            u.category,
            "Emergency"
        );

        strcpy(
            prefix,
            "EMG"
        );

        strcpy(
            u.vehicleType,
            "Car"
        );
    }

    else if (
        choice == 3
    )
    {
        strcpy(
            u.category,
            "Ambulance"
        );

        strcpy(
            prefix,
            "AMB"
        );

        strcpy(
            u.vehicleType,
            "Van"
        );
    }

    else
    {
        return;
    }


    printf(
        "Authorized Person / Department Name: "
    );


    clearInput();


    fgets(
        u.name,
        sizeof(u.name),
        stdin
    );


    u.name[
        strcspn(
            u.name,
            "\n"
        )
    ] = '\0';


    printf(
        "10-Digit Mobile: "
    );


    scanf(
        "%14s",
        u.mobile
    );


    if (
        !validMobile(
            u.mobile
        )
    )
    {
        printf(
            RED
            "Invalid mobile number.\n"
            RESET
        );

        return;
    }


    printf(
        "Vehicle Number: "
    );


    scanf(
        "%29s",
        u.vehicleNumber
    );


    if (
        findVehicle(
            u.vehicleNumber
        ) != -1
    )
    {
        printf(
            RED
            "Vehicle already registered.\n"
            RESET
        );

        return;
    }


    generateID(
        prefix,
        u.priorityID,
        sizeof(u.priorityID)
    );


    strcpy(
        u.loginID,
        u.priorityID
    );


    printf(
        "Create Password: "
    );


    scanf(
        "%79s",
        password
    );


    printf(
        "Confirm Password: "
    );


    scanf(
        "%79s",
        confirm
    );


    if (
        strcmp(
            password,
            confirm
        ) != 0 ||

        strlen(password) <
        4
    )
    {
        printf(
            RED
            "Invalid password.\n"
            RESET
        );

        return;
    }


    strcpy(
        u.password,
        password
    );


    users[totalUsers++] =
        u;


    saveUsers();


    printf(
        GREEN
        "\nPRIORITY REGISTRATION SUCCESSFUL\n"
        RESET
    );


    printf(
        "Priority Category : %s\n"
        "Priority ID       : %s\n"
        "Vehicle Number    : %s\n",

        u.category,

        u.priorityID,

        u.vehicleNumber
    );
}


/* ============================================================
   CATEGORY LOGIN
   ============================================================ */

int loginByCategory(
    const char *mode
)
{
    char id[20];

    char password[80];

    int index;


    printf(
        "\n" CYAN
        "================ %s LOGIN ================\n"
        RESET,

        mode
    );


    if (
        strcmp(
            mode,
            "STUDENT / FACULTY"
        ) == 0
    )
    {
        printf(
            "SAP ID: "
        );
    }

    else if (
        strcmp(
            mode,
            "DRIVER"
        ) == 0
    )
    {
        printf(
            "Driver ID: "
        );
    }

    else
    {
        printf(
            "Priority ID: "
        );
    }


    scanf(
        "%19s",
        id
    );


    index =
        findUser(
            id
        );


    if (
        index == -1
    )
    {
        printf(
            RED
            "Account not found.\n"
            RESET
        );

        return -1;
    }


    if (
        strcmp(
            mode,
            "STUDENT / FACULTY"
        ) == 0 &&

        strcmp(
            users[index].category,
            "Student"
        ) != 0 &&

        strcmp(
            users[index].category,
            "Faculty"
        ) != 0
    )
    {
        printf(
            RED
            "Please use the correct login category.\n"
            RESET
        );

        return -1;
    }


    if (
        strcmp(
            mode,
            "DRIVER"
        ) == 0 &&

        strcmp(
            users[index].category,
            "Driver"
        ) != 0
    )
    {
        printf(
            RED
            "Please use Driver Login.\n"
            RESET
        );

        return -1;
    }


    if (
        strcmp(
            mode,
            "PRIORITY VEHICLE"
        ) == 0 &&

        strcmp(
            users[index].category,
            "VIP"
        ) != 0 &&

        strcmp(
            users[index].category,
            "Emergency"
        ) != 0 &&

        strcmp(
            users[index].category,
            "Ambulance"
        ) != 0
    )
    {
        printf(
            RED
            "Please use Priority Vehicle Login.\n"
            RESET
        );

        return -1;
    }


    printf(
        "Password: "
    );


    scanf(
        "%79s",
        password
    );


    if (
        strcmp(
            users[index].password,
            password
        ) != 0
    )
    {
        printf(
            RED
            "Incorrect password.\n"
            RESET
        );

        return -1;
    }


    printf(
        GREEN
        "LOGIN SUCCESSFUL\n"
        RESET
    );


    printf(
        "Welcome, %s\n",
        users[index].name
    );


    return index;
}


/* ============================================================
   RESET PASSWORD
   ============================================================ */

void resetPassword(void)
{
    char id[20];

    char password[80];

    char confirm[80];

    int index;


    printf(
        "\nEnter SAP ID / Driver ID / Priority ID: "
    );


    scanf(
        "%19s",
        id
    );


    index =
        findUser(
            id
        );


    if (
        index == -1
    )
    {
        printf(
            RED
            "Account not found.\n"
            RESET
        );

        return;
    }


    printf(
        "New Password: "
    );


    scanf(
        "%79s",
        password
    );


    printf(
        "Confirm Password: "
    );


    scanf(
        "%79s",
        confirm
    );


    if (
        strcmp(
            password,
            confirm
        ) != 0 ||

        strlen(password) <
        4
    )
    {
        printf(
            RED
            "Invalid password.\n"
            RESET
        );

        return;
    }


    strcpy(
        users[index].password,
        password
    );


    saveUsers();


    printf(
        GREEN
        "Password reset successful.\n"
        RESET
    );
}


/* ============================================================
   PARKING HELPERS
   ============================================================ */

int findUserSlot(
    int userIndex
)
{
    int i;

    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            slots[i].occupied &&

            strcmp(
                slots[i].loginID,
                users[userIndex].loginID
            ) == 0
        )
        {
            return i;
        }
    }

    return -1;
}


int areaTotal(
    int area
)
{
    int i;

    int total =
        0;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            slots[i].areaID ==
            area
        )
        {
            total++;
        }
    }

    return total;
}


int areaOccupied(
    int area
)
{
    int i;

    int occupied =
        0;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            slots[i].areaID ==
                area &&

            slots[i].occupied
        )
        {
            occupied++;
        }
    }

    return occupied;
}


const char *congestion(
    int occupied,
    int total
)
{
    int percentage;


    if (
        total == 0
    )
    {
        return "NO DATA";
    }


    percentage =
        occupied *
        100 /
        total;


    if (
        percentage >=
        100
    )
    {
        return "FULL";
    }


    if (
        percentage >=
        80
    )
    {
        return "HIGH";
    }


    if (
        percentage >=
        50
    )
    {
        return "MODERATE";
    }


    return "LOW";
}


int walkingMinutes(
    int metres
)
{
    int minutes;


    minutes =
        (int)(
            (
                (
                    metres /
                    1.3
                ) +
                59
            ) /
            60
        );


    if (
        minutes < 1
    )
    {
        minutes =
            1;
    }


    return minutes;
}


/* ============================================================
   SMART SCORE
   ============================================================ */

int smartScore(
    int area,
    int destination,
    const char *vehicleType
)
{
    int i;

    int total =
        0;

    int occupied =
        0;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            slots[i].areaID ==
                area &&

            strcmp(
                slots[i].slotType,
                vehicleType
            ) == 0 &&

            strcmp(
                slots[i].reservationType,
                "GENERAL"
            ) == 0
        )
        {
            total++;


            if (
                slots[i].occupied
            )
            {
                occupied++;
            }
        }
    }


    if (
        total == 0
    )
    {
        return 999999;
    }


    return

        distanceMatrix[
            area - 1
        ][
            destination - 1
        ]

        +

        (
            occupied *
            100 /
            total
        ) *
        2;
}


/* ============================================================
   GENERAL SLOT SEARCH
   ============================================================ */

int findGeneralSlot(
    int area,
    const char *vehicleType
)
{
    int i;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            !slots[i].occupied &&

            slots[i].areaID ==
                area &&

            strcmp(
                slots[i].slotType,
                vehicleType
            ) == 0 &&

            strcmp(
                slots[i].reservationType,
                "GENERAL"
            ) == 0
        )
        {
            return i;
        }
    }


    return -1;
}


int bestGeneralArea(
    int destination,
    const char *vehicleType
)
{
    int area;

    int bestArea =
        -1;

    int bestScore =
        999999;


    for (
        area = 1;
        area <= AREA_COUNT;
        area++
    )
    {
        if (
            findGeneralSlot(
                area,
                vehicleType
            ) != -1
        )
        {
            int score;


            score =
                smartScore(
                    area,
                    destination,
                    vehicleType
                );


            if (
                score <
                bestScore
            )
            {
                bestScore =
                    score;

                bestArea =
                    area;
            }
        }
    }


    return bestArea;
}


/* ============================================================
   RESERVED SLOT SEARCH
   ============================================================ */

int findReservedSlot(
    const char *reservationType
)
{
    int i;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            !slots[i].occupied &&

            strcmp(
                slots[i].reservationType,
                reservationType
            ) == 0
        )
        {
            return i;
        }
    }


    return -1;
}


/* ============================================================
   DESTINATION LIST
   ============================================================ */

void displayDestinations(void)
{
    int i;


    printf(
        "\n"
        "============================================================\n"
        "                    SELECT DESTINATION\n"
        "============================================================\n"
    );


    for (
        i = 0;
        i < DEST_COUNT;
        i++
    )
    {
        printf(
            "%2d. %s\n",

            destinations[i].id,

            destinations[i].name
        );
    }
}


/* ============================================================
   DASHBOARD PROTOTYPE
   ============================================================ */

void generateDashboard(void);


/* ============================================================
   NORMAL PARKING
   ============================================================ */

void normalPark(
    int userIndex
)
{
    int destination;

    int area;

    int slot;

    int distance;

    int walk;


    char code[30];

    char timeText[60];

    char announcement[300];


    if (
        findUserSlot(
            userIndex
        ) != -1
    )
    {
        printf(
            YELLOW
            "Vehicle already parked.\n"
            RESET
        );

        return;
    }


    displayDestinations();


    printf(
        "\nDestination ID: "
    );


    scanf(
        "%d",
        &destination
    );


    if (
        destination < 1 ||
        destination > DEST_COUNT
    )
    {
        printf(
            RED
            "Invalid destination.\n"
            RESET
        );

        return;
    }


    area =
        bestGeneralArea(
            destination,
            users[userIndex].vehicleType
        );


    if (
        area == -1
    )
    {
        printf(
            RED
            "No GENERAL parking slot available.\n"
            RESET
        );

        printf(
            "VIP, Emergency and Ambulance slots remain protected.\n"
        );

        return;
    }


    slot =
        findGeneralSlot(
            area,
            users[userIndex].vehicleType
        );


    slots[slot].occupied =
        1;


    slots[slot].destinationID =
        destination;


    slots[slot].entryTime =
        time(NULL);


    strcpy(
        slots[slot].loginID,
        users[userIndex].loginID
    );


    strcpy(
        slots[slot].vehicleNumber,
        users[userIndex].vehicleNumber
    );


    saveParking();


    distance =
        distanceMatrix[
            area - 1
        ][
            destination - 1
        ];


    walk =
        walkingMinutes(
            distance
        );


    makeSlotCode(
        slot,
        code,
        sizeof(code)
    );


    formatTime(
        slots[slot].entryTime,
        timeText,
        sizeof(timeText)
    );


    printf(
        "\n" GREEN
        "============================================================\n"
        "                   PARKING SUCCESSFUL\n"
        "============================================================\n"
        RESET
    );


    if (
        strcmp(
            users[userIndex].category,
            "Driver"
        ) == 0
    )
    {
        printf(
            "Driver ID         : %s\n",
            users[userIndex].driverID
        );
    }

    else
    {
        printf(
            "SAP ID            : %s\n",
            users[userIndex].sapID
        );
    }


    printf(
        "Name              : %s\n"
        "Category          : %s\n"
        "Vehicle           : %s\n"
        "Destination       : %s\n"
        "Parking Area      : %s - %s\n"
        "Parking Slot      : %s\n"
        "Reservation Type  : GENERAL\n"
        "Distance          : ~%d metres\n"
        "Estimated Walk    : ~%d minute%s\n"
        "Entry Time        : %s\n",

        users[userIndex].name,

        users[userIndex].category,

        users[userIndex].vehicleNumber,

        destName(
            destination
        ),

        areaCode(
            area
        ),

        areaName(
            area
        ),

        code,

        distance,

        walk,

        walk == 1
            ? ""
            : "s",

        timeText
    );


    snprintf(
        announcement,
        sizeof(announcement),

        "Welcome to U P E S. "
        "Parking successful. "
        "Your parking area is %s.",

        areaName(
            area
        )
    );


    speak(
        announcement
    );


    generateDashboard();
}


/* ============================================================
   PRIORITY PARKING
   ============================================================ */

void priorityPark(
    int userIndex
)
{
    int slot =
        -1;


    char wanted[20];

    char code[30];

    char timeText[60];

    char announcement[300];


    if (
        findUserSlot(
            userIndex
        ) != -1
    )
    {
        printf(
            YELLOW
            "Vehicle already parked.\n"
            RESET
        );

        return;
    }


    if (
        strcmp(
            users[userIndex].category,
            "VIP"
        ) == 0
    )
    {
        strcpy(
            wanted,
            "VIP"
        );
    }

    else if (
        strcmp(
            users[userIndex].category,
            "Emergency"
        ) == 0
    )
    {
        strcpy(
            wanted,
            "EMERGENCY"
        );
    }

    else
    {
        strcpy(
            wanted,
            "AMBULANCE"
        );
    }


    /* FIRST CHOICE = CORRECT RESERVED TYPE */

    slot =
        findReservedSlot(
            wanted
        );


    /* ========================================================
       AMBULANCE FALLBACK

       Ambulance
       -> Ambulance slot
       -> Emergency slot
       -> General Van slot
       ======================================================== */

    if (
        slot == -1 &&

        strcmp(
            wanted,
            "AMBULANCE"
        ) == 0
    )
    {
        printf(
            YELLOW
            "Ambulance slots full. Checking Emergency slots...\n"
            RESET
        );


        slot =
            findReservedSlot(
                "EMERGENCY"
            );
    }


    /* ========================================================
       VIP FALLBACK
       ======================================================== */

    if (
        slot == -1 &&

        strcmp(
            wanted,
            "VIP"
        ) == 0
    )
    {
        printf(
            YELLOW
            "VIP slots full. Checking GENERAL parking...\n"
            RESET
        );


        slot =
            findGeneralSlot(
                6,
                "Car"
            );


        if (
            slot == -1
        )
        {
            slot =
                findGeneralSlot(
                    3,
                    "Car"
                );
        }
    }


    /* ========================================================
       EMERGENCY FALLBACK
       ======================================================== */

    if (
        slot == -1 &&

        strcmp(
            wanted,
            "EMERGENCY"
        ) == 0
    )
    {
        printf(
            YELLOW
            "Emergency slots full. Checking GENERAL emergency fallback...\n"
            RESET
        );


        slot =
            findGeneralSlot(
                6,
                "Car"
            );


        if (
            slot == -1
        )
        {
            slot =
                findGeneralSlot(
                    3,
                    "Car"
                );
        }
    }


    /* AMBULANCE GENERAL FALLBACK */

    if (
        slot == -1 &&

        strcmp(
            wanted,
            "AMBULANCE"
        ) == 0
    )
    {
        printf(
            YELLOW
            "Emergency slots also full. Checking GENERAL Van parking...\n"
            RESET
        );


        slot =
            findGeneralSlot(
                6,
                "Van"
            );
    }


    if (
        slot == -1
    )
    {
        printf(
            RED
            "No suitable priority parking slot available.\n"
            RESET
        );

        return;
    }


    slots[slot].occupied =
        1;


    slots[slot].destinationID =
        0;


    slots[slot].entryTime =
        time(NULL);


    strcpy(
        slots[slot].loginID,
        users[userIndex].loginID
    );


    strcpy(
        slots[slot].vehicleNumber,
        users[userIndex].vehicleNumber
    );


    saveParking();


    makeSlotCode(
        slot,
        code,
        sizeof(code)
    );


    formatTime(
        slots[slot].entryTime,
        timeText,
        sizeof(timeText)
    );


    printf(
        "\n" GREEN
        "============================================================\n"
        "               PRIORITY PARKING ALLOCATED\n"
        "============================================================\n"
        RESET
    );


    printf(
        "Priority ID       : %s\n"
        "Category          : %s\n"
        "Vehicle Number    : %s\n"
        "Parking Area      : %s - %s\n"
        "Parking Slot      : %s\n"
        "Allocated Type    : %s\n"
        "Entry Time        : %s\n"
        "Status            : OCCUPIED\n",

        users[userIndex].priorityID,

        users[userIndex].category,

        users[userIndex].vehicleNumber,

        areaCode(
            slots[slot].areaID
        ),

        areaName(
            slots[slot].areaID
        ),

        code,

        slots[slot].reservationType,

        timeText
    );


    if (
        strcmp(
            users[userIndex].category,
            "Ambulance"
        ) == 0
    )
    {
        snprintf(
            announcement,
            sizeof(announcement),

            "Ambulance priority parking allocated. "
            "Please keep the emergency access route clear."
        );
    }

    else if (
        strcmp(
            users[userIndex].category,
            "Emergency"
        ) == 0
    )
    {
        snprintf(
            announcement,
            sizeof(announcement),

            "Emergency vehicle parking allocated. "
            "Please keep the access route clear."
        );
    }

    else
    {
        snprintf(
            announcement,
            sizeof(announcement),

            "VIP parking successfully allocated."
        );
    }


    speak(
        announcement
    );


    generateDashboard();
}


/* ============================================================
   FIND MY VEHICLE
   ============================================================ */

void findMyVehicle(
    int userIndex
)
{
    int slot;


    char code[30];

    char timeText[60];


    slot =
        findUserSlot(
            userIndex
        );


    if (
        slot == -1
    )
    {
        printf(
            YELLOW
            "Vehicle not currently parked.\n"
            RESET
        );

        return;
    }


    makeSlotCode(
        slot,
        code,
        sizeof(code)
    );


    formatTime(
        slots[slot].entryTime,
        timeText,
        sizeof(timeText)
    );


    printf(
        "\n"
        "============================================================\n"
        "                       VEHICLE FOUND\n"
        "============================================================\n"
    );


    printf(
        "Vehicle      : %s\n"
        "Parking Area : %s - %s\n"
        "Parking Slot : %s\n"
        "Parking Type : %s\n"
        "Entry Time   : %s\n",

        users[userIndex].vehicleNumber,

        areaCode(
            slots[slot].areaID
        ),

        areaName(
            slots[slot].areaID
        ),

        code,

        slots[slot].reservationType,

        timeText
    );


    if (
        slots[slot].destinationID !=
        0
    )
    {
        printf(
            "Destination  : %s\n",

            destName(
                slots[slot].destinationID
            )
        );
    }
}


/* ============================================================
   SAVE HISTORY
   ============================================================ */

void saveHistory(
    int userIndex,
    int slot,
    time_t exitTime
)
{
    FILE *fp;

    FILE *check;

    int exists;


    char entry[60];

    char exitText[60];

    char code[30];


    check =
        fopen(
            HISTORY_FILE,
            "r"
        );


    exists =
        check != NULL;


    if (
        check != NULL
    )
    {
        fclose(
            check
        );
    }


    fp =
        fopen(
            HISTORY_FILE,
            "a"
        );


    if (
        fp == NULL
    )
    {
        return;
    }


    if (
        !exists
    )
    {
        fprintf(
            fp,

            "ID,"
            "Name,"
            "Category,"
            "Vehicle,"
            "Vehicle_Type,"
            "Parking_Type,"
            "Area,"
            "Slot,"
            "Destination,"
            "Entry,"
            "Exit\n"
        );
    }


    formatTime(
        slots[slot].entryTime,
        entry,
        sizeof(entry)
    );


    formatTime(
        exitTime,
        exitText,
        sizeof(exitText)
    );


    makeSlotCode(
        slot,
        code,
        sizeof(code)
    );


    fprintf(
        fp,

        "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",

        users[userIndex].loginID,

        users[userIndex].name,

        users[userIndex].category,

        users[userIndex].vehicleNumber,

        users[userIndex].vehicleType,

        slots[slot].reservationType,

        areaCode(
            slots[slot].areaID
        ),

        code,

        destName(
            slots[slot].destinationID
        ),

        entry,

        exitText
    );


    fclose(
        fp
    );
}


/* ============================================================
   EXIT + RECEIPT
   ============================================================ */

void exitVehicle(
    int userIndex
)
{
    int slot;


    time_t exitTime;


    char entry[60];

    char exitText[60];

    char code[30];


    FILE *fp;


    slot =
        findUserSlot(
            userIndex
        );


    if (
        slot == -1
    )
    {
        printf(
            YELLOW
            "Vehicle not currently parked.\n"
            RESET
        );

        return;
    }


    exitTime =
        time(NULL);


    formatTime(
        slots[slot].entryTime,
        entry,
        sizeof(entry)
    );


    formatTime(
        exitTime,
        exitText,
        sizeof(exitText)
    );


    makeSlotCode(
        slot,
        code,
        sizeof(code)
    );


    saveHistory(
        userIndex,
        slot,
        exitTime
    );


    fp =
        fopen(
            RECEIPT_FILE,
            "w"
        );


    if (
        fp != NULL
    )
    {
        fprintf(
            fp,

            "============================================================\n"
            "                 UPES SMART PARKING RECEIPT\n"
            "============================================================\n"

            "Name              : %s\n"
            "Category          : %s\n"
            "ID                : %s\n"
            "Vehicle Number    : %s\n"
            "Parking Area      : %s - %s\n"
            "Parking Slot      : %s\n"
            "Parking Type      : %s\n"
            "Entry Time        : %s\n"
            "Exit Time         : %s\n"
            "Status            : EXIT SUCCESSFUL\n"

            "============================================================\n",

            users[userIndex].name,

            users[userIndex].category,

            users[userIndex].loginID,

            users[userIndex].vehicleNumber,

            areaCode(
                slots[slot].areaID
            ),

            areaName(
                slots[slot].areaID
            ),

            code,

            slots[slot].reservationType,

            entry,

            exitText
        );


        fclose(
            fp
        );


        system(
            "/usr/bin/open receipt.txt"
        );
    }


    printf(
        GREEN
        "\nEXIT SUCCESSFUL\n"
        RESET
    );


    printf(
        "Vehicle : %s\n"
        "Slot    : %s\n",

        users[userIndex].vehicleNumber,

        code
    );


    if (
        strcmp(
            users[userIndex].vehicleType,
            "2-Wheeler"
        ) == 0
    )
    {
        speak(
            "Exit successful. "
            "Please wear your helmet. "
            "Thank you for using U P E S Smart Parking."
        );
    }

    else
    {
        speak(
            "Exit successful. "
            "Thank you for using U P E S Smart Parking."
        );
    }


    slots[slot].occupied =
        0;


    slots[slot].destinationID =
        0;


    slots[slot].entryTime =
        0;


    strcpy(
        slots[slot].loginID,
        "NONE"
    );


    strcpy(
        slots[slot].vehicleNumber,
        "NONE"
    );


    /*
       reservationType is NOT changed.

       VIP stays VIP.
       Emergency stays Emergency.
       Ambulance stays Ambulance.
    */


    saveParking();


    generateDashboard();
}


/* ============================================================
   RESERVED STATISTICS
   ============================================================ */

void reservedStats(
    const char *reservationType,
    int *total,
    int *occupied
)
{
    int i;


    *total =
        0;


    *occupied =
        0;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            strcmp(
                slots[i].reservationType,
                reservationType
            ) == 0
        )
        {
            (*total)++;


            if (
                slots[i].occupied
            )
            {
                (*occupied)++;
            }
        }
    }
}


/* ============================================================
   RESERVED STATUS

   OCCUPIED / TOTAL
   ============================================================ */

void showReserved(void)
{
    int total;

    int occupied;


    printf(
        "\n" CYAN
        "============================================================\n"
        "             RESERVED PARKING - OCCUPIED / TOTAL\n"
        "============================================================\n"
        RESET
    );


    reservedStats(
        "VIP",
        &total,
        &occupied
    );


    printf(
        "VIP       : %d / %d Occupied\n",

        occupied,

        total
    );


    reservedStats(
        "EMERGENCY",
        &total,
        &occupied
    );


    printf(
        "Emergency : %d / %d Occupied\n",

        occupied,

        total
    );


    reservedStats(
        "AMBULANCE",
        &total,
        &occupied
    );


    printf(
        "Ambulance : %d / %d Occupied\n",

        occupied,

        total
    );
}


/* ============================================================
   GENERAL CAPACITY

   OCCUPIED / TOTAL
   ============================================================ */

void showCapacity(void)
{
    int i;


    int total2W =
        0;

    int occupied2W =
        0;


    int totalCar =
        0;

    int occupiedCar =
        0;


    int totalVan =
        0;

    int occupiedVan =
        0;


    int totalBus =
        0;

    int occupiedBus =
        0;


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            strcmp(
                slots[i].reservationType,
                "GENERAL"
            ) != 0
        )
        {
            continue;
        }


        if (
            strcmp(
                slots[i].slotType,
                "2-Wheeler"
            ) == 0
        )
        {
            total2W++;


            if (
                slots[i].occupied
            )
            {
                occupied2W++;
            }
        }

        else if (
            strcmp(
                slots[i].slotType,
                "Car"
            ) == 0
        )
        {
            totalCar++;


            if (
                slots[i].occupied
            )
            {
                occupiedCar++;
            }
        }

        else if (
            strcmp(
                slots[i].slotType,
                "Van"
            ) == 0
        )
        {
            totalVan++;


            if (
                slots[i].occupied
            )
            {
                occupiedVan++;
            }
        }

        else
        {
            totalBus++;


            if (
                slots[i].occupied
            )
            {
                occupiedBus++;
            }
        }
    }


    printf(
        "\n" CYAN
        "============================================================\n"
        "              GENERAL PARKING - OCCUPIED / TOTAL\n"
        "============================================================\n"
        RESET
    );


    printf(
        "2-Wheeler : %d / %d\n",

        occupied2W,

        total2W
    );


    printf(
        "Car       : %d / %d\n",

        occupiedCar,

        totalCar
    );


    printf(
        "Van       : %d / %d\n",

        occupiedVan,

        totalVan
    );


    printf(
        "Bus       : %d / %d\n",

        occupiedBus,

        totalBus
    );


    printf(
        "GENERAL   : %d / 230\n",

        occupied2W +
        occupiedCar +
        occupiedVan +
        occupiedBus
    );
}


/* ============================================================
   LIVE PARKING MAP
   ============================================================ */

void liveMap(void)
{
    int area;

    int i;


    printf(
        "\n" CYAN
        "================================================================================================\n"
        "                              UPES LIVE PARKING MAP\n"
        "================================================================================================\n"
        RESET
    );


    printf(
        "%-5s %-28s %-13s %-13s %-13s %-13s %-10s\n",

        "AREA",

        "LOCATION",

        "2-WHEELER",

        "CAR",

        "VAN",

        "BUS",

        "STATUS"
    );


    for (
        area = 1;
        area <= AREA_COUNT;
        area++
    )
    {
        int total2W =
            0;

        int occupied2W =
            0;


        int totalCar =
            0;

        int occupiedCar =
            0;


        int totalVan =
            0;

        int occupiedVan =
            0;


        int totalBus =
            0;

        int occupiedBus =
            0;


        for (
            i = 0;
            i < TOTAL_SLOTS;
            i++
        )
        {
            if (
                slots[i].areaID !=
                area
            )
            {
                continue;
            }


            if (
                strcmp(
                    slots[i].slotType,
                    "2-Wheeler"
                ) == 0
            )
            {
                total2W++;


                if (
                    slots[i].occupied
                )
                {
                    occupied2W++;
                }
            }

            else if (
                strcmp(
                    slots[i].slotType,
                    "Car"
                ) == 0
            )
            {
                totalCar++;


                if (
                    slots[i].occupied
                )
                {
                    occupiedCar++;
                }
            }

            else if (
                strcmp(
                    slots[i].slotType,
                    "Van"
                ) == 0
            )
            {
                totalVan++;


                if (
                    slots[i].occupied
                )
                {
                    occupiedVan++;
                }
            }

            else
            {
                totalBus++;


                if (
                    slots[i].occupied
                )
                {
                    occupiedBus++;
                }
            }
        }


        printf(
            "%-5s %-28s %2d/%-10d %2d/%-10d %2d/%-10d %2d/%-10d %-10s\n",

            areaCode(
                area
            ),

            areaName(
                area
            ),

            occupied2W,

            total2W,

            occupiedCar,

            totalCar,

            occupiedVan,

            totalVan,

            occupiedBus,

            totalBus,

            congestion(
                occupied2W +
                occupiedCar +
                occupiedVan +
                occupiedBus,

                total2W +
                totalCar +
                totalVan +
                totalBus
            )
        );
    }


    printf(
        "------------------------------------------------------------------------------------------------\n"
        "Display Convention: OCCUPIED / TOTAL\n"
    );
}


/* ============================================================
   VIEW USERS
   ============================================================ */

void viewUsers(void)
{
    int i;


    for (
        i = 0;
        i < totalUsers;
        i++
    )
    {
        const char *label;


        if (
            strcmp(
                users[i].category,
                "Student"
            ) == 0 ||

            strcmp(
                users[i].category,
                "Faculty"
            ) == 0
        )
        {
            label =
                "SAP ID";
        }

        else if (
            strcmp(
                users[i].category,
                "Driver"
            ) == 0
        )
        {
            label =
                "Driver ID";
        }

        else
        {
            label =
                "Priority ID";
        }


        printf(
            "\n%s: %s\n"
            "Name: %s\n"
            "Category: %s\n"
            "Vehicle: %s (%s)\n",

            label,

            users[i].loginID,

            users[i].name,

            users[i].category,

            users[i].vehicleNumber,

            users[i].vehicleType
        );
    }
}


/* ============================================================
   SEARCH VEHICLE
   ============================================================ */

void searchVehicle(void)
{
    char vehicle[30];


    int userIndex;

    int slot;


    printf(
        "Vehicle Number: "
    );


    scanf(
        "%29s",
        vehicle
    );


    userIndex =
        findVehicle(
            vehicle
        );


    if (
        userIndex == -1
    )
    {
        printf(
            RED
            "Vehicle not registered.\n"
            RESET
        );

        return;
    }


    slot =
        findUserSlot(
            userIndex
        );


    printf(
        "Name     : %s\n"
        "Category : %s\n"
        "ID       : %s\n"
        "Vehicle  : %s\n"
        "Status   : %s\n",

        users[userIndex].name,

        users[userIndex].category,

        users[userIndex].loginID,

        users[userIndex].vehicleNumber,

        slot == -1
            ? "NOT PARKED"
            : "ON CAMPUS"
    );


    if (
        slot != -1
    )
    {
        char code[30];


        makeSlotCode(
            slot,
            code,
            sizeof(code)
        );


        printf(
            "Area         : %s - %s\n"
            "Slot         : %s\n"
            "Parking Type : %s\n",

            areaCode(
                slots[slot].areaID
            ),

            areaName(
                slots[slot].areaID
            ),

            code,

            slots[slot].reservationType
        );
    }
}


/* ============================================================
   HISTORY
   ============================================================ */

void viewHistory(void)
{
    FILE *fp;


    char line[1000];


    fp =
        fopen(
            HISTORY_FILE,
            "r"
        );


    if (
        fp == NULL
    )
    {
        printf(
            YELLOW
            "No parking history yet.\n"
            RESET
        );

        return;
    }


    while (
        fgets(
            line,
            sizeof(line),
            fp
        )
    )
    {
        printf(
            "%s",
            line
        );
    }


    fclose(
        fp
    );
}


/* ============================================================
   HTML DASHBOARD
   ============================================================ */

void generateDashboard(void)
{
    FILE *fp;


    int i;

    int area;


    int occupied =
        0;


    int vipTotal;

    int vipOccupied;


    int emergencyTotal;

    int emergencyOccupied;


    int ambulanceTotal;

    int ambulanceOccupied;


    char updated[60];


    time_t now;


    now =
        time(NULL);


    fp =
        fopen(
            DASHBOARD_FILE,
            "w"
        );


    if (
        fp == NULL
    )
    {
        return;
    }


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            slots[i].occupied
        )
        {
            occupied++;
        }
    }


    reservedStats(
        "VIP",
        &vipTotal,
        &vipOccupied
    );


    reservedStats(
        "EMERGENCY",
        &emergencyTotal,
        &emergencyOccupied
    );


    reservedStats(
        "AMBULANCE",
        &ambulanceTotal,
        &ambulanceOccupied
    );


    formatTime(
        now,
        updated,
        sizeof(updated)
    );


    fprintf(
        fp,

        "<!DOCTYPE html>"

        "<html>"

        "<head>"

        "<meta charset='UTF-8'>"

        "<meta name='viewport' "
        "content='width=device-width,initial-scale=1'>"

        "<meta http-equiv='refresh' content='5'>"

        "<title>UPES Smart Parking</title>"

        "<style>"

        "*{box-sizing:border-box}"

        "body{"
        "margin:0;"
        "background:#07111f;"
        "color:#e5e7eb;"
        "font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif"
        "}"

        "header{"
        "padding:22px 30px;"
        "background:#0d1726;"
        "border-bottom:1px solid #26364b;"
        "display:flex;"
        "justify-content:space-between;"
        "align-items:center"
        "}"

        ".live{"
        "background:#123524;"
        "color:#86efac;"
        "padding:9px 15px;"
        "border-radius:30px;"
        "font-weight:800"
        "}"

        ".container{"
        "max-width:1500px;"
        "margin:auto;"
        "padding:25px"
        "}"

        ".title{"
        "font-size:19px;"
        "font-weight:800;"
        "margin:28px 0 14px"
        "}"

        ".grid,.zones{"
        "display:grid;"
        "grid-template-columns:repeat(auto-fit,minmax(210px,1fr));"
        "gap:15px"
        "}"

        ".card,.zone{"
        "background:#101c2e;"
        "border:1px solid #25364d;"
        "border-radius:15px;"
        "padding:18px"
        "}"

        ".label{"
        "color:#94a3b8;"
        "font-size:12px;"
        "font-weight:700"
        "}"

        ".value{"
        "font-size:30px;"
        "font-weight:900;"
        "margin-top:8px"
        "}"

        ".red{color:#fb7185}"

        ".green{color:#4ade80}"

        ".blue{color:#60a5fa}"

        ".purple{color:#c084fc}"

        ".orange{color:#fb923c}"

        ".mini{"
        "background:#091525;"
        "padding:9px;"
        "border-radius:8px;"
        "margin-top:7px"
        "}"

        "table{"
        "width:100%%;"
        "border-collapse:collapse;"
        "background:#101c2e"
        "}"

        "th,td{"
        "padding:11px;"
        "text-align:left;"
        "border-bottom:1px solid #25364d"
        "}"

        ".table{"
        "overflow:auto;"
        "border-radius:15px"
        "}"

        ".footer{"
        "margin-top:28px;"
        "color:#64748b"
        "}"

        "</style>"

        "</head>"

        "<body>"

        "<header>"

        "<div>"
        "<h1>UPES Smart Parking</h1>"
        "<div>Bidholi Campus · Operations Command Center</div>"
        "</div>"

        "<div class='live'>● SYSTEM LIVE</div>"

        "</header>"

        "<div class='container'>"
    );


    /* OVERVIEW */

    fprintf(
        fp,

        "<div class='title'>"
        "Live Campus Overview"
        "</div>"

        "<div class='grid'>"

        "<div class='card'>"
        "<div class='label'>TOTAL SLOTS</div>"
        "<div class='value'>240</div>"
        "</div>"

        "<div class='card'>"
        "<div class='label'>OCCUPIED</div>"
        "<div class='value red'>%d</div>"
        "</div>"

        "<div class='card'>"
        "<div class='label'>EMPTY</div>"
        "<div class='value green'>%d</div>"
        "</div>"

        "<div class='card'>"
        "<div class='label'>OCCUPANCY</div>"
        "<div class='value blue'>%d%%</div>"
        "</div>"

        "</div>",

        occupied,

        TOTAL_SLOTS -
        occupied,

        occupied *
        100 /
        TOTAL_SLOTS
    );


    /* RESERVED */

    fprintf(
        fp,

        "<div class='title'>"
        "Priority & Reserved Parking"
        "</div>"

        "<div class='grid'>"

        "<div class='card'>"
        "<div class='label'>VIP PARKING</div>"
        "<div class='value purple'>%d / %d</div>"
        "<div>Occupied</div>"
        "</div>"

        "<div class='card'>"
        "<div class='label'>EMERGENCY PARKING</div>"
        "<div class='value red'>%d / %d</div>"
        "<div>Occupied</div>"
        "</div>"

        "<div class='card'>"
        "<div class='label'>AMBULANCE PARKING</div>"
        "<div class='value red'>%d / %d</div>"
        "<div>Occupied</div>"
        "</div>"

        "<div class='card'>"
        "<div class='label'>PROTECTED CAPACITY</div>"
        "<div class='value orange'>10</div>"
        "</div>"

        "</div>",

        vipOccupied,

        vipTotal,

        emergencyOccupied,

        emergencyTotal,

        ambulanceOccupied,

        ambulanceTotal
    );


    /* PARKING AREAS */

    fprintf(
        fp,

        "<div class='title'>"
        "Parking Zone Intelligence"
        "</div>"

        "<div class='zones'>"
    );


    for (
        area = 1;
        area <= AREA_COUNT;
        area++
    )
    {
        int total2W =
            0;

        int occupied2W =
            0;


        int totalCar =
            0;

        int occupiedCar =
            0;


        int totalVan =
            0;

        int occupiedVan =
            0;


        int totalBus =
            0;

        int occupiedBus =
            0;


        for (
            i = 0;
            i < TOTAL_SLOTS;
            i++
        )
        {
            if (
                slots[i].areaID !=
                area
            )
            {
                continue;
            }


            if (
                strcmp(
                    slots[i].slotType,
                    "2-Wheeler"
                ) == 0
            )
            {
                total2W++;


                if (
                    slots[i].occupied
                )
                {
                    occupied2W++;
                }
            }

            else if (
                strcmp(
                    slots[i].slotType,
                    "Car"
                ) == 0
            )
            {
                totalCar++;


                if (
                    slots[i].occupied
                )
                {
                    occupiedCar++;
                }
            }

            else if (
                strcmp(
                    slots[i].slotType,
                    "Van"
                ) == 0
            )
            {
                totalVan++;


                if (
                    slots[i].occupied
                )
                {
                    occupiedVan++;
                }
            }

            else
            {
                totalBus++;


                if (
                    slots[i].occupied
                )
                {
                    occupiedBus++;
                }
            }
        }


        fprintf(
            fp,

            "<div class='zone'>"

            "<h2 class='blue'>%s</h2>"

            "<div>%s</div>"

            "<div class='mini'>"
            "2-Wheeler "
            "<b>%d / %d occupied</b>"
            "</div>"

            "<div class='mini'>"
            "Car "
            "<b>%d / %d occupied</b>"
            "</div>"

            "<div class='mini'>"
            "Van "
            "<b>%d / %d occupied</b>"
            "</div>"

            "<div class='mini'>"
            "Bus "
            "<b>%d / %d occupied</b>"
            "</div>"

            "</div>",

            areaCode(
                area
            ),

            areaName(
                area
            ),

            occupied2W,

            total2W,

            occupiedCar,

            totalCar,

            occupiedVan,

            totalVan,

            occupiedBus,

            totalBus
        );
    }


    fprintf(
        fp,
        "</div>"
    );


    /* CURRENT VEHICLES */

    fprintf(
        fp,

        "<div class='title'>"
        "Vehicles Currently On Campus"
        "</div>"

        "<div class='table'>"

        "<table>"

        "<tr>"

        "<th>ID</th>"

        "<th>Vehicle</th>"

        "<th>Category</th>"

        "<th>Parking Type</th>"

        "<th>Area</th>"

        "<th>Slot</th>"

        "<th>Entry</th>"

        "</tr>"
    );


    for (
        i = 0;
        i < TOTAL_SLOTS;
        i++
    )
    {
        if (
            slots[i].occupied
        )
        {
            int userIndex;


            char code[30];

            char timeText[60];


            userIndex =
                findUser(
                    slots[i].loginID
                );


            makeSlotCode(
                i,
                code,
                sizeof(code)
            );


            formatTime(
                slots[i].entryTime,
                timeText,
                sizeof(timeText)
            );


            fprintf(
                fp,

                "<tr>"

                "<td>%s</td>"

                "<td>%s</td>"

                "<td>%s</td>"

                "<td>%s</td>"

                "<td>%s</td>"

                "<td>%s</td>"

                "<td>%s</td>"

                "</tr>",

                slots[i].loginID,

                slots[i].vehicleNumber,

                userIndex != -1
                    ? users[userIndex].category
                    : "Unknown",

                slots[i].reservationType,

                areaCode(
                    slots[i].areaID
                ),

                code,

                timeText
            );
        }
    }


    fprintf(
        fp,

        "</table>"

        "</div>"

        "<div class='footer'>"

        "Last Updated: %s · "
        "Auto Refresh: 5 seconds · "
        "Display Convention: OCCUPIED / TOTAL"

        "</div>"

        "</div>"

        "</body>"

        "</html>",

        updated
    );


    fclose(
        fp
    );
}


/* ============================================================
   USER / PRIORITY PORTAL
   ============================================================ */

void userPortal(
    int userIndex
)
{
    int choice;


    int priority;


    priority =

        strcmp(
            users[userIndex].category,
            "VIP"
        ) == 0

        ||

        strcmp(
            users[userIndex].category,
            "Emergency"
        ) == 0

        ||

        strcmp(
            users[userIndex].category,
            "Ambulance"
        ) == 0;


    while (
        1
    )
    {
        printf(
            "\n" GREEN
            "============================================================\n"
            "                    %s PARKING PORTAL\n"
            "============================================================\n"
            RESET,

            priority
                ? users[userIndex].category
                : "USER"
        );


        if (
            strcmp(
                users[userIndex].category,
                "Student"
            ) == 0 ||

            strcmp(
                users[userIndex].category,
                "Faculty"
            ) == 0
        )
        {
            printf(
                "SAP ID      : %s\n",
                users[userIndex].sapID
            );
        }

        else if (
            strcmp(
                users[userIndex].category,
                "Driver"
            ) == 0
        )
        {
            printf(
                "Driver ID   : %s\n",
                users[userIndex].driverID
            );
        }

        else
        {
            printf(
                "Priority ID : %s\n",
                users[userIndex].priorityID
            );
        }


        printf(
            "Name        : %s\n"
            "Category    : %s\n"
            "Vehicle     : %s\n",

            users[userIndex].name,

            users[userIndex].category,

            users[userIndex].vehicleNumber
        );


        printf(
            "\n1. View Parking Status\n"
            "2. Enter / Park Vehicle\n"
            "3. Find My Vehicle\n"
            "4. Exit Vehicle & Receipt\n"
            "5. Logout\n"
            "\nChoice: "
        );


        scanf(
            "%d",
            &choice
        );


        if (
            choice == 1
        )
        {
            if (
                priority
            )
            {
                showReserved();
            }

            else
            {
                showCapacity();
            }
        }

        else if (
            choice == 2
        )
        {
            if (
                priority
            )
            {
                priorityPark(
                    userIndex
                );
            }

            else
            {
                normalPark(
                    userIndex
                );
            }
        }

        else if (
            choice == 3
        )
        {
            findMyVehicle(
                userIndex
            );
        }

        else if (
            choice == 4
        )
        {
            exitVehicle(
                userIndex
            );
        }

        else if (
            choice == 5
        )
        {
            return;
        }
    }
}


/* ============================================================
   ADMIN MENU
   ============================================================ */

void adminMenu(void)
{
    int choice;


    while (
        1
    )
    {
        printf(
            "\n" YELLOW
            "============================================================\n"
            "                     ADMIN CONTROL PANEL\n"
            "============================================================\n"
            RESET
        );


        printf(
            "1. General Parking Status\n"
            "2. Reserved Parking Status\n"
            "3. Complete Live Parking Map\n"
            "4. Registered Users & Vehicles\n"
            "5. Search Vehicle\n"
            "6. Parking History\n"
            "7. Open Live HTML Dashboard\n"
            "8. Reset Active Parking\n"
            "9. Logout\n"
            "\nChoice: "
        );


        scanf(
            "%d",
            &choice
        );


        if (
            choice == 1
        )
        {
            showCapacity();
        }

        else if (
            choice == 2
        )
        {
            showReserved();
        }

        else if (
            choice == 3
        )
        {
            liveMap();
        }

        else if (
            choice == 4
        )
        {
            viewUsers();
        }

        else if (
            choice == 5
        )
        {
            searchVehicle();
        }

        else if (
            choice == 6
        )
        {
            viewHistory();
        }

        else if (
            choice == 7
        )
        {
            generateDashboard();


            system(
                "/usr/bin/open dashboard.html"
            );
        }

        else if (
            choice == 8
        )
        {
            char confirm;


            printf(
                RED
                "This clears all active parking. Continue? (Y/N): "
                RESET
            );


            scanf(
                " %c",
                &confirm
            );


            if (
                confirm == 'Y' ||
                confirm == 'y'
            )
            {
                initializeParking();


                saveParking();


                generateDashboard();


                printf(
                    GREEN
                    "Parking reset successful.\n"
                    RESET
                );
            }
        }

        else if (
            choice == 9
        )
        {
            return;
        }
    }
}


/* ============================================================
   MAIN
   ============================================================ */

int main(void)
{
    int choice;

    int userIndex;


    char pin[20];


    loadUsers();


    loadParking();


    generateDashboard();


    while (
        1
    )
    {
        printf(
            "\n" CYAN
            "============================================================\n"
            "          UPES SMART CAMPUS PARKING SYSTEM\n"
            "============================================================\n"
            RESET
        );


        printf(
            "1. Student / Faculty Registration\n"
            "2. Driver Registration\n"
            "3. VIP / Emergency / Ambulance Registration\n"
            "4. Student / Faculty Login\n"
            "5. Driver Login\n"
            "6. VIP / Emergency / Ambulance Login\n"
            "7. Forgot / Reset Password\n"
            "8. Admin Login\n"
            "9. Exit Application\n"
            "\nSelect Option: "
        );


        if (
            scanf(
                "%d",
                &choice
            ) != 1
        )
        {
            clearInput();

            continue;
        }


        if (
            choice == 1
        )
        {
            registerStudentFaculty();
        }

        else if (
            choice == 2
        )
        {
            registerDriver();
        }

        else if (
            choice == 3
        )
        {
            registerPriority();
        }

        else if (
            choice == 4
        )
        {
            userIndex =
                loginByCategory(
                    "STUDENT / FACULTY"
                );


            if (
                userIndex != -1
            )
            {
                userPortal(
                    userIndex
                );
            }
        }

        else if (
            choice == 5
        )
        {
            userIndex =
                loginByCategory(
                    "DRIVER"
                );


            if (
                userIndex != -1
            )
            {
                userPortal(
                    userIndex
                );
            }
        }

        else if (
            choice == 6
        )
        {
            userIndex =
                loginByCategory(
                    "PRIORITY VEHICLE"
                );


            if (
                userIndex != -1
            )
            {
                userPortal(
                    userIndex
                );
            }
        }

        else if (
            choice == 7
        )
        {
            resetPassword();
        }

        else if (
            choice == 8
        )
        {
            printf(
                "Admin PIN: "
            );


            scanf(
                "%19s",
                pin
            );


            if (
                strcmp(
                    pin,
                    ADMIN_PIN
                ) == 0
            )
            {
                adminMenu();
            }

            else
            {
                printf(
                    RED
                    "Incorrect Admin PIN.\n"
                    RESET
                );
            }
        }

        else if (
            choice == 9
        )
        {
            saveUsers();


            saveParking();


            generateDashboard();


            printf(
                GREEN
                "All data saved. Application closed.\n"
                RESET
            );


            return 0;
        }


        generateDashboard();
    }
}