#include "fix_fft.h"

#define REDPIN 39
#define GREENPIN 38
#define BLUEPIN 19
#define WHITEPIN 18
#define FADESPEED 2
int real[nPts];
int imag[nPts];
int sampleInterval;            
int r;
int g;
int b;
int w;
int rAmp;
int gAmp;
int bAmp;
int wAmp;
int IdleCounter = 0;

float prevHz;

float maxHz = 0;
float currentHz = 0;

bool rFlag;
bool gFlag;
bool bFlag;
bool wFlag;
bool rgFlag;
bool rbFlag;
bool rwFlag;
bool gbFlag;
bool gwFlag;
bool bwFlag;
bool rgbwFlag;


float fadeRatio = 0.001;

void setup(){
  Serial.begin(9600);
  delay(500);                   //give time for serial monitor to start up in Energia
  analogReadResolution(ANALOG_RESOLUTION);
        
  //****************** interval calculation *******************************  
  int unCorrectedSampleInterval = 500000/hiFreq;
  long startTime = micros();
  for(int i = 0; i < nPts; i++){             // determine total actual time for uncorrected interval
    real[i] = analogRead(ANALOG_IN);
    delayMicroseconds(unCorrectedSampleInterval);    // unadjusted sample interval
  }
  long endTime = micros();
  int totalTime = (int)(endTime - startTime);
  int expectedTime = nPts * unCorrectedSampleInterval;
  int errorTime = totalTime - expectedTime;
  sampleInterval = unCorrectedSampleInterval - errorTime/nPts;
  if (DEBUG) {
    Serial.println ("\nYou are now entering the frequency domain...\n");
    Serial.println ("Microcontroller   : MSP-EXP432401R");   
    Serial.print   ("Sample size       : ");
    Serial.println (nPts);
    Serial.print   ("Highest frequency : ");
    Serial.print   (hiFreq);
    Serial.println (" Hz");
    Serial.print   ("Resolution        : ");
    Serial.print   (FREQ_RESOLUTION);
    Serial.println (" Hz"); 
    Serial.print   ("\nunCorrectedSampleInterval = ");
    Serial.print   (unCorrectedSampleInterval);
    Serial.println (" micros");
    Serial.print   ("totalTime = ");
    Serial.print   (totalTime);
    Serial.println (" micros");    
    Serial.print   ("expectedTime = ");
    Serial.print   (expectedTime);
    Serial.println (" micros");
    Serial.print   ("errorTime = ");
    Serial.print   (errorTime);
    Serial.println (" micros");
    Serial.print   ("sampleInterval = ");
    Serial.print   (sampleInterval);
    Serial.println (" micros");
  }
  
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(WHITEPIN, OUTPUT);
}

void loop(){
  int i;
  int x;
  for (i=0; i<nPts; i++) {                   // read ADC pin nPts times at hiFreq kHz
    real[i] = analogRead(ANALOG_IN);
    delayMicroseconds(sampleInterval);       // adjusted sample interval
  }
  
  for( i=0; i<nPts; i++) imag[i] = 0;        // clear imaginary array
  
  fix_fft(real, imag, LOG2N, 0);             // perform fft on sampled points in real[i]
  
  for ( i = 0; i < nPts/2; i++)              //get the power magnitude in each bin
    {
      real[i] =sqrt((long)real[i] * (long)real[i] + (long)imag[i] * (long)imag[i]);        
    }
  // find the peak
  int peakHz = 0;
  int peaki = 0;
  float peakAmp = 0;
  for (i = 1; i < nPts/2; i++) {          // bin 0 holds the summation - not peak
    if (real[i] > peakHz) {
      peakHz = real[i];
      peakAmp = peakHz;
      peaki = i;
    }
  }
  peakHz = (peaki * FREQ_RESOLUTION);

  //If nothing is playing for a long period of time, reset scaling frequency
  if(IdleCounter == 75){
	  maxHz = 0;
	  IdleCounter = 0;
  }

  if(peakAmp > 1000){
    Serial.print("Peak frequency: ");
    Serial.print(peakHz);
    Serial.print("    ");
    Serial.print("Amplitude: ");
    Serial.print(peakAmp);
    Serial.print ("	  ");

    if(peakHz > maxHz){
  	  maxHz = peakHz;
    }

    currentHz = peakHz;
    float scaledHz = currentHz/maxHz;
    Serial.print ("Current frequency: ");
    Serial.println (scaledHz);
  }

  /*****************/
  /*Drum Set Colors*/ //Using colors between red to yellow
  /*****************/
  if(peakAmp > 1000){
	  if(peakAmp > 3000){
		  peakAmp = 3000;
	  }

	  //if snare is heard
	  if(currentHz == 60 && prevHz != 60){
		  analogWrite(WHITEPIN, 255);
		  delay(100);
	  }
	  //red
	  if((currentHz/maxHz) >= 0 && (currentHz/maxHz) < 0.0833){
		r = peakAmp * 0.085;
		g = 0;
		b = 0;
		w = 0;
		analogWrite(WHITEPIN, w);
		analogWrite(GREENPIN, g);
		analogWrite(BLUEPIN, b);
		analogWrite(REDPIN, r);
		rAmp = peakAmp;
		rFlag = true;
	  }
	  //orange
	  else if((currentHz/maxHz) >= 0.0833 && (currentHz/maxHz) < 0.167){
		r = peakAmp * 0.085;
		g = (peakAmp * 0.085)/8;
		b = 0;
		w = 0;
		analogWrite(BLUEPIN, b);
		analogWrite(WHITEPIN, w);
		analogWrite(REDPIN, r);
		analogWrite(GREENPIN, g);
		rAmp = peakAmp;
		gAmp = peakAmp;
		rgFlag = true;
	  }
	  //yelllow
	  else if((currentHz/maxHz) >= 0.167 && (currentHz/maxHz) < 0.25){
	  		r = peakAmp * 0.085;
	  		g = peakAmp * 0.085/4;
	  		b = 0;
	  		w = 0;
	  		analogWrite(BLUEPIN, b);
	  		analogWrite(WHITEPIN, w);
			analogWrite(GREENPIN, g);
			analogWrite(REDPIN, r);
	  		rAmp = peakAmp;
	  		gAmp = peakAmp;
	  		rgFlag = true;
	  	  }
	  //green
	  else if((currentHz/maxHz) >= 0.25 && (currentHz/maxHz) < 0.333){
	  		r = (peakAmp * 0.085);
	  		g = (peakAmp * 0.085);
	  		b = 0;
	  		w = 0;
	  		analogWrite(BLUEPIN, b);
	  		analogWrite(WHITEPIN, w);
			analogWrite(GREENPIN, g);
			analogWrite(REDPIN, r);
	  		rAmp = peakAmp;
	  		gAmp = peakAmp;
	  		rgFlag = true;
	  	  }
	  //green
	  else if((currentHz/maxHz) >= 0.333 && (currentHz/maxHz) < 0.42){
	  		r = 0;
	  		g = (peakAmp * 0.085);
	  		b = 0;
	  		w = 0;
	  		analogWrite(REDPIN, r);
	  		analogWrite(BLUEPIN, b);
	  		analogWrite(WHITEPIN, w);
			analogWrite(GREENPIN, g);
	  		gAmp = peakAmp;
	  		gFlag = true;
	  	  }
	  //green - blue
	  else if((currentHz/maxHz) >= 0.42 && (currentHz/maxHz) < 0.5){
	  		r = 0;
	  		g = (peakAmp * 0.085);
	  		b = (peakAmp * 0.085);
	  		w = 0;
	  		analogWrite(REDPIN, r);
	  		analogWrite(WHITEPIN, w);
			analogWrite(GREENPIN, g);
			analogWrite(BLUEPIN, b);
	  		gAmp = peakAmp;
	  		bAmp = peakAmp;
	  		gbFlag = true;
	  	  }
	  //dark blue
	  else if((currentHz/maxHz) >= 0.5 && (currentHz/maxHz) < 0.583){
	  		r = 0;
	  		g = (peakAmp * 0.085)/2;
	  		b = (peakAmp * 0.085);
	  		w = 0;
	  		analogWrite(REDPIN, r);
	  		analogWrite(WHITEPIN, w);
			analogWrite(BLUEPIN, b);
			analogWrite(GREENPIN, g);
	  		gAmp = peakAmp;
	  		bAmp = peakAmp;
	  		gbFlag = true;
	  	  }
	  //light purple
	  else if((currentHz/maxHz) >= 0.583 && (currentHz/maxHz) < 0.67){
	  		r = (peakAmp * 0.085)/4;
	  		g = 0;
	  		b = peakAmp * 0.085;
	  		w = 0;
  			analogWrite(GREENPIN, g);
	  		analogWrite(WHITEPIN, w);
			analogWrite(REDPIN, r);
			analogWrite(BLUEPIN, b);
	  		rAmp = peakAmp;
	  		bAmp = peakAmp;
	  		rbFlag = true;
	  	  }
	  //purple
	  else if((currentHz/maxHz) >= 0.67 && (currentHz/maxHz) < 0.75){
	  		r = (peakAmp * 0.085)/2;
	  		g = 0;
	  		b = peakAmp * 0.085;
	  		w = 0;
  			analogWrite(GREENPIN, g);
	  		analogWrite(WHITEPIN, w);
			analogWrite(REDPIN, r);
			analogWrite(BLUEPIN, b);
	  		rAmp = peakAmp;
	  		bAmp = peakAmp;
	  		rbFlag = true;
	  	  }
	  //purple - pink
	  else if((currentHz/maxHz) >= 0.75 && (currentHz/maxHz) < 0.83){
	  		r = peakAmp * 0.085;
	  		g = 0;
	  		b = (peakAmp * 0.085)/2;
	  		w = 0;
  			analogWrite(GREENPIN, g);
	  		analogWrite(WHITEPIN, w);
			analogWrite(REDPIN, r);
			analogWrite(BLUEPIN, b);
	  		rAmp = peakAmp;
	  		bAmp = peakAmp;
	  		rbFlag = true;
	  	  }
	  //pink
	  else if((currentHz/maxHz) >= 0.833 && (currentHz/maxHz) < 0.92){
	  		r = peakAmp * 0.085;
	  		g = 0;
	  		b = (peakAmp * 0.085)/4;
	  		w = 0;
	  		analogWrite(BLUEPIN, b);
	  		analogWrite(WHITEPIN, w);
			analogWrite(REDPIN, r);
			analogWrite(GREENPIN, g);
	  		rAmp = peakAmp;
	  		gAmp = peakAmp;
	  		rbFlag = true;
	  	  }
	  else if((currentHz/maxHz) >= 0.92){
	  		r = peakAmp * 0.085;
	  		g = peakAmp * 0.085;
	  		b = peakAmp * 0.085;
	  		w = peakAmp * 0.085;
			analogWrite(REDPIN, r);
			analogWrite(BLUEPIN, b);
			analogWrite(GREENPIN, g);
			analogWrite(WHITEPIN, w);
	  		rAmp = peakAmp;
	  		gAmp = peakAmp;
	  		bAmp = peakAmp;
	  		wAmp = peakAmp;
	  		rgbwFlag = true;
	  	  }
	  prevHz = currentHz;
  }

  /*********************/
  //Fade the lights out//
  /*********************/
  else{
    //Fading just red light
    if(rFlag){
      for (x = 255; x > 0; x--) {
		  analogWrite(REDPIN, x);
		  delay(rAmp*fadeRatio);
      }
      r = 0;
      analogWrite(REDPIN, 0);
      rFlag = false;
    }
    //Fading just green light
    else if(gFlag){
      for (x = 255; x > 0; x--) {
		  analogWrite(GREENPIN, x);
		  delay(gAmp*fadeRatio);
      }
      g = 0;
      analogWrite(GREENPIN, 0);
      gFlag = false;
    }
    //Fading just blue light
    else if(bFlag){
      for (x = 255; x > 0; x--) {
      analogWrite(BLUEPIN, x);
      delay(bAmp*fadeRatio);
      }
      b = 0;
      analogWrite(BLUEPIN, 0);
      bFlag = false;
    }
   //Fading just white light 
    else if(wFlag){
      for (x = 255; x > 0; x--) {
      analogWrite(WHITEPIN, x);
      delay(wAmp*fadeRatio);
      }
      w = 0;
      analogWrite(WHITEPIN, 0);
      wFlag = false;
    }
    //Fading Red & Green light at same time
    else if(rgFlag){
      if(r <= g){
        for (x = r; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(GREENPIN, x);
          delay(rAmp*fadeRatio);
        }
      }
      else{
        for (x = g; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(GREENPIN, x);
          delay(gAmp*fadeRatio);
        }
      }
      r = 0;
      g = 0;
      analogWrite(REDPIN, 0);
      analogWrite(GREENPIN, 0);
      rgFlag = false;
    }
    //Fading Red & Blue light at same time
    else if(rbFlag){
      if(r <= b){
        for (x = r; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(BLUEPIN, x);
          delay(rAmp*fadeRatio);
        }
      }
      else{
        for (x = b; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(BLUEPIN, x);
          delay(bFlag*fadeRatio);
        }
      }
      r = 0;
      b = 0;
      analogWrite(REDPIN, 0);
      analogWrite(BLUEPIN, 0);
      rbFlag = false;
    }
    //Fading Green and Blue light at same time
    else if(gbFlag){
      if(g <= b){
        for (x = g; x > 0; x--) {
          analogWrite(GREENPIN, x);
          analogWrite(BLUEPIN, x);
          delay(gAmp*fadeRatio);
        }
      }
      else{
        for (x = b; x > 0; x--) {
          analogWrite(GREENPIN, x);
          analogWrite(BLUEPIN, x);
          delay(bAmp*fadeRatio);
        }
      }
      g = 0;
      b = 0;
      analogWrite(GREENPIN, 0);
      analogWrite(BLUEPIN, 0);
      gbFlag = false;
    }
    //Fading Red and White COlor
    else if(rwFlag){
      if(r <= w){
        for (x = r; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(WHITEPIN, x);
          delay(rAmp*fadeRatio);
        }
      }
      else{
        for (x = w; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(WHITEPIN, x);
          delay(wAmp*fadeRatio);
        }
      }
      r = 0;
      w = 0;
      analogWrite(REDPIN, 0);
      analogWrite(WHITEPIN, 0);
      rwFlag = false;
    }
    //Fading Green and White Color
    else if(gwFlag){
      if(g <= w){
        for (x = g; x > 0; x--) {
          analogWrite(GREENPIN, x);
          analogWrite(WHITEPIN, x);
          delay(gAmp*fadeRatio);
        }
      }
      else{
        for (x = w; x > 0; x--) {
          analogWrite(GREENPIN, x);
          analogWrite(WHITEPIN, x);
          delay(wAmp*fadeRatio);
        }
      }
      g = 0;
      w = 0;
      analogWrite(GREENPIN, 0);
      analogWrite(WHITEPIN, 0);
      gwFlag = false;
    }
    //Fading Blue and White Color
    else if(bwFlag){
      if(b <= w){
        for (x = b; x > 0; x--) {
          analogWrite(BLUEPIN, x);
          analogWrite(WHITEPIN, x);
          delay(bAmp*fadeRatio);
        }
      }
      else{
        for (x = w; x > 0; x--) {
          analogWrite(BLUEPIN, x);
          analogWrite(WHITEPIN, x);
          delay(wAmp*fadeRatio);
        }
      }
      b = 0;
      w = 0;
      analogWrite(BLUEPIN, 0);
      analogWrite(WHITEPIN, 0);
      bwFlag = false;
    }
    //All lights are on. turn them off.
    else if(rgbwFlag){
        for (x = w; x > 0; x--) {
          analogWrite(REDPIN, x);
          analogWrite(GREENPIN, x);
          analogWrite(BLUEPIN, x);
          analogWrite(WHITEPIN, x);
          delay(wAmp*fadeRatio);
        }
      r = 0;
      g = 0;
      b = 0;
      w = 0;
      analogWrite(REDPIN, 0);
      analogWrite(GREENPIN, 0);
      analogWrite(BLUEPIN, 0);
      analogWrite(WHITEPIN, 0);
      rgbwFlag = false;
    }
    IdleCounter++;
  }
}

