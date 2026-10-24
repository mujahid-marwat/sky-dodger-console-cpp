#include <iostream>   
#include <iomanip>    
using namespace std;  

const int GROUND = 0;      
const int CEILING = 10;   
const int MAX_TURNS = 10;  

// shows the game status (HUD) each turn
void showHUD(int turn, int altitude, int score) {
    cout << "-------------------------" << endl;                         
    cout << left << setw(10) << "Turn:" << turn << endl;                 
    cout << left << setw(10) << "Altitude:" << altitude << " / " << CEILING << endl; 
    cout << left << setw(10) << "Score:" << score << endl;                
}


void updateAltitude(int &altitude, char move) {
    if (move == 'w' || move == 'W') {   
        altitude += 2;                  
    } else {                           
        altitude -= 1;               
    }
}


bool isOutOfBounds(int altitude) {
    return (altitude <= GROUND || altitude >= CEILING);  
}

// calculates the average altitude using static_cast
double calcAverageAltitude(int totalAltitude, int turns) {
    if (turns == 0) {                                    
        return 0.0;                                     
    }
    return static_cast<double>(totalAltitude) / turns;    
}

int main() {
    cout << "Group: 021-23-0045" << endl;   
    cout << "=== SKY DODGER ===" << endl;   

    int altitude = 5;       
    int score = 0;           
    int turn = 0;           
    int totalAltitude = 0;   
    bool alive = true;       
    char move;               

    while (alive && turn < MAX_TURNS) {    
        showHUD(turn, altitude, score);     
        cout << "Press w to flap, n to wait: ";  
        cin >> move;                        

        updateAltitude(altitude, move);      

        if (isOutOfBounds(altitude)) {      
            cout << "Altitude: " << altitude << endl;        
            cout << "GAME OVER - you hit the ground or ceiling!" << endl; 
            alive = false;                 
        } else {                            
            turn++;                         
            score++;                        
            totalAltitude += altitude;       
        }
    }

    cout << "=========================" << endl;                       
    cout << "Final score: " << score << endl;                            
    cout << "Turns survived: " << turn << endl;                        
    cout << fixed << setprecision(2);                                  
    cout << "Average altitude: " << calcAverageAltitude(totalAltitude, turn) << endl; 
    cout << boolalpha;                                                   
    cout << "Survived full run: " << alive << endl;                      

    return 0;   
}