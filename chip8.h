class Chip8_CPU
{
    u_char RAM[4096];       
    u_char REG[16];         
    uint16_t STACK[16];     
    uint16_t SP;            
    uint16_t I;             
    uint16_t PC;                    
    u_char DELAY_TIMER;     
    u_char SOUND_TIMER;     
    u_char SCREEN[64 * 32]; 


    void initialise();
    void load_game(std::string path);
};