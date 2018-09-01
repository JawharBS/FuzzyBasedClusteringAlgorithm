/**
 * Copyright (c) 2014 panStamp <contact@panstamp.com>
 * 
 * This file is part of the panStamp project.
 * 
 * panStamp  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 * 
 * panStamp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with panStamp; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 
 * USA
 * 
 * Author: Daniel Berenguer
 * Creation date: 07/23/2014
 */

/**
 * Basic radio example
 *
 * Each node acts simultaneously as sender and receiver. Each node sends a packet
 * every five senconds and toggles a LED each time a packet is received
 */
 
#define RFCHANNEL        0       // Let's use channel 0
#define SYNCWORD1        0xB5    // Synchronization word, high byte
#define SYNCWORD0        0x47    // Synchronization word, low byte
#define SOURCE_ADDR      4       // Sender address
#define DESTINATION_ADDR 5       // Receiver address
#define LEDPIN  4
#include "SoftwareSerial.h"
#include "modem.h"
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>

// Step 1 -  Instantiating an object library
Fuzzy* fuzzy = new Fuzzy();
CCPACKET txPacket;  // packet object
float voltage = 0;
int selfLevel = 1;
int selfId = 1;
float selfChance = 0;
int totalNumberOfLevels = 2;
float X = 18.70;
float Y = 14.47;
int numPack = 0;
float  tp = 100;
int level;
bool isClusterHead = false;
bool notClusterHead = false;
bool escapePacket = false;
bool node1 = false;
bool node2 = false;
bool node3 = false;
int chanceTab[3];
float neighboursData[4][5];
int nPack = 0;
int nbPacketsFromSameLevel = 0;
float selfDensity = 0;
float selfCentrality = 0;
int nbNeighboors = 1;
float Einit = 32100;
float Isleep = 0.00046;
float ITx = 0.0018;
float IRx = 0.001221;
float tTx = 1.2;
float tRx = 1.2;
float tSleep = 10000;
/**
 * This function is called whenever a wireless packet is received
 */

void rfPacketReceived(CCPACKET *packet)
{
  if (packet->length > /*5*/0)
  {
    rxPacket = packet;
    packetAvailable = true;
  }
  }

long getDecimal(float val)
{
  int intPart = int(val);
  long decPart = 1000*(val-intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places. 
                                    //Change to match the number of decimal places you need
  if(decPart>0)return(decPart);           //return the decimal part of float number if it is available 
  else if(decPart<0)return((-1)*decPart); //if negative, multiply by -1
  else if(decPart=0)return(00);           //return 0 if decimal part of float number is not available
}

void blinker(){
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
}


void neighboorsIntTab (String ch1, int neighboorsTab[])
  {
    int i = 0;
    int j = 0;
    int k = 0;
    char st[10];
    char ch [10];
    ch1.toCharArray(ch, 10);
   while (i<10)
        {
          if (ch[i] == ';')
            {
               neighboorsTab[k] = (String(st)).toInt();
               for (int u=0; u<10; u++)
                   st[u] = ' ';
               j = 0;
               k++;
              } 
          else
            {
              st[j] = ch[i];
              j++;
            }
            i++;
         }
      }

void sendDataFromSameLevelNodesToCHorBS(int id, int voltage, float x, float y, float chance)
{
    CCPACKET packet1;

    packet1.data[0] = selfLevel - 1;
    packet1.data[1] = 0;
    packet1.data[2] = selfLevel;
    packet1.data[3] = id;
    packet1.data[4] = int(voltage)/100; 
    packet1.data[5] = int(voltage)%100;
    packet1.data[6] = int(x); 
    packet1.data[7] = (x - int(x))*100;
    packet1.data[8] = int(y); 
    packet1.data[9] = (y - int(y))*100;
    packet1.data[10] = int(chance);
    packet1.data[11] = (chance - int(chance))*100;

    packet1.length = 12;

    panstamp.radio.sendData(packet1);
    blinker();
    blinker();
    blinker();
    blinker();
    blinker();
    blinker();
    blinker();
    blinker();
    blinker();
    blinker();
  
}
void sendDataFromCHToCHorBS(int level, int id, int voltage, float x, float y, float sC)
{
  CCPACKET packet1;

    packet1.data[0] = selfLevel - 1;
    packet1.data[1] = 0;
    packet1.data[2] = level;
    packet1.data[3] = id;
    packet1.data[4] = int(voltage)/100; 
    packet1.data[5] = int(voltage)%100;
    packet1.data[6] = int(x); 
    packet1.data[7] = (x - int(x))*100;
    packet1.data[8] = int(y); 
    packet1.data[9] = (y - int(y))*100;
    packet1.data[10] = int(sC);
    packet1.data[11] = (sC - int(sC))*100;
    
    
  // Set packet length
   
  packet1.length = 12;

  // Transmit packet
  panstamp.radio.sendData(packet1);
  blinker();
  blinker();
  blinker();
  blinker();
  blinker();
} 
void saveData(){ // ** TO BE CHANGED **
  
    int id = rxPacket->data[1];
    int voltage = rxPacket->data[2]*100 + rxPacket->data[3];
    float x = rxPacket->data[4] + rxPacket->data[5]/tp;
    float y = rxPacket->data[6] + (float)rxPacket->data[7]/tp;
    float chance = rxPacket->data[8] + (float)rxPacket->data[9]/tp;
    
      if (id == 1)
       {
        neighboursData[0][0] = (float)id;
        neighboursData[0][1] = (float)voltage;
        neighboursData[0][2] = x;
        neighboursData[0][3] = y;
        neighboursData[0][4] = chance;
       }
      else if (id == 2)
       {
         neighboursData[1][0] = (float)id;
         neighboursData[1][1] = (float)voltage;
         neighboursData[1][2] = x;
         neighboursData[1][3] = y; 
         neighboursData[1][4] = chance;
        }
       else if (id == 3)
       {
         neighboursData[2][0] = (float)id;
         neighboursData[2][1] = (float)voltage;
         neighboursData[2][2] = x;
         neighboursData[2][3] = y; 
         neighboursData[2][4] = chance;
        }
      /* else if (id == 4)
       {
         neighboursData[3][0] = (float)id;
         neighboursData[3][1] = (float)voltage;
         neighboursData[3][2] = x;
         neighboursData[3][3] = y; 
         neighboursData[3][4] = chance;
        }
       else if (id == 5)
       {
         neighboursData[4][0] = (float)id;
         neighboursData[4][1] = (float)voltage;
         neighboursData[4][2] = x;
         neighboursData[4][3] = y; 
         neighboursData[4][4] = chance;
        }
       else if (id == 6)
       {
         neighboursData[5][0] = (float)id;
         neighboursData[5][1] = (float)voltage;
         neighboursData[5][2] = x;
         neighboursData[5][3] = y; 
         neighboursData[5][4] = chance;
        }
       else if (id == 7)
       {
         neighboursData[6][0] = (float)id;
         neighboursData[6][1] = (float)voltage;
         neighboursData[6][2] = x;
         neighboursData[6][3] = y; 
         neighboursData[6][4] = chance;
        } */
}

float getDistance(float x1, float y1, float x2, float y2){
  return (float)sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

float sumDistances(){
  float sum;

  for (int i=0; i<nbNeighboors+1; i++){
    if (i+1 != selfId)
      sum = sq(getDistance(neighboursData[selfId-1][2],neighboursData[selfId-1][3],neighboursData[i][2],neighboursData[i][3]));
  }

  return sum;
}

float getEnergy(){

  if (isClusterHead)
    Einit = Einit - (float)(panstamp.getVcc()*(ITx*tTx + IRx*tRx +Isleep*tSleep))*0.000001;
  else
    Einit = Einit - (float)(panstamp.getVcc()*(ITx*tTx +Isleep*tSleep))*0.000001;
    
  return Einit;
}

float getDensity(int totalNodesNumber)
{
  return (float)nbNeighboors/totalNodesNumber;
}

float getCentrality()
{
  return sqrt((sumDistances()/nbNeighboors)/(selfLevel*70));
}


 
//void MessageToNode(int level, int dest_address, String message)
//{
//  CCPACKET packet;
//  int i = 1;
//  
//  //First data in the packet is the destination address
//  
//  packet.data[0] = level;
//  packet.data[1] = dest_address; 
//  
//  // Copy string into packet 
//  while ((message[i-1] != 0) && (i < CCPACKET_DATA_LEN))
//    {
//      packet.data[i+1] = message[i-1];
//      i++;
//    }
//
//  packet.length = i+1;
//  
//  // Transmit packet
//  panstamp.radio.sendData(packet);
//
//  
//}


int HighestChance(int chanceTab[], int n)
 {
  int i = 0;
  int maxi = 0;
  for (i=0; i<n-1; i++){
    if (chanceTab[i+1]>chanceTab[i])
       maxi = i+1;
  }
  return maxi;
 }

void sendSelfData(int level, int id, float voltage, float x, float y, float sC)
{
  CCPACKET packet1;

    packet1.data[0] = level;
    packet1.data[1] = id;
    packet1.data[2] = int(voltage)/100; 
    packet1.data[3] = int(voltage)%100;
    packet1.data[4] = int(x); 
    packet1.data[5] = (x - int(x))*100;
    packet1.data[6] = int(y); 
    packet1.data[7] = (y - int(y))*100;
    packet1.data[8] = int(sC); 
    packet1.data[9] = (sC - int(sC))*100;
       
  // Set packet length
   
  packet1.length = 10;

  // Transmit packet
  panstamp.radio.sendData(packet1);
 // blinker();
}
void setup()
{
  Serial.begin(38400);
  Serial.println("start");
  // Setup LED output pin
  pinMode(LED, OUTPUT);
 // digitalWrite(LED, LOW);
 
  panstamp.setLowTxPower();
  panstamp.init(); 

  Serial.begin(SERIAL_SPEED);
  Serial.flush();
  
  //Begin of Fuzzy Logic code
 
 // Step 2 - Creating a FuzzyInput energy
 FuzzyInput* energy = new FuzzyInput(1); // With its ID in param 

  // Creating the FuzzySet to compond FuzzyInput energy
 FuzzySet* lowE = new FuzzySet(16200, 17700, 18800, 20000); // low energy
 energy->addFuzzySet(lowE); // Add FuzzySet low to energy
 FuzzySet* mediumE = new FuzzySet(20000, 25000, 28500, 30000); // medium energy
 energy->addFuzzySet(mediumE); // Add FuzzySet medium to energy
 FuzzySet* highE = new FuzzySet(30000, 31000, 32000, 32400); // high energy
 energy->addFuzzySet(highE); // Add FuzzySet high to energy

 fuzzy->addFuzzyInput(energy); // Add Fuzzy Input to Fuzzy Object


 FuzzyInput* density = new FuzzyInput(2);// With its ID in param

  // Creating the FuzzySet to compond FuzzyInput density
 FuzzySet* lowD = new FuzzySet(0, 0, 0.1, 0.25); // low density
 energy->addFuzzySet(lowD); // Add FuzzySet low to energy
 FuzzySet* mediumD = new FuzzySet(0.1, 0.2, 0.35, 0.45); // medium density
 energy->addFuzzySet(mediumD); // Add FuzzySet medium to energy
 FuzzySet* highD = new FuzzySet(0.25, 0.4, 0.5, 0.5); // high density
 energy->addFuzzySet(highD); // Add FuzzySet high to energy

 fuzzy->addFuzzyInput(density); // Add Fuzzy Input to Fuzzy Object


 FuzzyInput* centrality = new FuzzyInput(3);// With its ID in param

  // Creating the FuzzySet to compond FuzzyInput energy
 FuzzySet* closeC = new FuzzySet(0, 0.2, 0.4, 0.4); // low energy
 energy->addFuzzySet(closeC); // Add FuzzySet low to energy
 FuzzySet* adequateC = new FuzzySet(0.2, 0.3, 0.5, 0.6); // medium energy
 energy->addFuzzySet(adequateC); // Add FuzzySet medium to energy
 FuzzySet* farC = new FuzzySet(0.4, 0.6, 1, 1); // high energy
 energy->addFuzzySet(farC); // Add FuzzySet high to energy

 fuzzy->addFuzzyInput(centrality); // Add Fuzzy Input to Fuzzy Object

 // Passo 3 - Creating FuzzyOutput chance
 FuzzyOutput* chance = new FuzzyOutput(1);// With its ID in param

 // Creating FuzzySet to compond FuzzyOutput chance
 FuzzySet* veryWeak = new FuzzySet(0, 0, 5, 10); // very weak chance
 chance->addFuzzySet(veryWeak); // Add FuzzySet veryWeak to chance
 FuzzySet* weak = new FuzzySet(0, 5, 15, 20); // weak chance
 chance->addFuzzySet(weak); // Add FuzzySet weak to chance
 FuzzySet* littleWeak = new FuzzySet(10, 15, 25, 30); // little weak chance
 chance->addFuzzySet(littleWeak); // Add FuzzySet littleWeak to chance
 FuzzySet* medium = new FuzzySet(20, 35, 45, 60); // medium chance
 chance->addFuzzySet(medium); // Add FuzzySet medium to chance
 FuzzySet* littleHigh = new FuzzySet(55, 60, 65, 75); // little high chance
 chance->addFuzzySet(littleHigh); // Add FuzzySet little high to chance
 FuzzySet* high = new FuzzySet(65, 70, 75, 95); // high chance
 chance->addFuzzySet(high); // Add FuzzySet high to chance
 FuzzySet* veryHigh = new FuzzySet(80, 90, 100, 100); // very high chance
 chance->addFuzzySet(veryHigh); // Add FuzzySet very high to chance

 fuzzy->addFuzzyOutput(chance); // Add FuzzyOutput to Fuzzy object

 //Passo 4 - Assembly the Fuzzy rules
 
 // Rule 1 Energy=Low Density=Low Centrality=Close
 FuzzyRuleAntecedent* ant011 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant011->joinWithAND(lowE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant1 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant1->joinWithAND(ant011, closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons1 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons1->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ant1, cons1); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule01); // Adding FuzzyRule to Fuzzy object

 // Rule 2 Energy=Low Density=Low Centrality=Adequate
 FuzzyRuleAntecedent* ant021 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant021->joinWithAND(lowE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant2 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant2->joinWithAND(ant021, adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons2 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons2->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ant2, cons2); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule02); // Adding FuzzyRule to Fuzzy object

  // Rule3  Energy=Low Density=Low Centrality=Far
 FuzzyRuleAntecedent* ant031 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant031->joinWithAND(lowE,lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant3 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant3->joinWithAND(ant031 ,farC ); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons3 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons3->addOutput(veryWeak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule03 = new FuzzyRule( 3, ant3, cons3); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule03); // Adding FuzzyRule to Fuzzy object

   // Rule4  Energy=low Density=medium Centrality=close
 FuzzyRuleAntecedent* ant041 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant041->joinWithAND(lowE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant4 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant4->joinWithAND(ant041 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons4 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons4->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule04 = new FuzzyRule(4, ant4, cons4); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule04); // Adding FuzzyRule to Fuzzy object

    // Rule5  Energy=low Density=medium Centrality=adequate
 FuzzyRuleAntecedent* ant051 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant051->joinWithAND(lowE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant5 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant5->joinWithAND(ant051 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons5 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons5->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule05 = new FuzzyRule(5, ant5, cons5); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule04); // Adding FuzzyRule to Fuzzy object

     // Rule6  Energy=low Density=medium Centrality=adequate
 FuzzyRuleAntecedent* ant061 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant061->joinWithAND(lowE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant6 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant6->joinWithAND(ant061 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons6 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons6->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule06 = new FuzzyRule(6, ant6, cons6); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule06); // Adding FuzzyRule to Fuzzy object

      // Rule7  Energy=low Density=high Centrality=close
 FuzzyRuleAntecedent* ant071 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant071->joinWithAND(lowE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant7 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant7->joinWithAND(ant071 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons7 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons7->addOutput(littleWeak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule07 = new FuzzyRule(7, ant7, cons7); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule07); // Adding FuzzyRule to Fuzzy object

       // Rule8  Energy=low Density=high Centrality=adequate
 FuzzyRuleAntecedent* ant081 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant081->joinWithAND(lowE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant8 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant8->joinWithAND(ant081 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons8 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons8->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule08 = new FuzzyRule(8, ant8, cons8); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule08); // Adding FuzzyRule to Fuzzy object

        // Rule9  Energy=low Density=high Centrality=far
 FuzzyRuleAntecedent* ant091 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant091->joinWithAND(lowE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant9 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant9->joinWithAND(ant091 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons9 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons9->addOutput(veryWeak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule09 = new FuzzyRule(9, ant9, cons9); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule09); // Adding FuzzyRule to Fuzzy object

         // Rule10  Energy=medium Density=low Centrality=close
 FuzzyRuleAntecedent* ant101 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant101->joinWithAND(mediumE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant10 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant10->joinWithAND(ant101 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons10 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons10->addOutput(littleHigh);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule10 = new FuzzyRule(10, ant10, cons10); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule10); // Adding FuzzyRule to Fuzzy object

          // Rule11  Energy=medium Density=low Centrality=adequate
 FuzzyRuleAntecedent* ant111 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant111->joinWithAND(mediumE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant11 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant11->joinWithAND(ant111 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons11 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons11->addOutput(medium);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule11 = new FuzzyRule(11, ant11, cons11); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule11); // Adding FuzzyRule to Fuzzy object

           // Rule12  Energy=medium Density=low Centrality=far
 FuzzyRuleAntecedent* ant121 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant121->joinWithAND(mediumE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant12 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant12->joinWithAND(ant121 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons12 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons12->addOutput(weak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule12 = new FuzzyRule(12, ant12, cons12); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule12); // Adding FuzzyRule to Fuzzy object

         // Rule13  Energy=medium Density=medium Centrality=close
 FuzzyRuleAntecedent* ant131 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant131->joinWithAND(mediumE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant13 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant13->joinWithAND(ant131 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons13 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons13->addOutput(high);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule13 = new FuzzyRule(13, ant13, cons13); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule13); // Adding FuzzyRule to Fuzzy object

          // Rule14  Energy=medium Density=medium Centrality=adequate
 FuzzyRuleAntecedent* ant141 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant141->joinWithAND(mediumE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant14 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant14->joinWithAND(ant141 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons14 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons14->addOutput(medium);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule14 = new FuzzyRule(14, ant14, cons14); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule14); // Adding FuzzyRule to Fuzzy object

           // Rule15  Energy=medium Density=low Centrality=far
 FuzzyRuleAntecedent* ant151 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant151->joinWithAND(mediumE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant15 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant15->joinWithAND(ant151 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons15 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons15->addOutput(littleWeak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule15 = new FuzzyRule(15, ant15, cons15); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule15); // Adding FuzzyRule to Fuzzy object

          // Rule16  Energy=medium Density=high Centrality=close
 FuzzyRuleAntecedent* ant161 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant161->joinWithAND(mediumE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant16 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant16->joinWithAND(ant161 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons16 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons16->addOutput(high);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule16 = new FuzzyRule(16, ant16, cons16); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule16); // Adding FuzzyRule to Fuzzy object

          // Rule11  Energy=medium Density=low Centrality=adequate
 FuzzyRuleAntecedent* ant171 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant171->joinWithAND(mediumE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant17 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant17->joinWithAND(ant171 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons17 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons17->addOutput(littleHigh);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule17 = new FuzzyRule(17, ant17, cons17); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule17); // Adding FuzzyRule to Fuzzy object

           // Rule18  Energy=medium Density=high Centrality=far
 FuzzyRuleAntecedent* ant181 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant181->joinWithAND(mediumE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant18 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant18->joinWithAND(ant181 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons18 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons18->addOutput(littleWeak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule18 = new FuzzyRule(18, ant18, cons18); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule18); // Adding FuzzyRule to Fuzzy object

          // Rule19  Energy=high Density=low Centrality=close
 FuzzyRuleAntecedent* ant191 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant191->joinWithAND(highE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant19 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant19->joinWithAND(ant191 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons19 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons19->addOutput(littleHigh);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule19 = new FuzzyRule(19, ant19, cons19); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule19); // Adding FuzzyRule to Fuzzy object

          // Rule20  Energy=high Density=low Centrality=adequate
 FuzzyRuleAntecedent* ant201 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant201->joinWithAND(highE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant20 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant20->joinWithAND(ant201 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons20 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons20->addOutput(medium);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule20 = new FuzzyRule(20, ant20, cons20); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule20); // Adding FuzzyRule to Fuzzy object

           // Rule21  Energy=high Density=low Centrality=far
 FuzzyRuleAntecedent* ant211 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant211->joinWithAND(highE, lowD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant21 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant21->joinWithAND(ant211 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons21 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons21->addOutput(littleWeak);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule21 = new FuzzyRule(21, ant21, cons21); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule21); // Adding FuzzyRule to Fuzzy object

           // Rule22  Energy=high Density=medium Centrality=close
 FuzzyRuleAntecedent* ant221 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant221->joinWithAND(highE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant22 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant22->joinWithAND(ant221 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons22 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons22->addOutput(high);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule22 = new FuzzyRule(22, ant22, cons22); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule22); // Adding FuzzyRule to Fuzzy object

          // Rule23  Energy=high Density=medium Centrality=adequate
 FuzzyRuleAntecedent* ant231 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant231->joinWithAND(highE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant23 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant23->joinWithAND(ant231 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons23 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons23->addOutput(littleHigh);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule23 = new FuzzyRule(23, ant23, cons23); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule23); // Adding FuzzyRule to Fuzzy object

           // Rule24  Energy=high Density=medium Centrality=far
 FuzzyRuleAntecedent* ant241 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant241->joinWithAND(highE, mediumD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant24 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant24->joinWithAND(ant241 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons24 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons24->addOutput(medium);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule24 = new FuzzyRule(24, ant24, cons24); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule24); // Adding FuzzyRule to Fuzzy object

            // Rule25  Energy=high Density=high Centrality=close
 FuzzyRuleAntecedent* ant251 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant251->joinWithAND(highE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant25 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant25->joinWithAND(ant251 , closeC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons25 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons25->addOutput(veryHigh);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule25 = new FuzzyRule(25, ant25, cons25); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule25); // Adding FuzzyRule to Fuzzy object

          // Rule26  Energy=high Density=high Centrality=adequate
 FuzzyRuleAntecedent* ant261 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant261->joinWithAND(highE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant26 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant26->joinWithAND(ant261 , adequateC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons26 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons26->addOutput(littleHigh);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule26 = new FuzzyRule(26, ant26, cons26); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule26); // Adding FuzzyRule to Fuzzy object

           // Rule27  Energy=high Density=high Centrality=far
 FuzzyRuleAntecedent* ant271 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant271->joinWithAND(highE, highD); // Adding corresponding FuzzySet to Antecedent object
  FuzzyRuleAntecedent* ant27 = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ant27->joinWithAND(ant271 , farC); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* cons27 = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 cons27->addOutput(medium);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule27 = new FuzzyRule(27, ant27, cons27); // Passing the Antecedent and the Consequent of expression
 fuzzy->addFuzzyRule(fuzzyRule27); // Adding FuzzyRule to Fuzzy object

 

 // End of Fuzzy logic code
  voltage = panstamp.getVcc()/1000;

  panstamp.init(); 

 /* panstamp.radio.setChannel(RFCHANNEL);
  panstamp.radio.setSyncWord(SYNCWORD1, SYNCWORD0);
  panstamp.radio.setDevAddress(SOURCE_ADDR);
  panstamp.radio.setCCregs(); */

  // Let's disable address check for the current project so that our device
  // will receive packets even not addressed to it.
  panstamp.radio.disableAddressCheck();

  // Declare RF callback function
  rfPacketReceived(rxPacket);
  panstamp.attachInterrupt(rfPacketReceived);
  
  // Declare RF callback function
 // panstamp.setPacketRxCallback(rfPacketReceived);
  Serial.println("device initialized");
  Serial.println("done");
  
}

void loop()
{
    voltage = panstamp.getVcc();
  //  voltStr+=String(int(voltage))+ "."+String(getDecimal(voltage));                                                                                              // Self-incrementing value  
    escapePacket = false;

    fuzzy->setInput(1,getEnergy());
    fuzzy->setInput(2,getDensity(2));
    fuzzy->setInput(3,getCentrality());

    fuzzy->fuzzify();
    selfChance = fuzzy->defuzzify(1);

    Serial.println(selfChance);
    
    if (isClusterHead == false){
    sendSelfData(selfLevel,selfId,voltage,X,Y,selfChance);
   // panstamp.sleepSec(10);
    
    }

    if (notClusterHead == true)
    {
         sendSelfData(selfLevel,selfId,voltage,X,Y,selfChance);
         delay(3000);
         sendSelfData(selfLevel,selfId,voltage,X,Y,selfChance);
         delay(3000);
         sendSelfData(selfLevel,selfId,voltage,X,Y,selfChance);
         delay(10000);
    }
    
    if (packetAvailable){

//      if (rxPacket->data[0] != selfLevel)
//           escapePacket = true;
//           
//      if (escapePacket == false)
//        {  
         if ((isClusterHead == false) && (notClusterHead == false) && (rxPacket->data[0] == selfLevel))
      {
         if (rxPacket->data[1] == 2) // ** TO BE CHANGED **
         {
          saveData();
          node2 = true; // ** TO BE CHANGED **
         }
         if (rxPacket->data[1] == 3) // ** TO BE CHANGED **
         {
          saveData();
          node3 = true; // ** TO BE CHANGED **
         }
          
        //nPack ++;
        
        if ((node2 == true) && (node3 == true)) // ** TO BE CHANGED **
        {
        chanceTab[selfId-1] = selfChance;
        if (HighestChance(chanceTab, 2) + 1 == selfId) // The node is selected as Cluster Head
        {
          isClusterHead = true;
            blinker();
            blinker();
            blinker();
            blinker();
            blinker();
            blinker();
            blinker();
            blinker();
            blinker();
        }
        else // The node is not selected as Cluster Head
         notClusterHead = true;
         
       // nPack = 0;
        
        }
      }
          
       // if the node is selected as a Cluster Head
      if ((isClusterHead == true) && (rxPacket->data[0] == selfLevel))
       {
        // transmit the received packets from nodes of same level
        if ((rxPacket->data[0] == selfLevel) && (rxPacket->data[1] != 0))
           {
            nbPacketsFromSameLevel++;
            int id = rxPacket->data[1];
            int voltage1 = rxPacket->data[2]*100 + rxPacket->data[3];
            float x = rxPacket->data[4] + rxPacket->data[5]/tp;
            float y = rxPacket->data[6] + (float)rxPacket->data[7]/tp;
            float chance = rxPacket->data[8] + (float)rxPacket->data[9]/tp;
            sendDataFromSameLevelNodesToCHorBS(id, voltage1, x, y, chance);
            if (nbPacketsFromSameLevel == 2) //** TO BE CHANGED **//
            {
              sendDataFromSameLevelNodesToCHorBS(selfId, voltage, X, Y, selfChance);
              nbPacketsFromSameLevel = 0;   //** TO BE CHANGED **//
            }
           }
        // Forward the received packet from another Cluster Head to other Cluster Head or Base Station
        if ((rxPacket->data[0] == selfLevel) && (rxPacket->data[1] == 0))
        {   
            int level = rxPacket->data[2];
            int id = rxPacket->data[3];
            int voltage1 = rxPacket->data[4]*100 + rxPacket->data[5];
            float x = rxPacket->data[6] + rxPacket->data[7]/tp;
            float y = rxPacket->data[8] + (float)rxPacket->data[9]/tp;
            float chance = rxPacket->data[10] + (float)rxPacket->data[11];
            sendDataFromCHToCHorBS(level, id, voltage1, x, y, chance);
         }
         
       }
       
  //  }
      }
  
  // Enable wireless reception
    panstamp.rxOn();
  // For low-power applications replace "delay" by "panstamp.sleepWd(WDTO_8S)" for example
}

