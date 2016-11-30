/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>

#define GREEN_LED BIT6

char p1[] = "Player1:0";
char p2[] = "Player2:0";


AbRect testRect = {abRectGetBounds, abRectCheck, {3, 12}}; /* 5x10 rectangle */
AbRect testRect1 = {abRectGetBounds, abRectCheck, {3, 12}};
AbRect centerField = {abRectGetBounds, abRectCheck, {1, (screenHeight/2)-2}}; 

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {(screenWidth/2)-2, (screenHeight/2)-2}
};

Layer ballLayer = {
  (AbShape *)&circle8,
  {(screenWidth/2), (screenHeight/2)},
  {0,0}, {0,0},
  COLOR_WHITE,
  0
};

Layer netLayer = {
  (AbShape *) &centerField,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  &ballLayer,
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &netLayer,
};

Layer paddle2 = {
  (AbShape *)&testRect1,
  {((screenWidth*3)/4)+20, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  &fieldLayer,
};

Layer paddle1 = {
  (AbShape *)&testRect,
  {(screenWidth/4)-20, screenHeight/2},
  {0,0}, {0,0},
  COLOR_WHITE,
  &paddle2,
  };

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml3 = { &ballLayer, {-1,2}, 0 }; /**< not all layers move */
MovLayer ml1 = { &paddle2, {0,2}, &ml3 }; 
MovLayer ml0 = { &paddle1, {0,0}, &ml1 }; 




movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, Region *fence)
{
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	newPos.axes[axis] += (2*velocity);
      }	/**< if outside of fence */
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}


void collision1(){

  if((ballLayer.pos.axes[0]-(circle8.radius) <= (paddle1.pos.axes[0] + 1))
     && (ballLayer.pos.axes[1] >= paddle1.pos.axes[1] - 12)
     && (ballLayer.pos.axes[1] <= paddle1.pos.axes[1] + 12)){

    ballLayer.posNext.axes[0] += 3;
    ml3.velocity.axes[0] = -ml3.velocity.axes[0];
  }
}

void collision2(){

  if((ballLayer.pos.axes[0]+(circle8.radius) >= (paddle2.pos.axes[0] - 3))
     && (ballLayer.pos.axes[1] >= paddle2.pos.axes[1] - 12)
      && (ballLayer.pos.axes[1] <= paddle2.pos.axes[1] + 12)){

    ballLayer.posNext.axes[0] -= 3;
    ml3.velocity.axes[0] = -ml3.velocity.axes[0];
  }

}

/* Received help on this method from Raul Hinostroza */
void player1Switches(u_int button){

 
  
  if(!(button & (1<<0)))
    ml0.velocity.axes[1] = 4;
  else if(!(button & (1<<1)))
    ml0.velocity.axes[1] = -4;
  else
    ml0.velocity.axes[1] = 0;
  
}
/* Received help on this method from Raul Hinostroza */
void player2Switches(u_int button){


  if(!(button & (1<<2)))
    ml1.velocity.axes[1] = 4;
  else if((button & (1<<3)))
    ml1.velocity.axes[1] = -4;
  else
    ml1.velocity.axes[1] = 0;

}

void changeScore(){

  if((ballLayer.pos.axes[0] - circle8.radius) < 4){
    if(p2[8]<'8'){
      ballLayer.posNext.axes[0] = (screenWidth/2);
      ballLayer.posNext.axes[1] = (screenHeight/2);
      p2[8] = p2[8] + 1;
    }
    else{
      p2[8] = p2[8] + 1;
      ballLayer.posNext.axes[0] = (screenWidth/2);
      ballLayer.posNext.axes[1] = (screenHeight/2);
      ml3.velocity.axes[0] = ml3.velocity.axes[1] = 0;
      drawString5x7((screenWidth/2)+7,14, "Winner", COLOR_YELLOW, COLOR_BLACK);
    }
  }
   else if((ballLayer.pos.axes[0] + circle8.radius) > (screenWidth - 4)){
    if(p1[8]<'8'){
      ballLayer.posNext.axes[0] = (screenWidth/2);
      ballLayer.posNext.axes[1] = (screenHeight/2);
      p1[8] = p1[8] + 1;
    }
    else{
      p1[8] = p1[8] + 1;
      ballLayer.posNext.axes[0] = (screenWidth/2);
      ballLayer.posNext.axes[1] = (screenHeight/2);
      ml3.velocity.axes[0] = ml3.velocity.axes[1] = 0;
      drawString5x7(7,14, "Winner", COLOR_ORANGE, COLOR_BLACK);
    }
  }
  
}

u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */

/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(15);

  shapeInit();

  layerInit(&paddle1);
  layerDraw(&paddle1);

  
  layerGetBounds(&fieldLayer, &fieldFence);
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  u_int button;
  for(;;) {

    button = p2sw_read();
    if (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
      
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;

    player1Switches(button);
    //player2Switches(button);
    collision1();    
    collision2();
    changeScore();
    
    drawString5x7(5,5, p1, COLOR_WHITE, COLOR_BLACK);
    drawString5x7((screenWidth/2)+6, 5, p2, COLOR_WHITE, COLOR_BLACK);
    
    movLayerDraw(&ml0, &paddle1);
  }
}



/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;/**< Green LED on when cpu on */
  count ++;
  if (count >= 15) {
    mlAdvance(&ml0, &fieldFence);
    if (p2sw_read()){
      redrawScreen = 1;
    }
    
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
