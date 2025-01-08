
//===========================================================
//
//  File Name:           laserPointer.c
//  Project Name:        LP_Jetson
//
//  Project Engineer:    Jacob M. Romero
//                       Creative Engineering Solutions, LLC
//                       jrom876@gmail.com
//  Github Repo:         https://github.com/jrom876/LP_Jetson
//
// Jetson Nano:         ARM A57 4 cores
// Zotac machine:       Intel Core i7 9750H 6 cores/12 threads
//
//  ----------
//  Copyright
//  ----------
//
//  Copyright (C) Creative Engineering Solutions, LLC - All Rights Reserved
//  This file is the property of Creative Engineering Solutions, LLC
//  Unauthorized copying of this file, via any medium is strictly prohibited.
//  Proprietary and confidential
//
//===========================================================
// https://raspberry-projects.com/pi/programming-in-c/gui-programming-in-c/gtk/installing-gtk
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "laserPointer.h"
//#include <gtk/gtk.h>

#define PI 3.14159265359

int GLOBAL_TARGETS_DETECTED = 0;
double GLOBAL_BLANKER_PRR = 0.0;
double GLOBAL_BLANKER_PWM  = 0.0;

char *temper[6] = {"unspecified","startled","blinded","agressive","flee"};
//////
//======= LaserPointer =======//

struct LaserPointer createLaser(int x, int y, float pr, float pw){
  struct LaserPointer lp;
  lp.xcoord = ((x > XMAX) || (x < XMIN)) ? 1 : x;
  lp.ycoord = ((y > YMAX) || (y < YMIN)) ? 1 : y;
  lp.prr = (pr < 0) ? 0 : pr;
  lp.pwm = ((pw > 1) || (pw < 0)) ? 0 : pw;
  printf("\nINITIAL LASER VALUES: \n"); // DBPRINT
  printf("%i : %i : %0.3f : %0.3f\n",lp.xcoord,
           lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}
////////////////////////////
////======= Target =======//
struct Target createTarget(int x, int y, float dist, char disp, int pel, float ambl, char* tid){
  struct Target tar;
  tar.xloc = ((x > XMAX) || (x < XMIN)) ? 1 : x;
  tar.yloc = ((y > YMAX) || (y < YMIN)) ? 1 : y;
  tar.dist = ((dist > 100) || (dist < 0)) ? 1 : dist;
  tar.disp = ((disp != 'S')&&(disp != 'B')&&(disp != 'A')&&(disp != 'F')) ? 'U' : disp;
  tar.pel = pel < 0 ? 0 : pel;
  tar.ambLight = ambl;
  tar.targetID = tid;
  // tar.mytuple = (tar.xloc,tar.yloc,tar.dist,tar.disp,tar.pel)
  printf("\nINITIAL TARGET VALUES: \n"); // DBPRINT
  printf("%i : %i : %0.3f : %c : %i : %s\n",tar.xloc,
           tar.yloc,tar.dist,tar.disp,tar.pel,tar.targetID); //DBPRINT
  return tar;
}

/////////////////////////////////////
//////// Construction Zone //////////
///////////////////////////////

// int updateTarget(struct Node* node, struct Target target) {
//     struct Node* head = NULL;
//     struct Node* second = NULL;
//     struct Node* third = NULL;
//     // allocate 3 nodes in the heap
//     head = (struct Node*)malloc(sizeof(struct Node));
//     second = (struct Node*)malloc(sizeof(struct Node));
//     third = (struct Node*)malloc(sizeof(struct Node));
//     return 0;
// }

// This function prints contents of linked list
// starting from the given node
// use function call: printList(head);
void printList(struct Node* n)
{
    while (n != NULL) {
        printf("X %i : Y %i : dist %0.2f : disp %c : pel %i : ambLight %0.2f : TID %s\n",n->target.xloc,n->target.yloc,
                n->target.dist,n->target.disp,n->target.pel,n->target.ambLight,n->target.targetID);
        n = n->next;
    }
}

void push(struct Node** head_ref, struct Target new_target)
{
    // allocate node
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
    // put in the target
    new_node->target = new_target;
    // link the old list off the new node
    new_node->next = (*head_ref);
    // move the head to point to the new node
    (*head_ref) = new_node;
}

/* Given a reference (pointer to pointer) to the head of a list
   and a tid, deletes the first occurrence of tid in linked list */
void deleteNode(struct Node **head_ref, char* tid){
    // Store head node
    struct Node* temp = *head_ref, *prev;
    // If head node itself holds the tid to be deleted
    if (temp != NULL && temp->target.targetID == tid){
        *head_ref = temp->next;   // Changed head
        free(temp);               // free old head
        return;
    }
    // Search for the tid to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && temp->target.targetID != tid) {
        prev = temp;
        temp = temp->next;
    }
    // If tid was not present in linked list
    if (temp == NULL) return;
    // Unlink the node from linked list
    prev->next = temp->next;
    free(temp);  // Free memory
}

struct Target getNthTarget(struct Node* head, int index){
    struct Node* current = head;
  // the index of the node we're currently looking at
    int count = 0;
    while (current != NULL){
        if (count == index){
            printf("%s\n",current->target.targetID);
            return(current->target);
        }
        count++;
        current = current->next;
    }
    // if it gets to here, there is a problem
    printf("disp %c",current->target.disp);
    return head->target;
}

/* Function to delete the entire linked list */
void deleteList(struct Node** head_ref){
   /* deref head_ref to get the real head */
   struct Node* current = *head_ref;
   struct Node* next;

   while (current != NULL)
   {
       next = current->next;
       free(current);
       current = next;
   }
   /* deref head_ref to affect the real head back
      in the caller. */
   *head_ref = NULL;
}

int checkPEL(struct Node* head, int limit)
{
    struct Node* current = head;
    int count = 0;
    while (current != NULL) {
        if (current->target.pel >= limit){
            count++;
            printf("Target %s over PEL: %i\n",current->target.targetID,current->target.pel);
          }
        current = current->next;
    }
    printf("Number of targets over PEL: %i\n",count);
    return count;
}

int targetsDetected(int flag){
  int result = GLOBAL_TARGETS_DETECTED += flag;
  return result;
}

void clearTargetsDetected(){
  GLOBAL_TARGETS_DETECTED = 0;
}

//////// End of Construction Zone //////////
////////////////////////////////////////////////////

/////////////////////////////////////////////////

////======= Setters ======////
struct LaserPointer copyLP(struct LaserPointer lp, int x, int y, float pr, float pw){
  lp.xcoord = ((x > XMAX) || (x < XMIN)) ? 1 : x;
  lp.ycoord = ((y > YMAX) || (y < YMIN)) ? 1 : y;
  lp.prr = (pr < 0) ? 0 : pr;
  lp.pwm = ((pw > 1) || (pw < 0)) ? 0 : pw;
  printf("\nINITIAL LASER VALUES: \n"); // DBPRINT
  printf("%i : %i : %0.3f : %0.3f\n",lp.xcoord,
           lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}

struct Target copyTarget(struct Target tar, int x, int y, float dist, char disp, int pel, char *targetID){
  tar.xloc = ((x > XMAX) || (x < XMIN)) ? 1 : x;
  tar.yloc = ((y > YMAX) || (y < YMIN)) ? 1 : y;
  tar.dist = ((dist > 100) || (dist < 0)) ? 0.1 : dist;
  tar.disp = ((disp != 'S')&&(disp != 'B')&&(disp != 'A')&&(disp != 'F')) ? 'U' : disp;
  tar.pel = pel < 0 ? 0 : pel;
  tar.targetID = targetID;
  printf("\nNEW TARGET VALUES: \n"); // DBPRINT
  printf("%i : %i : %0.3f : %c : %i : %s\n",tar.xloc,
           tar.yloc,tar.dist,tar.disp,tar.pel,tar.targetID); //DBPRINT
  return tar;
}

////======= Getters =======////
 int getXLP(struct LaserPointer lp){
   return lp.xcoord;
 }
 int getYLP(struct LaserPointer lp){
   return lp.ycoord;
 }
 float getPRRLP(struct LaserPointer lp){
   return lp.prr;
 }
 float getPWMLP(struct LaserPointer lp){
   return lp.pwm;
 }

 int getXTar(struct Target t){
   return t.xloc;
 }
 int getYTar(struct Target t){
   return t.yloc;
 }
 float getDistTar(struct Target t){
   return t.dist;
 }
 char getDispTar(struct Target t){
   return t.disp;
 }
 int getPelTar(struct Target t){
   return t.pel;
 }
 int getAmblTar(struct Target t){
   return t.ambLight;
 }
 char * getTarID(struct Target t){
   return t.targetID;
 }

//======= Laser Pointer Control Methods =======//

struct LaserPointer *armLaser(struct LaserPointer *lp){
  centerLaser(lp);
  // GLOBAL_BLANKER_PRR = 0.0;  // These 4 lines store the laser levels in global
  // GLOBAL_BLANKER_PWM = 0.0; // vars before blanking so they can be retrieved
  GLOBAL_BLANKER_PRR = lp->prr; // later by the unBlankLaser command.
  GLOBAL_BLANKER_PWM = lp->pwm; //
  lp->prr = 0;
  lp->pwm = 0;
  printf("Arming LASER: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp->xcoord,
            lp->ycoord,lp->prr,lp->pwm); //DBPRINT
  return lp;
}

struct LaserPointer armLaser_byval(struct LaserPointer lp){
  // centerLaser_byval(lp);
  // GLOBAL_BLANKER_PRR = 0.0;  // These 4 lines store the laser levels in global
  // GLOBAL_BLANKER_PWM = 0.0; // vars before blanking so they can be retrieved
  GLOBAL_BLANKER_PRR = lp.prr; // later by the unBlankLaser command.
  GLOBAL_BLANKER_PWM = lp.pwm; //
  lp.prr = 0;
  lp.pwm = 0;
  printf("Arming LASER by value: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp.xcoord,
            lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}

void testOut(void){
  printf("Test My GUI\n");
}

struct LaserPointer *darmLaser(struct LaserPointer *lp){
  // blankLaser(lp);
  centerLaser(lp);
  // GLOBAL_BLANKER_PRR = 0.0;  // These 4 lines store the laser levels in global
  // GLOBAL_BLANKER_PWM = 0.0; // vars before blanking so they can be retrieved
  GLOBAL_BLANKER_PRR = lp->prr; // later by the unBlankLaser command.
  GLOBAL_BLANKER_PWM = lp->pwm; //
  lp->prr = 0;
  lp->pwm = 0;
  printf("Disarming LASER: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp->xcoord,
            lp->ycoord,lp->prr,lp->pwm); //DBPRINT
  return lp;
}

struct LaserPointer darmLaser_byval(struct LaserPointer lp){
  // centerLaser_byval(lp);
  // GLOBAL_BLANKER_PRR = 0.0;  // These 4 lines store the laser levels in global
  // GLOBAL_BLANKER_PWM = 0.0; // vars before blanking so they can be retrieved
  GLOBAL_BLANKER_PRR = lp.prr; // later by the unBlankLaser command.
  GLOBAL_BLANKER_PWM = lp.pwm; //
  lp.prr = 0;
  lp.pwm = 0;
  printf("Disarming LASER by value: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp.xcoord,
            lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}

struct LaserPointer *centerLaser(struct LaserPointer *lp){
  lp->xcoord = XMAX/2.0;
  lp->ycoord = YMAX/2.0;
  lp->prr = 1.0;
  lp->pwm = 0.25;
  // printf("CENTERING LASER AT: "); // DBPRINT
  // printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp->xcoord,
  //           lp->ycoord,lp->prr,lp->pwm); //DBPRINT
  return lp;
}

struct LaserPointer centerLaser_byval(struct LaserPointer lp){
  lp.xcoord = XMAX/2.0;
  lp.ycoord = YMAX/2.0;
  lp.prr = 1.0;
  lp.pwm = 0.25;
  printf("CENTERING LASER by value AT: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp.xcoord,
            lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}

// This function stores the laser levels in global vars before blanking
// so they can be retrieved later by the unBlankLaser command.
struct LaserPointer *blankLaser(struct LaserPointer *lp){
  // GLOBAL_BLANKER_PRR = 0.0;
  // GLOBAL_BLANKER_PWM = 0.0;
  GLOBAL_BLANKER_PRR = lp->prr;
  GLOBAL_BLANKER_PWM = lp->pwm;
  lp->prr = 0;
  lp->pwm = 0;
  printf("BLANKING LASER AT: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp->xcoord,
            lp->ycoord,lp->prr,lp->pwm); // DBPRINT
  return lp;
}

struct LaserPointer blankLaser_byval(struct LaserPointer lp){
  // GLOBAL_BLANKER_PRR = 0.0;  // These 4 lines store the laser levels in global
  // GLOBAL_BLANKER_PWM = 0.0; // vars before blanking so they can be retrieved
  GLOBAL_BLANKER_PRR = lp.prr; // later by the unBlankLaser command.
  GLOBAL_BLANKER_PWM = lp.pwm; //
  lp.prr = 0;
  lp.pwm = 0;
  printf("BLANKING LASER by value: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp.xcoord,lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}

struct LaserPointer *unBlankLaser(struct LaserPointer *lp){
  lp->prr = GLOBAL_BLANKER_PRR;
  lp->pwm = GLOBAL_BLANKER_PWM;
  printf("UN-BLANKING LASER AT: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp->xcoord,
            lp->ycoord,lp->prr,lp->pwm); // DBPRINT
  return lp;
}

struct LaserPointer unBlankLaser_byval(struct LaserPointer lp){
  lp.prr = GLOBAL_BLANKER_PRR;
  lp.pwm = GLOBAL_BLANKER_PWM;
  printf("UNBLANKING LASER by value: "); // DBPRINT
  printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp.xcoord,
            lp.ycoord,lp.prr,lp.pwm); //DBPRINT
  return lp;
}

struct LaserPointer *moveLaserToTarget(struct Target *target, struct LaserPointer *lp){
  lp->xcoord      = target->xloc;
  lp->ycoord      = target->yloc;
  printf("MOVING LASER TO TARGET %s \tat: ",target->targetID); // DBPRINT
  printf("%i X : %i Y : %0.2f distance (m) \n",lp->xcoord,
            lp->ycoord,target->dist); // DBPRINT
  return lp;
 }

 struct LaserPointer moveLaserToTarget_byval(struct Target target, struct LaserPointer lp){
   lp.xcoord      = target.xloc;
   lp.ycoord      = target.yloc;
   printf("MOVING LASER TO TARGET %s by value \tat: ",target.targetID); // DBPRINT
   printf("%i X : %i Y : %0.3f prr : %0.2f pwm\n",lp.xcoord,
             lp.ycoord,lp.prr,lp.pwm); //DBPRINT
   return lp;
  }

/////////////////////////////////////
//////// Construction Zone //////////

//////////////////////////////////////////////////////////////////

/// NOTE: Need to add blanking protocol per Target PEL settings
struct LaserPointer *scanTargetBox(struct Target *target, struct LaserPointer *lp, int msec){
  lp = moveLaserToTarget(target,lp);
  // lp = laserTrackTarget(target,lp);
  int x0 = lp->xcoord - getScanWidth(target,XMAX,3.0,CAMERA_VIEW_ANGLE)*0.5;
  int y0 = lp->ycoord + getScanWidth(target,YMAX,1.5,CAMERA_VIEW_ANGLE)*0.5;
  int xf = lp->xcoord + getScanWidth(target,XMAX,3.0,CAMERA_VIEW_ANGLE)*0.5;
  int yf = lp->ycoord - getScanWidth(target,YMAX,1.5,CAMERA_VIEW_ANGLE)*0.5;

  x0 = x0 < XMIN ? XMIN : x0;
  y0 = y0 < YMIN ? YMIN : y0;
  xf = xf > XMAX ? XMAX : xf;
  yf = yf > YMAX ? YMAX : yf;

  printf("x0: %i\n", x0); // DBPRINT
  printf("y0: %i\n", y0); // DBPRINT
  printf("xf: %i\n", xf); // DBPRINT
  printf("yf: %i\n", yf); // DBPRINT

  // int count = 0;
  clock_t start_time = clock();
  while(clock() < start_time + msec){
     lp->xcoord = x0;
     lp->ycoord = y0;
     for(lp->ycoord=y0;lp->ycoord<yf;lp->ycoord++){
        // unBlankLaser(lp);
        delay(10);
        for(lp->xcoord=x0;lp->xcoord<xf;lp->xcoord++){
          delay(10);
        }
        // blankLaser(lp);
     }
  }
  // moveLaserToTarget(target,lp);
  // blankLaser(lp);
  return lp;
}

//// cameramax = width of camera screen in pixels, defined as XMAX or YMAX for now
//// scanWidth = width or height in meters of square beam we are projecting across the target
//// betadeg = the camera's viewing angle in degrees; I am assuming 110 degrees for now
int getScanWidth(struct Target *target, float cameramax, float scanWidth, double betadeg){
  // double betaradians = betadeg * (PI/180.0);
  int base = (2*(cameramax*scanWidth*tan(betadeg))/(target->dist));
  printf("scan size: \t%i pixels\n",base); // DBPRINT
  return base;
}

/// NOTE: Need to add blanking protocol per Target PEL settings
/// This needs to be as atomic as possible for mutiple target tracking!!
struct LaserPointer *laserReactToTarget(struct Target *target, struct LaserPointer *lp){
  printf("Reacting to Target\n");
  lp = moveLaserToTarget(target,lp);
  char td = target->disp;
  switch (td) {
   case 'U':
      printf("Case Unspecified\n"); // DBPRINT
      lp->pwm = 0.07;
      scanTargetBox(target,lp,100);
      printxyz(lp); // DBPRINT
      printtarget(target); // DBPRINT
      break;
   case 'S':
      printf("Case Startled\n"); // DBPRINT
      lp->pwm = 0.50;
      scanTargetBox(target,lp,100);
      printxyz(lp); // DBPRINT
      printtarget(target); // DBPRINT
      break;
   case 'B':
      printf("Case Blinded\n"); // DBPRINT
      lp->pwm = 0.30;
      scanTargetBox(target,lp,100);
      printxyz(lp); // DBPRINT
      printtarget(target); // DBPRINT
      break;
   case 'A':
      printf("Case Aggressive\n"); // DBPRINT
      lp->pwm = 0.95;
      scanTargetBox(target,lp,100);
      printxyz(lp); // DBPRINT
      printtarget(target); // DBPRINT
      break;
   case 'F':
      printf("Case Flee\n"); // DBPRINT
      lp->pwm = 0.10;
      scanTargetBox(target,lp,100);
      printxyz(lp); // DBPRINT
      printtarget(target); // DBPRINT
      break;
   default:
      printf("default\n"); // DBPRINT
      break;
  }
  return lp;
}

/// NOTE: Need to add blanking protocol per Target PEL settings
/// This needs to be as atomic as possible for mutiple target tracking!!
struct LaserPointer laserReactToTarget_byval(struct Target target, struct LaserPointer lp){
  printf("Reacting to Target by value\n");
  lp = moveLaserToTarget_byval(target,lp);
  char td = target.disp;
  switch (td) {
   case 'U':
      printf("Case Unspecified\n"); // DBPRINT
      lp.pwm = 0.07;
      // printxyz_byval(lp); // DBPRINT
      // printtarget_byval(target); // DBPRINT
      break;
   case 'S':
      printf("Case Startled\n"); // DBPRINT
      lp.pwm = 0.50;
      // printxyz_byval(lp); // DBPRINT
      // printtarget_byval(target); // DBPRINT
      break;
   case 'B':
      printf("Case Blinded\n"); // DBPRINT
      lp.pwm = 0.30;
      // printxyz_byval(lp); // DBPRINT
      // printtarget_byval(target); // DBPRINT
      break;
   case 'A':
      printf("Case Aggressive\n"); // DBPRINT
      lp.pwm = 0.95;
      // printxyz_byval(lp); // DBPRINT
      // printtarget_byval(target); // DBPRINT
      break;
   case 'F':
      printf("Case Flee\n"); // DBPRINT
      lp.pwm = 0.10;
      // printxyz_byval(lp); // DBPRINT
      // printtarget_byval(target); // DBPRINT
      break;
   default:
      printf("default\n"); // DBPRINT
      break;
  }
  return lp;
}

//////// End of Construction Zone //////////
////////////////////////////////////////////////////

////======= End of Laser Pointer Control Methods =======////
////////////////////////////////////////////////////////////

////======= Target Control Methods =======////

// Generates and prints 'count' random
// numbers in range [lower, upper].
int genRandInRange(int lower, int upper,int count){
    int i, num;
    for (i = 0; i < count; i++) {
      num = (rand() % (upper - lower + 1)) + lower;
      delay(1);
      printf("%d ", num);
    }
    return num;
}

struct Target *centerTarget(struct Target *target){
  target->xloc = XMAX/2.0;
  target->yloc = YMAX/2.0;
  printf("CENTERING TARGET %s   at: \t",target->targetID); // DBPRINT
  printf("%i X : %i Y  %0.2f:\n",target->xloc,
            target->yloc,target->dist); //DBPRINT
  return target;
}

void laserTrackTarget(struct Target *target, struct LaserPointer *lp){
  // int flag = 0;
  // while(flag==0){
    lp->xcoord      = target->xloc;
    lp->ycoord      = target->yloc;
    printf("MV LSR TO TARG %s at: ",target->targetID); // DBPRINT
    printf("%i X : %i Y : %0.2f dist : %c disp\n",lp->xcoord,
              lp->ycoord,target->dist,target->disp); // DBPRINT
  // }
 }

void moveTargetAround(struct Target *target, struct LaserPointer *lp, int msec){
  printf("\n");
  target = centerTarget(target);
  clock_t start_time = clock();
  while(clock() < start_time + msec){
     int xdiff = genRandInRange(-40,70,1);
     int ydiff = genRandInRange(-40,70,1);
     int distdiff = genRandInRange(-5,4,1);
     target->xloc = ((target->xloc+xdiff<XMAX)&&(target->xloc+xdiff>XMIN))?target->xloc+xdiff:XMAX/2;
     target->yloc = ((target->yloc+ydiff<YMAX)&&(target->yloc+ydiff>YMIN))?target->yloc+ydiff:YMAX/2;
     target->dist = ((target->dist+distdiff<25)&&(target->dist+distdiff>1))?target->dist+distdiff:5;
     printf("\t\t%i X : %i Y : %0.2f dist\n",target->xloc,target->yloc,target->dist); //DBPRINT
     // laserTrackTarget(target,lp);
     scanTargetBox(target,lp,10);
     // getScanWidth(target,XMAX,3.0,CAMERA_VIEW_ANGLE)*0.5;
     // getScanWidth(target,YMAX,1.5,CAMERA_VIEW_ANGLE)*0.5;
     printf("\n");
  }
}

/////////////////////////////////////////
////======= All Other Functions =======//

////======= The obligatory and ubiquitous C delay function =======////
// For seconds, uncomment the second line.
void delay(int tim){
  //tim = 1000 * tim;
  clock_t start_time = clock();
  while (clock() < start_time + tim);
}
////======= DBPRINT Methods =======////
void printxyz(struct LaserPointer *lp){
  printf("Laser Coordinates: %i X : %i Y : %0.3f prr : %0.3f pwm\n",lp->xcoord,
           lp->ycoord,lp->prr,lp->pwm); //DBPRINT
}
void printxyz_byval(struct LaserPointer lp){
  printf("Laser Coordinates: %i X : %i Y : %0.3f prr : %0.3f pwm\n",lp.xcoord,
           lp.ycoord,lp.prr,lp.pwm); //DBPRINT
}
void printtarget(struct Target *targ){
  printf("Target Coordinates: %i X : %i Y : %0.3f dist : %c disp : %i PEL : %s TID \n\n",targ->xloc,
           targ->yloc,targ->dist,targ->disp,targ->pel,targ->targetID); //DBPRINT
}
void printtarget_byval(struct Target targ){
  printf("Target Coordinates: %i X : %i Y : %0.3f dist : %c disp : %i PEL\n\n",targ.xloc,
           targ.yloc,targ.dist,targ.disp,targ.pel); //DBPRINT
}


///////////////////////////////////////
//// ======== Test Objects ========////
//// The Square
// struct LaserPointer drawTestSquare(int msec){
//   struct LaserPointer *lp1 = createLaser(61440,61440,1,0.25);
//   delay(8);
//   int count = 0;
//   //int milli_seconds = 1000 * msec;
//   clock_t start_time = clock();
//   printf("Square\n");
//   while(clock() < start_time + msec){
//     // printxyz(lp1);
//     lp1.ycoord = 4095;
//     // printxyz(lp1); // DBPRINT
//     delay(8);
//     lp1.xcoord = 4095;
//     // printxyz(lp1); // DBPRINT
//     delay(8);
//     lp1.ycoord = 61440;
//     // printxyz(lp1); // DBPRINT
//     delay(8);
//     lp1.xcoord = 61440;
//     lp1.ycoord = 61440;
//     // printxyz(lp1); // DBPRINT
//     printf("count: %i\n",count+=1); // DBPRINT
//     printf("\n"); // DBPRINT
//     delay(8);
//   }
//   return lp1;
// }
//// The Ellipse
struct LaserPointer drawEllipse(int msec, double t, double a, double b, double z) {
  double x = a * cos(t);
  double y = b * sin(t);
  struct LaserPointer lp0 = createLaser(x,y,1.0,0.25);
  int count = 0;
  // centerLaser(lp0);
  clock_t start_time = clock();
  printf("Ellipse\n");
  while(clock() < start_time + msec){
    double theta = (atan(lp0.ycoord/lp0.xcoord));
    lp0.xcoord = a * cos(t);
    lp0.ycoord = b * sin(t);
    double r = sqrt((lp0.xcoord*lp0.xcoord)+(lp0.ycoord*lp0.ycoord));
    count += 1;
    printf("%i : %i\ttheta: %lf\tr: %lf\tcount: %i\n",lp0.xcoord,lp0.ycoord,theta,r,count);
    // printf("%lf : %lf : %lf\ttheta: %lf\tr: %lf\n",lp0.xcoord,lp0.ycoord,z,theta,r);
    t += 0.1;
  }
  return lp0;
}
//// The Cardioid
struct LaserPointer drawCardioid(double msec, double a, double theta, double zc) {
  // double theta = thetaDegrees * PI/180.0; // Converts input degrees to radians
  double r = a * (1-cos(theta));
  double x = a * cos(theta) * (1-cos(theta));
  double y = a * sin(theta) * (1-cos(theta));
  struct LaserPointer lp0 = createLaser(x,y,1.0,0.25);
  double z = zc;
  int count = 0;
  // centerLaser(lp0);
  clock_t start_time = clock();
  printf("Cardioid\n");
  while(clock() < start_time + msec){
    r = a * (1-cos(theta));
    lp0.xcoord = a * cos(theta) * (1-cos(theta));
    lp0.ycoord = a * sin(theta) * (1-cos(theta));
    // double theta = atan(y/x); // Convert theta to radians
    double angle = (atan(lp0.ycoord/lp0.xcoord)) * (180.0/PI); // Convert theta to degrees
    count += 1;
    printf("%i : %i\ttheta: %lf\tr: %lf\tcount: %i\n",lp0.xcoord,lp0.ycoord,angle,r,count);
    printf("%lf : %lf : %lf\ttheta: %lf\tr: %lf\n",x,y,z,theta,r);
    theta += 0.1;
  }
  return lp0;
}
// Cardioid Equations
// r = a(1 - cos(θ))
// x = a cos(θ)(1-cos(θ))
// y = a sin(θ)(1-cos(θ))
////========= End of Test Objects ==========////
////========================================////
////////////////////////////////////////
////============= Main =============////

int main()
{
    struct LaserPointer lp1 = createLaser(4103,65073,12.5,0.75);

    struct Target t1 = createTarget(1073,10073,5.0,'A', 14, 1500,"ABC");
    struct Target t4 = createTarget(6001,22073,10.0,'B', 120, 24001,"3210-boom");
    struct Target t7 = createTarget(67000,-1073,120.0,'S', 100, 7500,"alpha-2.3");struct Node* head = NULL;
    push(&head, t1);
    push(&head, t7);
    push(&head, t4);
    printf("Element at index 0 is ");
    getNthTarget(head, 0);
    printList(head);
    //// Check the count function
    checkPEL (head,99);

    // struct LaserPointer lp1 = createLaser(4103,65073,12.5,0.75);
    // struct LaserPointer lp7 = createLaser(67000,-1073,1,0.25);
    // centerLaser(lp7);

    // struct Target t1 = createTarget(1073,10073,5.0,'A', 142, 1500,"ABC");
    // struct Target t4 = createTarget(6001,22073,10.0,'B', 10, 24001,"3210-boom");
    // struct Target t7 = createTarget(67000,-1073,120.0,'S', 100, 7500,"alpha-2.3");
    // moveTargetAround(&t1, &lp1, 200);
    // moveTargetAround(&t4, &lp1, 200);
    // getScanWidth(t1,XMAX,30,);
    // moveLaserToTarget(&t1,&lp1);

    /////////////////////////////////
    // struct Node* head = NULL;
    //
    // push(&head, t1);
    // push(&head, t7);
    // push(&head, t4);
    // // Check the count function
    // printf("Element at index 0 is ");
    // getNthTarget(head, 0);
    // printList(head);
    // checkPEL (head,99);
    // moveLaserToTarget(&t1,&lp1);
    moveTargetAround(&t1, &lp1, 480);
    // moveTargetAround(&t4, &lp1, 240);
    // moveTargetAround(&t7, &lp1, 240);
    // deleteList(&head);

    //scanTargetBox(&t1,&lp1,10);
    // scanTargetBox(&t7,&lp1,10);
    // scanTargetBox(&t4,&lp1,10);
    ///////////////////////////////

    // moveLaserToTarget(&t1,&lp1);
    // struct Dispatcher dsp1 = createDispatcher("ABC");

    // copyLP(lp1,5000,5000,10.5,0.1);
    // copyTarget(t1,45000,43000,3.7,'A',150,"helium");
    // laserReactToTarget(&t1,&lp1);
    // blankLaser(&lp1);
    // unBlankLaser(&lp1);
    // armLaser(&lp1);
    // darmLaser(&lp1);
    return 0;
 }

 /// GTK sample code ///
 // int main( int  argc,
 //        char *argv[] )
 //   {
 //     GtkWidget *window;
 //     gtk_init (&argc, &argv);
 //     window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
 //     gtk_widget_show (window);
 //     gtk_main ();
 //     return 0;
 //   }
 // https://askubuntu.com/questions/397432/fatal-error-gtk-gtk-h-no-such-file-or-directory-using-make
 // https://www.quora.com/How-do-I-create-a-beautiful-GUI-in-C-Linux
 // gcc myfile.c -o myfile `pkg-config --cflags --libs gtk+-2.0`
 ///////////////////////


 // drawTestSquare(500);
 // drawEllipse(700,0,1,2,0.25);
 // drawCardioid(1200,2,1,2.0);
 //
 // #include <phidget22.h>
 // #include <stdio.h>
