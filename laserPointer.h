
//===========================================================
//
//  File Name:           laserPointer.h
//  Project Name:        LP_Jetson
//  Project Owner:       Creative Engineering Solutions, LLC
//  Project Engineer:    Jacob M. Romero
//                       Creative Engineering Solutions, LLC
//                       jrom876@gmail.com
//  Github Repo:         https://github.com/jrom876/laser_galvanometer

/**
	Copyright (C) 2019, 2021 
	Jacob Romero, Creative Engineering Solutions, LLC
	cesllc876@gmail.com

	This program is free software; you can redistribute it
	and/or modify it under the terms of the GNU General Public  
	License as published by the Free Software Foundation, version 2.

	This program is distributed in the hope that it will be
	useful, but WITHOUT ANY WARRANTY; without even the implied 
	warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	
	See the GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public
	License along with this program; if not, write to:
	The Free Software Foundation, Inc.
	59 Temple Place, Suite 330
	Boston, MA 02111-1307 USA

**/

//===========================================================

#include <string.h>
#include <stdio.h>
#include <check.h>
#ifndef laserPointerH
#define laserPointerH

#define XMAX 0xFFFF
#define YMAX 0xFFFF
#define XMIN 1
#define YMIN 1
#define PEL_LIMIT 1000
#define MAX_ITEMS 3

// #define XMAX_DYN(x,cm,sw,b,d) x+(2*(cm*sw*tan(b))/(d))
// #define YMAX_DYN(y,cm,sw,b,d) y+(2*(cm*sw*tan(b))/(d))
// #define XMIN_DYN(x,cm,sw,b,d) x-(2*(cm*sw*tan(b))/(d))
// #define YMIN_DYN(y,cm,sw,b,d) y-(2*(cm*sw*tan(b))/(d))

#define X_TAN_BETA(degrees) tan(float(degrees*(PI/180.0)))
#define Y_TAN_BETA(degrees) tan(float(degrees*(PI/180.0)))
#define CAMERA_VIEW_ANGLE 110
#define MICROPROCESSOR "/dev/ttyUSB0"
#define PI 3.14159265359

//// Flash Pattern Definitions
// #define STARTLE         startle
// #define GLARE           glare
// #define DANCING_BEAM    dancing_beam
// #define FLASH_BLINDNESS flash_blindness
// #define STROBE          strobe

struct LaserPointer {
  int     xcoord;
  int     ycoord;
  float   prr;      // Pulse Repetition Rate
  float   pwm;      // PWM
};

struct Target {
  int     xloc;
  int     yloc;
  float   dist;
  char    disp; // NOTE: do not make this char a string
  int     pel;
  float   ambLight; // Ambient Light sensor input
  char    *targetID; // The target's unique ID number
};

struct Node {
  struct  Target target;
  struct  Node* next;
};

struct LaserPointer createLaser(int x, int y, float pr, float pw);
struct Target       createTarget(int x, int y, float dist, char dispo, int pel, float ambL, char *targetID);

struct LaserPointer copyLP(struct LaserPointer lp, int x, int y, float pr, float pw);
struct Target       copyTarget(struct Target tar, int x, int y, float dist, char disp, int pel, char *targetID);

void   push(struct Node** head_ref, struct Target new_target);
void   printList(struct Node* n);
struct Target getNthTarget(struct Node* head, int index);
void   deleteList(struct Node** head_ref);
int    checkPEL(struct Node* head, int limit);
// int createTargetList(struct Target t1, struct Target t2, struct Target t3);

struct LaserPointer *armLaser(struct LaserPointer *lp);
struct LaserPointer *darmLaser(struct LaserPointer *lp);
struct LaserPointer *centerLaser(struct LaserPointer *lp);
struct LaserPointer *blankLaser(struct LaserPointer *lp);
struct LaserPointer *unBlankLaser(struct LaserPointer *lp);
struct LaserPointer *moveLaserToTarget(struct Target *target, struct LaserPointer *lp);
struct LaserPointer *laserReactToTarget(struct Target *target, struct LaserPointer *lp1);
struct LaserPointer *scanTargetBox(struct Target *target, struct LaserPointer *lp, int msec);

struct LaserPointer armLaser_byval(struct LaserPointer lp);
struct LaserPointer darmLaser_byval(struct LaserPointer lp);
struct LaserPointer centerLaser_byval(struct LaserPointer lp);
struct LaserPointer blankLaser_byval(struct LaserPointer lp);
struct LaserPointer unBlankLaser_byval(struct LaserPointer lp);
struct LaserPointer moveLaserToTarget_byval(struct Target target, struct LaserPointer lp);
struct LaserPointer laserReactToTarget_byval(struct Target target, struct LaserPointer lp);
int    getScanWidth(struct Target *target, float cameramax, float scanWidth, double betadeg);

int   getXLP(struct LaserPointer lp);
int   getYLP(struct LaserPointer lp);
float getPRRLP(struct LaserPointer lp);
float getPWMLP(struct LaserPointer lp);
int   getXTar(struct Target t);
int   getYTar(struct Target t);
float getDistTar(struct Target t);
char  getDispoTar(struct Target t);
int   getPelTar(struct Target t);
int   getAmblTar(struct Target t);
char * getIDTar(struct Target t);

void moveTargetAround(struct Target *target, struct LaserPointer *lp, int msec);
void laserTrackTarget(struct Target *target, struct LaserPointer *lp);
struct Target *centerTarget(struct Target *target);
int genRandInRange(int lower, int upper,int count);

int   targetsDetected(int flag);
void  clearTargetsDetected();

void  delay(int msec);
void  printtarget(struct Target *targ);
void  printtarget_byval(struct Target targ);
void  printxyz(struct LaserPointer *lp);
void  printxyz_byval(struct LaserPointer lp);

//// Shape Tracers ////
struct LaserPointer drawTestSquare(int msec);
struct LaserPointer drawCardioid(double msec, double a, double theta, double zc);
struct LaserPointer drawEllipse(int msec, double t, double a, double b, double z);

#endif
