//Import all libraries necessary for this class.
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <SPI.h>
//Define pins.
#define BREAKOUT_RESET 9
#define BREAKOUT_CS 10
#define BREAKOUT_DCS 8
#define SHIELD_RESET -1
#define SHIELD_CS 7
#define SHIELD_DCS 6
#define CARDCS 4
#define DREQ 3

//Object variable for the VS1053.
Adafruit_VS1053_FilePlayer vs1053 = Adafruit_VS1053_FilePlayer(
	BREAKOUT_RESET,
	BREAKOUT_CS,
	BREAKOUT_DCS,
	DREQ,
	CARDCS
);

/*
mp3Index is the index of an .mp3 file inside the SD card.
mp3IndexPrevious is a temporary variable to detect change in
	mp3Index.
*/
unsigned int mp3Index = 1;
unsigned int mp3IndexPrevious = 0;

//Constant variables of the limit of mp3Index.
const unsigned int mp3IndexMin = 1;
const unsigned int mp3IndexMax = 3;

/*
Buffer variables to convert integer to String to char array.
50 here is an arbitrary number.
But make sure that this number is long enough to holds any .mp3
	file name inside the SD card.
*/
char mp3IndexBufferCharArray[50];
String mp3IndexBufferString;

void setup(){

	Serial.begin(9600);
	Serial.println(
		"VS1053 class setup."
	);

	//Verify all components.
	if(!vs1053.begin()){
		Serial.println("VS1053 is not connected.");
		while(1);
	}
	Serial.println("VS1053 is connected.");
	if(!vs1053.useInterrupt(VS1053_FILEPLAYER_PIN_INT)){
		Serial.println("VS1053 DREQ pin is not connected to interrupt pin.");
		while(1);
	}
	Serial.println("VS1053 DREQ pin is connected to interrupt pin.");
	if(!SD.begin(CARDCS)){
		Serial.println(
			"SD card slot is not connected."
		);
		while(1);
	}
	Serial.println("SD card slot is connected.");

	//Print the directory inside the Sd card into Serial console.
	PrintFileDirectory(SD.open("/"), 0);

	//Set the default volume.
	vs1053.setVolume(20, 20);
	//Set the default .mp3 file.
	mp3IndexBufferString = String(mp3Index) + ".mp3";
	mp3IndexBufferString.toCharArray(mp3IndexBufferCharArray, 50);
	Play(mp3IndexBufferCharArray);

}

/*
The loop() override function is basically deivided between these two while loops.
The first while loop happen when there is an audio file playing and the second
	while loop happen when there is no audio file playing.
*/
void loop(){

	while(vs1053.playingMusic){
		DebugSerialButton(vs1053.playingMusic);
	}
	while(!vs1053.playingMusic){
		DebugSerialButton(vs1053.playingMusic);
	}

}

void Play(char _index[]){

	/*
	Checking the audio file availbility when the audio file is not found then
		halt the system.
	*/
	if(!vs1053.startPlayingFile(_index)){
		Serial.println("Could not play " + String(_index));
		while(1);
	}
	Serial.println("Play button pressed.");
	Serial.println(String(_index) + " is currently being played.");

}

//Stop playing button function.
void Stop(){

	vs1053.stopPlaying();
	Serial.println("Stop button just pressed.");

}

//Pause/Unpause button function.
void PauseUnpause(bool _isPaused){

	if(_isPaused){
		vs1053.pausePlaying(false);
		Serial.println("Unpause button just pressed.");
	}
	else if(!_isPaused){
		vs1053.pausePlaying(true);
		Serial.println("Pause button just pressed.");
	}

}

//Next/Previous button function.
void NextPrevious(bool _isNextButton){

	//Stop the currently played audio file.
	vs1053.stopPlaying();
	//Add or substract one point from mp3Index.
	if(_isNextButton){ mp3Index ++; }
	else if(!_isNextButton){ mp3Index --; }
	/*
	Just incase the mp3Index is beyond the available file, so we need to
		bound the value of mp3Index.
	*/
	mp3Index = BoundAndInvertValue(mp3Index, mp3IndexMin, mp3IndexMax);
	//Convert the integer to String.
	mp3IndexBufferString = String(mp3Index) + ".mp3";
	//Convert the String into character array.
	mp3IndexBufferString.toCharArray(mp3IndexBufferCharArray, 50);
	//Play the music based on the mp3 index in character array.
	Play(mp3IndexBufferCharArray);
	if(_isNextButton){ Serial.println("Next button just pressed."); }
	else if(!_isNextButton){ Serial.println("Previous button just pressed."); }

}

//PENDING.
void VolumePlus(){}

//PENDING.
void VolumeMin(){}

/*
Original PrintFileDirectory from the Adafruit VS1053 library.
The purpose of this function is to show every file inside the Serial console.
*/
void PrintFileDirectory(File _dir, int _numTabs){

	while(true){
		File entry = _dir.openNextFile();
		if(!entry){ break; }
		for(uint8_t i = 0; i < _numTabs; i ++){
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if(entry.isDirectory()){
			Serial.println("/");
			PrintFileDirectory(entry, _numTabs + 1);
		}
		else{
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
	}

}

/*
If there is a value that go beyond maximum or minimum limit
	revert it back to its minimum or maximum limit.
*/
int BoundAndInvertValue(int _value, int _minValue, int _maxValue){

	if(_value > _maxValue){ _value = _minValue; }
	else if(_value < _minValue){ _value = _maxValue; }
	return _value;
	
}

//For this prototype I use Serial input to activate the button.
void DebugSerialButton(bool _isPlayingMusic){

	if(Serial.available() > 0){
		int mode = Serial.read();
		if(mode == 49){
			if(_isPlayingMusic){ Stop(); }
			else if(!_isPlayingMusic){
				Play(mp3IndexBufferCharArray);
			}
		}
		else if(mode == 50){
			PauseUnpause(vs1053.paused());
		}
		else if(mode == 51){ NextPrevious(true); }
		else if(mode == 52){ NextPrevious(false); }
		else if(mode == 53){ /*PENDING: Volume button.*/ }
		else if(mode == 54){ /*PENDING: Volume button.*/ }
	}

}