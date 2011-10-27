// this file includes naming convenetions and constant shared by the vectorizer passes
// this file should NOT include any enviroment (VOLCANO, MIC, APPLE, DX, etc...) specific data 


// Maximum width supported as input
#define MAX_INPUT_VECTOR_WIDTH 16

// Define estimated amount of instructions in function
#define ESTIMATED_INST_NUM 128

// Maximum supported packetization width
#define MAX_PACKET_WIDTH 16