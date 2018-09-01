/*
   modem.pde

   Copyright (c) 2014 panStamp <contact@panstamp.com>

   This file is part of the panStamp project.

   panStamp  is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   panStamp is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with panStamp; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
   USA

   Author: Daniel Berenguer
   Creation date: 15/02/2011

   Device:
   Serial gateway or modem

   Description:
   This is not a proper SWAP gateway but a simple transparent UART-RF
   interface. This device can be used from a serial console as hyperterminal
   or minicom. Wireless packets are passed to/from the host computer in ASCII
   format whilst configuration is done via AT commands.

   Visit our wiki for details about the protocol in case you want to develop
   your own PC library for this device.
*/
//#include <rExcel.h>
#define FIREBASE_HOST "https://fuzzy-based-clustering.firebaseio.com/"
#define FIREBASE_AUTH "v6bvdkFI11lauHmwz83luLOhnv3mb3VWWc8wWtRv"
#include <Chrono.h>
#include <FuzzyRule.h>
#include <FuzzyComposition.h>
#include <Fuzzy.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzyOutput.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzySet.h>
#include <FuzzyRuleAntecedent.h>
#include "modem.h"
#ifdef PANSTAMP_NRG
#include "timer1a0.h"
#define TIMER         timer1a0
#define RESET_TIMER()
#define INIT_TIMER()  TIMER.attachInterrupt(isrT1event)
#define START_TIMER() TIMER.start(1000)
#define STOP_TIMER()  TIMER.stop()
#endif

/**
   LED pin
*/
#define LEDPIN  4
// Step 1 -  Instantiating an object library
Fuzzy* fuzzy = new Fuzzy();

byte charToHex(byte ch);
String packetString;
char str;
int NumberOfPacketsfromCH1toBS = 0;
int NumberOfPacketsfromNodetoCH1 = 0;
int NumberOfPacketsfromBStoCH1 = 0;
int numPack = 0;
int numPackFromLevel1 = 0;
int numPackFromLevel2 = 0;
int NumPackFromCH = 0;
float node[4][4];
float level2Nodes[3][5];
float level1chances[5];
float level2Chances[2];
float distance[5][5];
float level2Distances[2][2];
String neighboors[5];
String level2Neighboors[2];
float  tp = 100;
int neighboorsTab[10];
int level2NeighboorsTab[10];
int i = 0;
int level1CHid = 0;
int level2CHid = 0;
bool valid = false;
Chrono chrono;
bool l2n1 = false;
bool l2n2 = false;
bool l1n1 = false;
bool l1n2 = false;
bool l1n3 = false;
float Einit = 16524.73;
float Einit1 = 32400;
float Isleep = 0.00046;
float ITx = 0.0018;
float IRx = 0.001221;
float tTx = 1.2;
float tRx = 1.2;
float tSleep = 10000;
/**
   This function is called whenever a wireless packet is received
*/

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
      
void rfPacketReceived(CCPACKET *packet)
{
  if (packet->length > /*5*/0)
  {
    rxPacket = packet;
    packetAvailable = true;
  }
}

void printChrono() {

 Serial.print(chrono.elapsed());
 Serial.print(" ms");
}

void blinker()
{
  digitalWrite(LED, !digitalRead(LED));
}
/**
   isrT1event

   Timer1 interrupt routine
*/
void isrT1event(void)
{
    // Detach Timer1 interrupt
    STOP_TIMER();
    RESET_TIMER();
    // Pending "+++" command?
    if (!strcmp(strSerial, AT_GOTO_CMDMODE))
    {
      panstamp.rxOff();  // Disable wireless reception
      Serial.println("OK-Command mode");
      serMode = SERMODE_COMMAND;
    }
    memset(strSerial, 0, sizeof(strSerial));
    len = 0;
}


void setup()
{
  
  
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);


  // Enter high ITx power mode
//  panstamp.setHighITxPower();
 
  // Long distance board?
  //panstamp.radio.enableHGM();

  // Reset serial buffer
 // memset(strSerial, 0, sizeof(strSerial));

  Serial.begin(SERIAL_SPEED);
  Serial.flush();
  panstamp.init(); 
  chrono.restart();
  printChrono();
  Serial.println("modem ready !");
//  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
  /*char ch[] = "1,2,3,4";
   while (i<10)
        {
          if (ch[i] == ',')
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
         } */
 

 //Begin of Fuzzy Logic code
 
 // Step 2 - Creating a FuzzyInput energy
 FuzzyInput* energy = new FuzzyInput(1);// With its ID in param

  // Creating the FuzzySet to compond FuzzyInput energy
 FuzzySet* low = new FuzzySet(0, 0, 500, 1000); // low energy
 energy->addFuzzySet(low); // Add FuzzySet low to energy
 FuzzySet* medium = new FuzzySet(500, 1000, 2500, 3000); // medium energy
 energy->addFuzzySet(medium); // Add FuzzySet medium to energy
 FuzzySet* high = new FuzzySet(2500, 3000, 3250, 3500); // high energy
 energy->addFuzzySet(high); // Add FuzzySet high to energy

 fuzzy->addFuzzyInput(energy); // Add Fuzzy Input to Fuzzy Object


 // Passo 3 - Creating FuzzyOutput chance
 FuzzyOutput* chance = new FuzzyOutput(1);// With its ID in param

 // Creating FuzzySet to compond FuzzyOutput chance
 FuzzySet* bad = new FuzzySet(0, 0, 10, 25); // bad chance
 chance->addFuzzySet(bad); // Add FuzzySet bad to chance
 FuzzySet* average = new FuzzySet(10, 25, 75, 80); // medium chance
 chance->addFuzzySet(average); // Add FuzzySet medium to chance
 FuzzySet* good = new FuzzySet(75, 80, 100, 100); // good chance
 chance->addFuzzySet(good); // Add FuzzySet good to chance

 fuzzy->addFuzzyOutput(chance); // Add FuzzyOutput to Fuzzy object

 //Passo 4 - Assembly the Fuzzy rules
 // FuzzyRule "IF energy = low THEN chance = bad"
 FuzzyRuleAntecedent* ifEnergyLow = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ifEnergyLow->joinSingle(low); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* thenChanceBad = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 thenChanceBad->addOutput(bad);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifEnergyLow, thenChanceBad); // Passing the Antecedent and the Consequent of expression
 
 fuzzy->addFuzzyRule(fuzzyRule01); // Adding FuzzyRule to Fuzzy object


 // FuzzyRule "IF energy = medium THEN chance = medium"
 FuzzyRuleAntecedent* ifEnergyMedium = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ifEnergyMedium->joinSingle(medium); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* thenChanceAverage = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 thenChanceAverage->addOutput(average); // Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifEnergyMedium, thenChanceAverage); // Passing the Antecedent and the Consequent of expression
 
 fuzzy->addFuzzyRule(fuzzyRule02); // Adding FuzzyRule to Fuzzy object


 // FuzzyRule "IF energy = high THEN chance = good"
 FuzzyRuleAntecedent* ifEnergyHigh = new FuzzyRuleAntecedent(); // Instantiating an Antecedent to expression
 ifEnergyHigh->joinSingle(high); // Adding corresponding FuzzySet to Antecedent object
 FuzzyRuleConsequent* thenChanceGood = new FuzzyRuleConsequent(); // Instantiating a Consequent to expression
 thenChanceGood->addOutput(good);// Adding corresponding FuzzySet to Consequent object
 // Instantiating a FuzzyRule object
 FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifEnergyHigh, thenChanceGood); // Passing the Antecedent and the Consequent of expression
 
 fuzzy->addFuzzyRule(fuzzyRule03); // Adding FuzzyRule to Fuzzy object

 // End of Fuzzy logic code

  // Disable address check from the RF IC
  panstamp.radio.disableAddressCheck();

  // Declare RF callback function
  rfPacketReceived(rxPacket);
  panstamp.attachInterrupt(rfPacketReceived);
  
  // Initialize Timer object
  INIT_TIMER();

  digitalWrite(LEDPIN, LOW);
}

int HighestChance(float chance[], int n)
 {
  int i = 0;
  int max = 0;
  for (i=0; i<n-1; i++){
    if (chance[i+1]>chance[i])
       max = i+1;
  }
  return max;
 }




void MessageToNode(int level, int dest_address, String message)
{
  CCPACKET packet;
  int i = 1;
  
  //First data in the packet is the destination address
  
  packet.data[0] = level;
  packet.data[1] = dest_address; 
  
  // Copy string into packet 
  while ((message[i-1] != 0) && (i < CCPACKET_DATA_LEN))
    {
      packet.data[i+1] = message[i-1];
      i++;
    }

  packet.length = i+1;

  // Transmit packet
  panstamp.radio.sendData(packet);

}

void SendData(int level, int id, int x)
{
  CCPACKET packet1;

    packet1.data[0] = level;
    packet1.data[1] = id;
    packet1.data[2] = x; 
    
  // Set packet length
   
  packet1.length = 3;

  // Transmit packet
  panstamp.radio.sendData(packet1);
}

void loop()
{
  
  // Read wireless packet?

 if (packetAvailable) // if packetAvailable 
  {

    //  Serial.println((float)rxPacket->data[0] + (float)rxPacket->data[1]/100);
    //  Serial.println((float)rxPacket->data[2] + (float)rxPacket->data[3]/100);
   // if (rxPacket->data[0] == 2

   if (rxPacket->data[1] != 0){
     Serial.print("node ");
     Serial.println(rxPacket->data[1]);
     Serial.print("level ");
     Serial.println(rxPacket->data[0]);
      float volt1 = rxPacket->data[2]*100 + rxPacket->data[3];
      float chance = rxPacket->data[8] + (float)rxPacket->data[9]/100;
      Serial.print("chance ");
      Serial.print(chance);
      Serial.println("%");
   }
//         if (rxPacket->data[1] == 3){
//           Einit1 = Einit1 - (float)(volt1*(ITx*tTx +Isleep*tSleep))*0.000001;
//           Serial.println(Einit1); 
//         }
//         else 
//         {
//           Einit = Einit - (float)(volt1*(ITx*tTx +Isleep*tSleep))*0.000001;
//           Serial.println(Einit);
//         }

      

       
    digitalWrite(LEDPIN, HIGH);
    
    // Disable wireless reception
    panstamp.rxOff();

    byte i;
    packetAvailable = false;

     // Enable wireless reception
    panstamp.rxOn();

    if (serMode == SERMODE_DATA)
    {
      
     // char floatbuf[32];
      int level = rxPacket->data[0];
      int id = rxPacket->data[1];
     
      
      
     if ((level == 0) && (id == 0)/* && (valid == true)*/)
     { 
      NumPackFromCH ++;
//      printChrono();
//      Serial.print(" ");
//      Serial.print(NumPackFromCH);
//      Serial.println(" packets from Cluster Head to Base Station");
      int level1 = rxPacket->data[2];
      int id1 = rxPacket->data[3];
      float voltage1 = rxPacket->data[4]*100 + rxPacket->data[5];
      float x1 = rxPacket->data[6] + rxPacket->data[7]/tp;
      float y1 = rxPacket->data[8] + (float)rxPacket->data[9]/tp;
      float chance1 = rxPacket->data[10] + (float)rxPacket->data[11]/tp;
      
      if (level1 == 2)
      {
        numPackFromLevel2++;
//        printChrono();
//        Serial.print(" ");
//        Serial.print(numPackFromLevel2);
//        Serial.println(" packets from level 2");
        
      if (id1 == 1)
       {
        l2n1 = true;
//        Serial.print("level 2 node 1 packet arrived at : ");
//        printChrono();
        level2Nodes[0][0] = (float)id1;
        level2Nodes[0][1] = Einit - (float)(voltage1*(ITx*tTx + Isleep*tSleep))*0.000001;
        level2Nodes[0][2] = x1;
        level2Nodes[0][3] = y1;
        level2Nodes[0][4] = chance1;
       }
      else if (id1 == 2)
       {
         l2n2 = true;
//         Serial.print("level 2 node 2 packet arrived at : ");
//         printChrono();
         level2Nodes[1][0] = (float)id1;
         level2Nodes[1][1] = Einit - (float)(voltage1*(ITx*tTx + IRx*tRx +Isleep*tSleep))*0.000001;
         level2Nodes[1][2] = x1;
         level2Nodes[1][3] = y1;
         level2Nodes[1][4] = chance1;
        }
     /*  else if (id1 == 3)
       {
         level2Nodes[2][0] = (float)id1;
         level2Nodes[2][1] = (float)voltage1;
         level2Nodes[2][2] = x1;
         level2Nodes[2][3] = y1;
         level2Nodes[2][4] = chance1;
        }
       else if (id == 4)
       {
         level2Nodes[3][0] = (float)id1;
         level2Nodes[3][1] = (float)voltage1;
         level2Nodes[3][2] = x1;
         level2Nodes[3][3] = y1; 
         level2Nodes[3][4] = chance1;
        }
       else if (id == 5)
       {
         level2Nodes[4][0] = (float)id1;
         level2Nodes[4][1] = (float)voltage1;
         level2Nodes[4][2] = x1;
         level2Nodes[4][3] = y1; 
         level2Nodes[4][4] = chance1;
        }
       else if (id == 6)
       {
         level2Nodes[5][0] = (float)id1;
         level2Nodes[5][1] = (float)voltage1;
         level2Nodes[5][2] = x1;
         level2Nodes[5][3] = y1; 
         level2Nodes[5][4] = chance1;
        }
       else if (id == 7)
       {
         level2Nodes[6][0] = (float)id1;
         level2Nodes[6][1] = (float)voltage1;
         level2Nodes[6][2] = x1;
         level2Nodes[6][3] = y1; 
         level2Nodes[6][4] = chance1;
        } */

      if ((l2n1) && (l2n2))  // ** TO BE CHANGED **
      {
        Serial.println("Level 2 nodes data : ");
        for (int i=0; i<2; i++)
              {
                printChrono();
                Serial.print(" ");
                if (i+1 == 2)
                {
                  Serial.print("Cluster Head : ");
                }
                else
                { 
                  Serial.print("node ");
                  Serial.print(i+1);
                  Serial.print(" : ");
                }
                Serial.print((int)level2Nodes[i][1]);
                Serial.println(" J");
                Serial.print("Chance : ");
                Serial.print(level2Nodes[i][4]);
                Serial.println(" %");
                
              }
//        Serial.println("Level 2 nodes data : ");
//      for (int u=1; u<3; u++)
//         for (int v=1; v<3; v++)
//          {
//             level2Distances[u][v] = sqrt((level2Nodes[u-1][2] - level2Nodes[v-1][2])*(level2Nodes[u-1][2] - level2Nodes[v-1][2]) + (level2Nodes[u-1][3] - level2Nodes[v-1][3])*(level2Nodes[u-1][3] - level2Nodes[v-1][3])) ;       
//             
//          } 
//            Serial.print("      id    |");
//             for (int i=0; i<2; i++)
//              {
//                Serial.print("    ");
//                Serial.print((int)level2Nodes[i][0]);
//                Serial.print("    |");
//              }
//                Serial.println("");
//                Serial.println("  -------------------------------------------");
//             Serial.print("   voltage  |");    
//             for (int i=0; i<2; i++)
//              {
//                Serial.print(" ");
//                Serial.print((int)level2Nodes[i][1]);
//                Serial.print(" mV");
//                Serial.print(" |");
//              }
//                Serial.println("");
//                Serial.println("  -------------------------------------------");
//              Serial.print("  X (meters)|");
//              for (int i=0; i<2; i++)
//              {
//                Serial.print("  ");
//                Serial.print(level2Nodes[i][2]);
//                Serial.print("  |");
//              }
//                Serial.println("");
//                Serial.println("  -------------------------------------------");
//              Serial.print("  Y (meters)|"); 
//              for (int i=0; i<2; i++)
//              {
//                Serial.print("  ");
//                Serial.print(level2Nodes[i][3]);
//                Serial.print("  |");
//              }
//                Serial.println("");
//                Serial.println("");
//            for (int u=1; u<3; u++)
//               for (int v=1; v<3; v++)
//               {
//                  if (u != v)
//                   {
//                    if (level2Distances[u][v] < 70)
//                       {
//                        level2Neighboors[u] += String(v) + "; ";
//                       }
//                     Serial.print("  distance between nodes ");
//                     Serial.print(u);
//                     Serial.print(" and ");
//                     Serial.print(v);
//                     Serial.print(" is ");
//                     Serial.print(level2Distances[u][v]);
//                     Serial.println (" meters");
//                     }
//             }
//             Serial.println("  -------------------------------------------");
//           for (int k=1; k<3; k++)
//                     {
//                       Serial.print("  node");
//                       Serial.print(k);
//                       Serial.print(" neighboors are : ");
//                       Serial.println(level2Neighboors[k]);
//                       neighboorsIntTab (level2Neighboors[k], level2NeighboorsTab);
//                       //MessageToNode(1, level1CHid, level2Neighboors[k]);
//                       level2Neighboors[k] = "";
//                     }
//                 Serial.println("  --------------------------------------------");
//                 Serial.println("  --------------------------------------------");
//          for (int u=0; u<2; u++)
//        {
//         fuzzy->setInput(1, level2Nodes[u][1]); 
//         // Step 6 - Exe the fuzzification 
//         fuzzy->fuzzify(); 
//         // Step 7 - Exe the desfuzzyficação for each output, passing its ID
//         level2Chances[u] = fuzzy->defuzzify(1);
//        }
//        for (int u=0; u<2; u++){
//           Serial.print("  node");
//           Serial.print(u+1);
//           Serial.print(" chance : ");
//           Serial.print(level2Chances[u]); 
//           Serial.println("%");
//        }
//
//        // Step 5 - Report inputs value, passing its ID and value
//
//     level2CHid = HighestChance(level2Chances,2) + 1;
//     Serial.println("  ");
//     Serial.print(" node ");
//     Serial.print(level2CHid);
//     Serial.println(" is selected as Cluster Head");
//     Serial.println("");
//     Serial.println("  --------------------------------------------");
//     Serial.println("  --------------------------------------------");
//     
//     l2n1 = false;
//     l2n2 = false;
      }
      
      //SendData(1, 1, 9);
     }
     
    
     else if ((level1 == 1) /*&& (valid == false)*/)
     {
        numPackFromLevel1++;
//        printChrono();
//        Serial.print(" ");
//        Serial.print(numPackFromLevel1);
//        Serial.println(" packets from level 1");
        if (id1 == 1)
         {
          l1n1 = true;
//          Serial.print("level 1 node 1 packet arrived at : ");
//          printChrono();
          node[0][0] = (float)id1;
          node[0][1] = Einit - (float)(voltage1*(ITx*tTx + Isleep*tSleep))*0.000001;
          node[0][2] = x1;
          node[0][3] = y1;
         }
        else if (id1 == 2)
         {
           l1n2 = true;
//           Serial.print("level 1 node 2 packet arrived at : ");
//           printChrono();
           node[1][0] = (float)id1;
           node[1][1] = Einit - (float)(voltage1*(ITx*tTx + Isleep*tSleep))*0.000001;
           node[1][2] = x1;
           node[1][3] = y1; 
          }
         else if (id1 == 3)
         {
           l1n3 = true;
//           Serial.print("level 1 node 3 packet arrived at : ");
//           printChrono();
           node[2][0] = (float)id1;
           node[2][1] = Einit - (voltage1*(float)(ITx*tTx + IRx*tRx + Isleep*tSleep))*0.000001;
           node[2][2] = x1;
           node[2][3] = y1; 
          }
        /* else if (id == 4)
         {
           node[3][0] = (float)id;
           node[3][1] = (float)voltage;
           node[3][2] = x;
           node[3][3] = y; 
          } */
     
   if ((l1n1) && (l1n2) && (l1n3))
    {
      Serial.println("Level 1 nodes data : ");
        for (int i=0; i<3; i++)
              {
                printChrono();
                Serial.print(" ");
                if (i+1 == 3)
                {
                  Serial.print("Cluster Head : ");
                }
                else
                { 
                  Serial.print("node ");
                  Serial.print(i+1);
                  Serial.print(" : ");
                }
                Serial.print((int)node[i][1]);
                Serial.println(" J");
              }
//      Serial.print("All packets received at : ");
//      printChrono();
//      for (int u=1; u<4; u++)
//         for (int v=1; v<4; v++)
//          {
//             distance[u][v] = sqrt((node[u-1][2] - node[v-1][2])*(node[u-1][2] - node[v-1][2]) + (node[u-1][3] - node[v-1][3])*(node[u-1][3] - node[v-1][3])) ;       
//             
//          } 
//            Serial.print("      id    |");
//             for (int i=0; i<3; i++)
//              {
//                Serial.print("    ");
//                Serial.print((int)node[i][0]);
//                Serial.print("    |");
//              }
//                Serial.println("");
//                Serial.println("  -------------------------------------------");
//             Serial.print("   voltage  |");    
//             for (int i=0; i<3; i++)
//              {
//                Serial.print(" ");
//                Serial.print((int)node[i][1]);
//                Serial.print(" mV");
//                Serial.print(" |");
//              }
//                Serial.println("");
//                Serial.println("  -------------------------------------------");
//              Serial.print("  X (meters)|");
//              for (int i=0; i<3; i++)
//              {
//                Serial.print("  ");
//                Serial.print(node[i][2]);
//                Serial.print("  |");
//              }
//                Serial.println("");
//                Serial.println("  -------------------------------------------");
//              Serial.print("  Y (meters)|"); 
//              for (int i=0; i<3; i++)
//              {
//                Serial.print("  ");
//                Serial.print(node[i][3]);
//                Serial.print("  |");
//              }
//                Serial.println("");
//                Serial.println("");
//            for (int u=1; u<4; u++)
//               for (int v=1; v<4; v++)
//               {
//                  if (u != v)
//                   {
//                    if (distance[u][v] < 70)
//                       {
//                        neighboors[u] += String(v) + "; ";
//                       }
//                     Serial.print("  distance between nodes ");
//                     Serial.print(u);
//                     Serial.print(" and ");
//                     Serial.print(v);
//                     Serial.print(" is ");
//                     Serial.print(distance[u][v]);
//                     Serial.println (" meters");
//                     }
//                 
//             }
//             Serial.println("  -------------------------------------------");
//           for (int k=1; k<4; k++)
//                     {
//                       Serial.print("  node");
//                       Serial.print(k);
//                       Serial.print(" neighboors are : ");
//                       Serial.println(neighboors[k]);
//                       neighboorsIntTab (neighboors[k], neighboorsTab);
//                       for (int u=0; neighboorsTab[u] == 0; u++)
//                         {
//                          Serial.print("neighor ");
//                          Serial.print(u+1);
//                          Serial.print(" is ");
//                          Serial.println(neighboorsTab[u]);
//                         }
//                       MessageToNode(level, k, neighboors[k]);
//                       neighboors[k] = "";
//                     }
//                 Serial.println("  --------------------------------------------");
//                 Serial.println("  --------------------------------------------");
//          for (int u=0; u<3; u++)
//        {
//         fuzzy->setInput(1, node[u][1]); 
//         // Step 6 - Exe the fuzzification 
//         fuzzy->fuzzify(); 
//         // Step 7 - Exe the desfuzzyficação for each output, passing its ID
//         level1chances[u] = fuzzy->defuzzify(1);
//        }
//        
//        for (int u=0; u<3; u++){
//           Serial.print("  node");
//           Serial.print(u+1);
//           Serial.print(" chance : ");
//           Serial.print(level1chances[u]); 
//           Serial.println("%");
//        }
//
//     //HighestChance(level1chances,4) + 1;
//     Serial.println("  ");
//     Serial.print(" node ");
//     Serial.print(HighestChance(level1chances,3) + 1);
//     Serial.println(" is selected as Cluster Head");
//     Serial.println("");
//     
//     Serial.println("  --------------------------------------------");
//     Serial.println("  --------------------------------------------"); 
    /* Serial.print(" Number of packet from CH1 to BS = ");
     Serial.print(NumberOfPacketsfromCH1toBS);
     Serial.print(" Number of packet from node to CH1 = ");
     Serial.print(NumberOfPacketsfromNodetoCH1);
     Serial.print(" Number of packet from BS to CH1 = ");
     Serial.print(NumberOfPacketsfromBStoCH1); */
//     delay(5000); 

     l1n1 = false;
     l1n2 = false;
     l1n3 = false;
     }
 
     }  
     }
   //SendData(1, 1, 9);
   //SendData(level, level1CHid, 8);
  // Serial.print("msg sent");
    // Enable wireless reception
    panstamp.rxOn();
    digitalWrite(LEDPIN, LOW);
  } 

  // Read serial command
 /* if (Serial.available() > 0)
  {
    // Disable wireless reception
    panstamp.rxOff();

    ch = Serial.read();

    if (len >= SERIAL_BUF_LEN - 1)
    {
      memset(strSerial, 0, sizeof(strSerial));
      len = 0;
    }
    else if (ch == 0x0D)
    {
      STOP_TIMER();
      strSerial[len] = 0;
//      handleSerialCmd(strSerial);
      memset(strSerial, 0, sizeof(strSerial));
      len = 0;
    }
    else if (ch != 0x0A)
    {
      strSerial[len] = ch;
      len++;
      START_TIMER();
    }

    // Enable wireless reception
    panstamp.rxOn();
  } */
  }
panstamp.rxOn();
}



